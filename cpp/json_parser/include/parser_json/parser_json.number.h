#pragma once

namespace parser::detail {


  template <typename Char, typename Maker>
  class nest_json<Char, Maker>::node_number
    : public node_base
  {
  public:
    static constexpr index_t radix{10};

  protected:
    index_t before_dot{0};
    index_t after_dot{0};
    index_t dot{-1};
    bool is_negative{false};

    void on_digit(index_t digit)
    {
      if( dot < 0 )
      {
        before_dot *= radix;
        before_dot += digit;
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
        resp.status = json_status::n_number_double_dot;
        st.next_action = &parser_state::action_ask_parent;
        return;
      }
      dot = 1;
    }

  public:
    void parse(parser_state & st, response_type & resp, Char ch) override
    {
      if( ksi::chars::is_digit(ch) )
      {
        on_digit(ksi::chars::digit_of(ch));
      }
      else if( ch == info::dot )
      {
        on_dot(st, resp, ch);
      }
      else
      {
        st.next_action = &parser_state::action_ask_parent;
      }
    }

    result_type get_result(parser_state & st) override
    {
      if( dot < 0 )
      {
        return st.maker_pointer->make_integer(before_dot);
      }
      else
      {
        typename Maker::floating part{static_cast<Maker::floating>(before_dot)};
        typename Maker::floating frac{static_cast<Maker::floating>(after_dot)};
        frac /= dot;
        part += frac;
        return st.maker_pointer->make_floating(is_negative ? -part : part);
      }
    }
  };


} // end ns
