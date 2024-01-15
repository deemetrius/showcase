#pragma once

#include <cstdint>

namespace parser::detail {

  /*
    вспомогательные классы для проверки
    можно ли без переполнения цифру добавить в целое число

    иначе будет задействовано формирование числа с плавающей точкой
  */

  template <typename Integer, Integer Radix, bool Is_positive>
  struct numeric_edge
  {
    static constexpr Integer calc_last_digit(Integer number)
    {
      if constexpr( Is_positive )
      {
        return (number % Radix);
      }
      else
      {
        return -(number % Radix);
      }
    }

    Integer prelimit;
    Integer last_digit; // not negative

    constexpr numeric_edge(Integer limit)
      : prelimit{ limit / Radix}
      , last_digit{ calc_last_digit(limit) }
    {}

    constexpr bool is_fit(Integer number, Integer digit) const
    {
      if constexpr( Is_positive )
      {
        return (
          (number < prelimit) || (
            (number == prelimit) && (digit <= last_digit)
          )
        );
      }
      else
      {
        return (
          (number > prelimit) || (
            (number == prelimit) && (digit <= last_digit)
          )
        );
      }
    }
  };

  template <typename Integer_target, typename Integer_storage, Integer_storage Radix>
  struct numeric_edges
  {
    static_assert(
      sizeof(Integer_target) <= sizeof(Integer_storage)
    );

    using limits = std::numeric_limits<Integer_target>;

    static constexpr numeric_edge<Integer_storage, Radix, true>
      edge_plus{ limits::max() };
    static constexpr numeric_edge<Integer_storage, Radix, false>
      edge_minus{ limits::min() };

    static constexpr bool clarify(Integer_storage number, Integer_storage digit)
    {
      return (
        (number < 0) ?
        edge_minus.is_fit(number, digit) :
        edge_plus.is_fit(number, digit)
      );
    }
  };


  // node_number

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
    using integer_target = Maker::integer;
    using floating = Maker::floating;


    static constexpr integer
      radix{10};
    using edges = numeric_edges<integer_target, integer, 10>;


  protected:
    integer before_dot{ 0 }; // can be nigative
    integer after_dot{ 0 }; // can be nigative too
    integer dot{ -1 }; // no dot from start
    integer sign{ 0 };
    floating part{ 0.0 };
    bool is_float{ false };
    index_t count_digits{ 0 };

    using node_base::node_base; // base ctor

    void on_digit(integer digit)
    {
      ++count_digits;
      integer digit_adding{(sign < 0) ? (-digit) : digit};
      if( dot < 0 )
      {
        if(
          (is_float == false) && edges::clarify(before_dot, digit)
        ) {
          before_dot *= radix;
          before_dot += digit_adding;
          part = static_cast<floating>(before_dot);
        }
        else
        {
          is_float = true;
          part *= radix;
          part += digit_adding;
        }
      }
      else
      {
        after_dot *= radix;
        after_dot += digit_adding;
        dot *= radix;
        // maybe todo: check precision limit
      }
    }

    void on_dot(parser_state & st, response_type & resp, Char ch)
    {
      if( dot > -1 )
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
        on_digit( ksi::chars::digit_of<integer>(ch) );
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
        if( st.params->number.nan_only_dot && (count_digits == 0) && (sign == 0) )
        {
          return st.maker->make_floating(
            this->start_pos,
            std::numeric_limits<floating>::quiet_NaN()
          );
        }

        // sign dot (no digits) as infinity
        if( st.params->number.infinity_sign_dot && (count_digits == 0) && (sign != 0) )
        {
          return st.maker->make_floating(
            this->start_pos,
            std::numeric_limits<floating>::infinity() * sign
          );
        }

        floating ret{part};

        {
          floating ret_frac{static_cast<Maker::floating>(after_dot)};
          ret_frac /= dot;
          ret += ret_frac;
        }

        return st.maker->make_floating(
          this->start_pos,
          ret
        );
      }
    }
  };


} // end ns
