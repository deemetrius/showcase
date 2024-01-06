#pragma once

namespace parser::detail {


  template <typename Char, typename Maker>
  class nest_json<Char, Maker>::node_number
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

    using node_base::node_base;

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

    result_type get_result(parser_state & st) override
    {
      if( is_float == false )
      {
        return st.maker->make_integer(
          this->start_pos,
          (sign < 0) ? -before_dot : before_dot
        );
      }
      else
      {
        if( st.params->number.nan_only_dot && (count_digits == 0) && (sign == 0) )
        {
          return st.maker->make_floating(
            this->start_pos,
            std::numeric_limits<floating>::quiet_NaN()
          );
        }
        if( st.params->number.infinity_sign_dot && (count_digits == 0) && (sign != 0) )
        {
          return st.maker->make_floating(
            this->start_pos,
            std::numeric_limits<floating>::infinity() * sign
          );
        }
        floating ret_part{part};
        floating ret_frac{static_cast<Maker::floating>(after_dot)};
        ret_frac /= dot;
        ret_part += ret_frac;
        return st.maker->make_floating(
          this->start_pos,
          (sign < 0) ? -ret_part : ret_part
        );
      }
    }
  };


} // end ns
