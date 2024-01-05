#pragma once

namespace parser::detail {


  template <typename Char, typename Maker>
  class nest_json<Char, Maker>::node_number
    : public node_base
  {
  public:
    using integer = Maker::integer;
    using floating = Maker::floating;
    using limits = std::numeric_limits<integer>;

    static constexpr integer
      radix{10}
    , max_pre{limits::max() / radix}
    , max_last_digit{limits::max() % radix}
    ;

  protected:
    integer before_dot{0};
    integer after_dot{0};
    integer dot{-1};
    integer sign{0};
    floating part{0.0};
    bool is_float = false;
    index_t count_digits{ 0 };

    void on_digit(integer digit)
    {
      ++count_digits;
      if( dot < 0 )
      {
        if(
          is_float == false && (
            before_dot < max_pre || (
              (before_dot == max_pre) && (digit <= max_last_digit)
            )
          )
        ) {
          before_dot *= radix;
          before_dot += digit;
          part = static_cast<floating>(before_dot);
        }
        else
        {
          is_float = true;
          part *= radix;
          part += digit;
        }
      }
      else
      {
        after_dot *= radix;
        after_dot += digit;
        dot *= radix;
      }
    }

    void on_dot(parser_state & st, response_type & resp, Char ch)
    {
      if( dot > -1 )
      {
        st.next_action = &parser_state::action_ask_parent;
        return;
      }
      dot = 1;
      is_float = true;
    }

  public:
    void parse(parser_state & st, response_type & resp, Char ch) override
    {
      if( st.position->char_pos == this->start_char_pos )
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
        on_digit(ksi::chars::digit_of<integer>(ch));
        return;
      }

      if( ch == info::dot )
      {
        on_dot(st, resp, ch);
        return;
      }
      
      // no matches
      st.next_action = &parser_state::action_ask_parent;
    }

    result_type get_result(parser_state & st) override
    {
      if( is_float == false )
      {
        return st.maker_pointer->make_integer((sign < 0) ? -before_dot : before_dot);
      }
      else
      {
        if( st.params->number.nan_only_dot && (count_digits == 0) && (sign == 0) )
        {
          return st.maker_pointer->make_floating( std::numeric_limits<floating>::quiet_NaN() );
        }
        if( st.params->number.infinity_sign_dot && (count_digits == 0) && (sign != 0) )
        {
          return st.maker_pointer->make_floating(std::numeric_limits<floating>::infinity() * sign);
        }
        floating ret_part{part};
        floating ret_frac{static_cast<Maker::floating>(after_dot)};
        ret_frac /= dot;
        ret_part += ret_frac;
        return st.maker_pointer->make_floating((sign < 0) ? -ret_part : ret_part);
      }
    }
  };


} // end ns
