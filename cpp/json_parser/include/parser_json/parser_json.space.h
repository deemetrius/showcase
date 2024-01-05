#pragma once

namespace parser::detail {


  template <typename Char, typename Maker>
  class nest_json<Char, Maker>::node_space
    : public node_base
  {
  protected:
    using node_base::node_base;

  public:
    static bool condition(json_params const * params, Char ch)
    {
      return
           (ch == info::space)
        || (ch == info::tab)
        || (ch == info::cr)
        || (ch == info::lf)
        ;
    }

    static state create(json_params const * params, pos_type pos)
    {
      return std::make_unique<node_space>(pos);
    }

    static constexpr choicer_type choicer{&condition, &create, 0};


    void parse(parser_state & st, response_type & resp, Char ch) override
    {
      if( condition(st.params, ch) == false )
      {
        // not match
        st.next_action = &parser_state::action_ask_parent_no_value;
      }
    }

    result_type get_result(parser_state & st) override
    {
      throw unexpected_result{ this->start_pos};
      return st.maker->make_null(
        this->start_pos
      );
    }
  };


} // end ns
