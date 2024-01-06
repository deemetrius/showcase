#pragma once

namespace parser::detail {


  template <typename Char, typename Maker>
  class nest_json<Char, Maker>::node_text
    : public node_base
  {
  public:
    static bool condition(json_params const * params, Char ch)
    {
      return (ch == info::quote_double);
    }

    static state create(Maker * maker, json_params const * params, pos_type start_pos)
    {
      return std::make_unique<node_text>(start_pos);
    }

    static constexpr choicer_type choicer{&condition, &create};


    using text_type = Maker::text;
    using stream_type = std::basic_stringstream<Char>;

    stream_type stream;

    using node_base::node_base;

    void parse(parser_state & st, response_type & resp, Char ch) override
    {
      if( st.position->char_pos == this->start_pos.char_pos )
      {
        if( ch != info::quote_double )
        {
          // not match
          st.next_action = &parser_state::action_ask_parent_no_value;
        }
        return;
      }

      if( ch == info::quote_double )
      {
        st.next_action = &parser_state::action_up;
        return;
      }

      // todo: escape sequences

      stream << ch;
    }
    
    result_type get_result(parser_state & st) override
    {
      return st.maker->make_text(
        this->start_pos,
        ksi::conv::from_string::to<text_type>{}(stream.str())
      );
    }
  };


} // end ns
