#pragma once

#include "json.nest.h"

namespace parser::detail {


  template <typename Char, typename Maker, typename Log_string>
  class json_nest<Char, Maker, Log_string>::node_number
    : public node_base
  {
  public:
    static std::string get_name() { return "t_number"; }

    static bool condition(json_params const * params, Char ch)
    {
      return (
        ksi::chars::is_digit(ch) || is_eq(ch, info::minus, info::plus, info::dot)
      );
    }

    static ptr_node create(
      Maker * maker,
      json_params const * params,
      pos_type start_pos,
      state_data const & data
    )
    {
      return std::make_unique<node_number>(start_pos);
    }

    static constexpr choicer_type choicer{ &get_name, &condition, &create };


    using integer_target = Maker::integer;
    using floating_target = Maker::floating;

    using limits_float = std::numeric_limits<floating_target>;

  protected:
    enum { initial_radix = 10 };

    struct count_info
    {
      using type = std::uintmax_t;

      // props

      type total{ 0 }; // zero digits at beginning are skipped
      type fract_only{ 0 };

      bool is_full{ false };
    };

    struct fract_info
    {
      using type = std::intmax_t;

      enum divider_status : type { no_divider = 0 };

      static constexpr lib_number::numeric_edge<type, initial_radix, true>
        edge{ std::numeric_limits<type>::max() };

      // props

      type radix{ initial_radix };
      type value{ 0 };
      type divider{ no_divider };

      bool is_full{ false };
    };

    struct int_part_info
    {
      using type_int = std::intmax_t;
      using type_float = long double;

      using edges = lib_number::numeric_edges<integer_target, type_int, initial_radix>;

      enum kind { kind_int, kind_float };

      // props

      kind state{ kind_int };

      type_int radix{ initial_radix };
      type_int value_int{ 0 };
      type_float value_float{ 0.0 };
    };

    enum sign_state : typename int_part_info::type_int
    {
      sign_negative = -1, sign_absent = 0, sign_positive = +1
    };

    using e_type = fract_info::type; //std::remove_const_t<decltype(limits_float::max_exponent10)>;

    enum e_sign_state : e_type
    {
      e_sign_negative = -1, e_sign_absent = 0, e_sign_positive = +1
    };

    struct e_part_info
    {
      using edges_type = lib_number::numeric_edges_custom<e_type, initial_radix>;

      // props
      edges_type edges{ limits_float::min_exponent10, limits_float::max_exponent10 - 1 };
      e_type radix{ initial_radix };
      e_type e_sign{ e_sign_absent };
      e_type value{ 0 };
      e_type offset{ 0 }; // first non-zero digit distance to dot; 01.0 means: .offset == 0
      bool should_be_zero{ false };
      bool ignore_digits{ false };

      void edges_calc()
      {
        edges = edges_type{
          static_cast<e_type>(limits_float::min_exponent10) - offset,
          static_cast<e_type>(limits_float::max_exponent10) - offset - 1
        };
      }

      bool is_present() const
      {
        return (e_sign != e_sign_absent);
      }
    };

    using flags_t = size_t;
    enum kind : flags_t
    {
      kind_end       = (1 << 0),
      kind_sign      = (1 << 1),
      kind_digit     = (1 << 2),
      kind_dot       = (1 << 3),
      kind_dot_digit = (1 << 4),
      kind_e         = (1 << 5),
      kind_e_sign    = (1 << 6),
      kind_e_digit   = (1 << 7),

      is_kind_start      = (kind_digit | kind_sign | kind_dot),
      was_kind_sign      = (kind_digit | kind_dot),
      was_kind_digit     = (kind_digit | kind_dot | kind_e | kind_end),
      was_kind_dot       = (kind_dot_digit | kind_end),
      was_kind_dot_digit = (kind_dot_digit | kind_e | kind_end),
      was_kind_e         = (kind_e_sign),
      was_kind_e_sign    = (kind_e_digit),
      was_kind_e_digit   = (kind_e_digit | kind_end),
    };

    // props
    flags_t req{ is_kind_start };
    std::vector<Log_string> expected;
    int_part_info::type_int signum{ sign_absent };
    int_part_info int_part{};
    fract_info fract{};
    e_part_info e_part{};
    count_info count{};
    bool was_non_zero{ false };
    bool was_digit{ false };

    // base ctor
    using node_base::node_base; 

    bool check_huge(parser_state & st, response_type & resp)
    {
      if( count.is_full ) { return true; }
      if( was_non_zero )
      {
        if( count.total == std::numeric_limits<count_info::type>::max() )
        {
          count.is_full = true;
          log_messages{ st.data.log, &resp, st.position.get() }.number_huge(
            count.total
          );
          return true;
        }
        ++count.total;
      }
      return false;
    }

    void on_digit(parser_state & st, response_type & resp, int_part_info::type_int digit)
    {
      was_digit = true;
      
      // skip leading zeros, prepare E-notation
      if( was_non_zero ) { ++e_part.offset; }
      else if( digit != 0 ) { was_non_zero = true; }
      else { return; }

      std::cout << "{ " << e_part.offset << " }";

      if( check_huge(st, resp) ) { return; }

      typename int_part_info::type_int digit_adding{ (signum < 0) ? (-digit) : digit };

      if( int_part.state == int_part_info::kind_int )
      {
        if( int_part_info::edges::clarify(int_part.value_int, digit) )
        {
          int_part.value_int *= int_part.radix;
          int_part.value_int += digit_adding;
          int_part.value_float = static_cast<floating_target>(int_part.value_int);
          return;
        }
        else
        {
          int_part.state = int_part_info::kind_float;
        }
      }

      int_part.value_float *= int_part.radix;
      int_part.value_float += digit_adding;
    }

    void on_digit_fract(parser_state & st, response_type & resp, fract_info::type digit)
    {
      was_digit = true;

      bool is_first_non_zero_digit{ false };
      if( (was_non_zero == false) && (digit != 0) )
      {
        is_first_non_zero_digit = true;
        was_non_zero = true;
      }

      if( check_huge(st, resp) ) { return; }

      if( fract.is_full ) { return; }
      if( fract_info::edge.can_be_added(fract.divider, 0) )
      {
        fract.value *= fract.radix;
        fract.value += digit;
        fract.divider *= fract.radix;
        ++count.fract_only;
        if( is_first_non_zero_digit )
        {
          e_part.offset = -static_cast<e_type>(count.fract_only);
        }
      }
      else
      {
        fract.is_full = true;
        log_messages{ st.data.log, &resp, st.position.get() }.number_fractional_part_too_long(
          count.fract_only
        );
      }
    }

    void on_dot(parser_state & st, response_type & resp, Char ch)
    {
      if( fract.divider != fract_info::no_divider ) // seems no need anymore to check this
      {
        st.skip_read();
        st.after_fn = &chain_actions::chain_up_result;
        return;
      }
      fract.divider = 1;
      int_part.state = int_part_info::kind_float;
    }

    void on_digit_e(parser_state & st, response_type & resp, e_type digit)
    {
      if( e_part.ignore_digits ) { return; }

      std::cout << std::boolalpha << e_part.should_be_zero << " ";

      if( e_part.should_be_zero && (digit != 0) )
      {
        e_part.ignore_digits = true;
        log_messages{ st.data.log, &resp, st.position.get() }.number_e_notation_not_zero();
        return;
      }

      e_type fixed{ 0 };
      if( e_part.edges.clarify(e_part.value, digit, e_part.e_sign, fixed) )
      {
        e_part.value = fixed;
      }
      else
      {
        e_part.ignore_digits = true;
        log_messages{ st.data.log, &resp, st.position.get() }.number_e_notation_range(
          std::exchange(e_part.value, fixed), digit,
          e_part.edges.number_low, e_part.edges.number_high,
          fixed
        );
      }
    }

  public:
    void parse(parser_state & st, response_type & resp, Char ch) override
    {
      expected.clear();

      if( lib_bits::flags_has_any(req, kind_sign) )
      {
        if( ch == info::plus )
        {
          req = was_kind_sign;
          signum = sign_positive;
          return;
        }
        if( ch == info::minus )
        {
          req = was_kind_sign;
          signum = sign_negative;
          return;
        }

        expected.push_back(log_conv_type{}("Sign"));
      }

      if( lib_bits::flags_has_any(req, kind_digit) )
      {
        if( ksi::chars::is_digit(ch) )
        {
          req = was_kind_digit;
          on_digit(st, resp, ksi::chars::digit_of<fract_info::type>(ch));
          return;
        }

        expected.push_back(log_conv_type{}("Digit"));
      }

      if( lib_bits::flags_has_any(req, kind_dot) )
      {
        if( ch == info::dot )
        {
          req = was_kind_dot;
          on_dot(st, resp, ch);
          return;
        }

        expected.push_back(log_conv_type{}("Dot"));
      }

      if( lib_bits::flags_has_any(req, kind_dot_digit) )
      {
        if( ksi::chars::is_digit(ch) )
        {
          req = was_kind_dot_digit;
          on_digit_fract(st, resp, ksi::chars::digit_of<int_part_info::type_int>(ch));
          return;
        }

        expected.push_back(log_conv_type{}("Digit"));
      }

      if( lib_bits::flags_has_any(req, kind_e) )
      {
        bool condition{
          st.params->number.e_notation_lowercase_letter ?
          is_eq(ch, info::letter_E, info::letter_e) :
          (ch == info::letter_E)
        };
        if( condition )
        {
          req = was_kind_e;
          e_part.edges_calc();
          return;
        }

        expected.push_back(log_conv_type{}("Letter_E"));
      }

      if( lib_bits::flags_has_any(req, kind_e_sign) )
      {
        if( ch == info::plus )
        {
          req = was_kind_e_sign;
          e_part.e_sign = e_sign_positive;
          if( e_part.edges.number_high < 0 )
          {
            e_part.ignore_digits = true;
            log_messages{ st.data.log, &resp, this->start_pos }.number_e_notation_not_negative();
            return;
          }
          if( e_part.edges.number_high == 0 )
          {
            e_part.should_be_zero = true;
          }
          return;
        }
        if( ch == info::minus )
        {
          req = was_kind_e_sign;
          e_part.e_sign = e_sign_negative;
          if( e_part.edges.number_low > 0 )
          {
            e_part.ignore_digits = true;
            log_messages{ st.data.log, &resp, this->start_pos }.number_e_notation_not_positive();
            return;
          }
          if( e_part.edges.number_low == 0 )
          {
            e_part.should_be_zero = true;
          }
          return;
        }

        expected.push_back(log_conv_type{}("Sign"));
      }

      if( lib_bits::flags_has_any(req, kind_e_digit) )
      {
        if( ksi::chars::is_digit(ch) )
        {
          req = was_kind_e_digit;
          on_digit_e(st, resp, ksi::chars::digit_of<e_type>(ch));
          return;
        }

        expected.push_back(log_conv_type{}("Digit"));
      }

      // not match
      if( lib_bits::flags_absent(req, kind_end) )
      {
        log_messages{ st.data.log, &resp, st.position.get() }.number_unexpected_symbol(
          expected
        );
      }
      st.after_fn = &chain_actions::chain_up_result;
      st.skip_read();
    }

    result_type get_result(parser_state & st, response_type & resp) override
    {
      if( int_part.state == int_part_info::kind_int )
      {
        integer_target number = static_cast<integer_target>(int_part.value_int);
        return st.maker->make_integer(st.data.path, this->start_pos, number);
      }
      else
      {
        // dot_only as nan
        if(
          st.params->number.nan_from_dot_only &&
          (was_digit == false) && (signum == sign_absent)
        )
        {
          return st.maker->make_floating(
            st.data.path,
            this->start_pos,
            std::numeric_limits<floating_target>::quiet_NaN()
          );
        }

        // sign dot (no digits) as infinity
        if(
          st.params->number.infinity_from_dot_signed &&
          (was_digit == false) && (signum != sign_absent)
        )
        {
          return st.maker->make_floating(
            st.data.path,
            this->start_pos,
            std::numeric_limits<floating_target>::infinity() * signum
          );
        }

        if( count.total > (limits_float::digits10) )
        {
          log_messages{ st.data.log, &resp, this->start_pos }.number_too_much_digits(
            count.total, limits_float::digits10
          );
        }

        long double ret{ int_part.value_float };

        if( fract.divider != fract_info::no_divider )
        {
          long double ret_fractional{static_cast<long double>(fract.value)};
          ret_fractional /= fract.divider;
          if( signum < 0 )
          {
            ret -= ret_fractional;
          }
          else
          {
            ret += ret_fractional;
          }
        }

        if( e_part.is_present() )
        {
          e_type allowed_power;
          if( e_part.edges.is_unfit_calc_correct(e_part.value, allowed_power) )
          {
            log_messages{ st.data.log, &resp, this->start_pos }.number_e_was_unfit(
              e_part.value,
              e_part.edges.number_low, e_part.edges.number_high,
              allowed_power
            );
          }
          if( allowed_power < 0 )
          {
            long double factor{ std::powl(
              initial_radix, -static_cast<long double>(allowed_power)
            ) };
            ret /= factor;
          }
          else
          {
            long double factor{ std::powl(
              initial_radix, static_cast<long double>(allowed_power)
            ) };
            ret *= factor;
          }
        }

        return st.maker->make_floating(
          st.data.path,
          this->start_pos,
          static_cast<floating_target>(ret)
        );
      }
    }

    void input_ended(parser_state & st, response_type & resp) override
    {
      if( lib_bits::flags_absent(req, kind_end) )
      {
        log_messages{ st.data.log, &resp, st.position.get() }.number_not_complete(
          expected
        );
      }
    }
  }; // end class


} // end ns
