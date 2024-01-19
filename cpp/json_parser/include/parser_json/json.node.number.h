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
      return
        ksi::chars::is_digit(ch)
        || (ch == info::minus)
        || (ch == info::plus)
        || (ch == info::dot)
      ;
    }

    static ptr_node create(Maker * maker, json_params const * params, pos_type start_pos)
    {
      return std::make_unique<node_number>(start_pos);
    }

    static constexpr choicer_type choicer{&get_name, &condition, &create};


    using integer = std::intmax_t;
    using integer_unsigned = std::uintmax_t;
    using integer_target = Maker::integer;
    using floating = Maker::floating;

    using limits_float = std::numeric_limits<floating>;
    using limits_fract = std::numeric_limits<integer_unsigned>;

    static constexpr integer
      radix{ 10 };
    using edges = lib_number::numeric_edges<integer_target, integer, radix>;
    using edge_fract_type = lib_number::numeric_edge<integer_unsigned, radix, true>;
    static constexpr edge_fract_type edge_fract{ limits_fract::max() };


  protected:
    enum dot_status { no_dot = 0 };

    // props
    bool was_digit{ false };
    bool was_non_zero{ false };
    bool huge_digit_count{ false };
    integer before_dot{ 0 }; // can be negative
    integer_unsigned after_dot{ 0 };
    integer_unsigned dot{ no_dot };
    integer sign{ 0 };
    long double part{ 0.0 };
    bool is_float{ false };
    index_t count_digits{ 0 };

    using node_base::node_base; // base ctor

    void on_digit(parser_state & st, integer digit)
    {
      was_digit = true;
      if( digit != 0 ) { was_non_zero = true; }
      if( huge_digit_count ) { return; }
      if( was_non_zero )
      {
        if( count_digits == std::numeric_limits<index_t>::max() )
        {
          huge_digit_count = true;
          st.data.log->inform({
            log_messages::number_huge(count_digits),
            json_message_type::n_warning,
            st.position.get()
          });
          return;
        }
        ++count_digits;
      }
      integer digit_adding{(sign < 0) ? (-digit) : digit};
      if( is_float == false )
      {
        if( edges::clarify(before_dot, digit) )
        {
          before_dot *= radix;
          before_dot += digit_adding;
          part = static_cast<floating>(before_dot);
          return;
        }
        else
        {
          is_float = true;
        }
      }
      if( dot == no_dot )
      {
        part *= radix;
        part += digit_adding;
        return;
      }
      if( edge_fract.can_be_added(after_dot, digit) )
      {
        after_dot *= radix;
        after_dot += digit;
        dot *= radix;
      }
    }

    void on_dot(parser_state & st, response_type & resp, Char ch)
    {
      if( dot != no_dot )
      {
        st.skip_read();
        st.after_fn = &parser_state::action_up_result;
        return;
      }
      dot = 1;
      is_float = true;
    }

  public:
    void parse(parser_state & st, response_type & resp, Char ch) override
    {
      if( st.position->char_pos == this->start_pos.char_pos )
      {
        if( ch == info::plus )
        {
          sign = +1;
          return;
        }
        if( ch == info::minus )
        {
          sign = -1;
          return;
        }
      }

      if( ksi::chars::is_digit(ch) )
      {
        on_digit( st, ksi::chars::digit_of<integer>(ch) );
        return;
      }

      if( ch == info::dot )
      {
        on_dot(st, resp, ch);
        return;
      }
      
      // not match
      st.skip_read();
      st.after_fn = &parser_state::action_up_result;
    }

    result_type get_result(parser_state & st, response_type & resp) override
    {
      if( is_float == false )
      {
        integer_target number = static_cast<integer_target>(before_dot);
        return st.maker->make_integer(
          this->start_pos,
          number
        );
      }
      else
      {
        // dot_only as nan
        if( st.params->number.nan_only_dot && (was_digit == false) && (sign == 0) )
        {
          return st.maker->make_floating(
            this->start_pos,
            std::numeric_limits<floating>::quiet_NaN()
          );
        }

        // sign dot (no digits) as infinity
        if( st.params->number.infinity_sign_dot && (was_digit == false) && (sign != 0) )
        {
          return st.maker->make_floating(
            this->start_pos,
            std::numeric_limits<floating>::infinity() * sign
          );
        }

        if( count_digits > (limits_float::digits10) )
        {
          st.data.log->inform({
            log_messages::number_too_much_digits(count_digits, limits_float::digits10),
            json_message_type::n_warning,
            this->start_pos
          });
        }

        long double ret{ part };

        if( dot != no_dot )
        {
          long double ret_fractional{static_cast<long double>(after_dot)};
          ret_fractional /= dot;
          if( sign < 0 )
          {
            ret -= ret_fractional;
          }
          else
          {
            ret += ret_fractional;
          }
        }

        return st.maker->make_floating(
          this->start_pos,
          static_cast<floating>(ret)
        );
      }
    }
  };


} // end ns
