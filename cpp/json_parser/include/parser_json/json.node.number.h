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
      using type = std::uintmax_t;

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
    { sign_negative = -1, sign_absent = 0, sign_positive = +1 };

    // props
    int_part_info::type_int signum{ sign_absent };
    int_part_info int_part{};
    fract_info fract{};
    count_info count{};
    bool was_digit{ false };
    bool was_non_zero{ false };

    // base ctor
    using node_base::node_base; 

    void on_digit(parser_state & st, int_part_info::type_int digit)
    {
      was_digit = true;
      if( digit != 0 ) { was_non_zero = true; }
      if( count.is_full ) { return; }
      if( was_non_zero )
      {
        if( count.total == std::numeric_limits<count_info::type>::max() )
        {
          count.is_full = true;
          st.data.log->inform({
            log_messages::number_huge(count.total),
            json_message_type::n_warning,
            st.position.get()
          });
          return;
        }
        ++count.total;
      }
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
      if( fract.divider == fract_info::no_divider )
      {
        int_part.value_float *= int_part.radix;
        int_part.value_float += digit_adding;
        return;
      }
      if( fract.is_full ) { return; }
      if( fract_info::edge.can_be_added(fract.divider, 0) )
      {
        fract.value *= fract.radix;
        fract.value += digit;
        fract.divider *= fract.radix;
        ++count.fract_only;
      }
      else
      {
        fract.is_full = true;
        st.data.log->inform({
          log_messages::number_fractional_part_too_long(count.fract_only),
          json_message_type::n_notice,
          st.position.get()
        });
      }
    }

    void on_dot(parser_state & st, response_type & resp, Char ch)
    {
      if( fract.divider != fract_info::no_divider )
      {
        st.skip_read();
        st.after_fn = &chain_actions::chain_up_result;
        return;
      }
      fract.divider = 1;
      int_part.state = int_part_info::kind_float;
    }

  public:
    void parse(parser_state & st, response_type & resp, Char ch) override
    {
      if( st.position->char_pos == this->start_pos.char_pos )
      {
        if( ch == info::plus )
        {
          signum = sign_positive;
          return;
        }
        if( ch == info::minus )
        {
          signum = sign_negative;
          return;
        }
      }

      if( ksi::chars::is_digit(ch) )
      {
        on_digit( st, ksi::chars::digit_of<int_part_info::type_int>(ch) );
        return;
      }

      if( ch == info::dot )
      {
        on_dot(st, resp, ch);
        return;
      }
      
      // not match
      st.skip_read();
      st.after_fn = &chain_actions::chain_up_result;
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
          st.data.log->inform({
            log_messages::number_too_much_digits(count.total, limits_float::digits10),
            json_message_type::n_warning,
            this->start_pos
          });
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

        return st.maker->make_floating(
          st.data.path,
          this->start_pos,
          static_cast<floating_target>(ret)
        );
      }
    }
  };


} // end ns
