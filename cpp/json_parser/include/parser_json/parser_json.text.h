#pragma once

namespace parser::detail {


  template <typename Char, typename Maker>
  class nest_json<Char, Maker>::node_text
    : public node_base
  {
  public:
    static std::string get_name() { return "t_text"; }

    static bool condition(json_params const * params, Char ch)
    {
      return (ch == info::quote_double);
    }

    static ptr_node create(Maker * maker, json_params const * params, pos_type start_pos)
    {
      return std::make_unique<node_text>(start_pos);
    }

    static constexpr choicer_type choicer{&get_name, &condition, &create};


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
          st.skip_read();
          st.after_fn = &parser_state::action_up_only;
        }
        return;
      }

      if( ch == info::quote_double )
      {
        st.after_fn = &parser_state::action_up_result;
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

    void input_ended(parser_state & st, response_type & resp) override
    {
      if( resp.status == json_status::n_ok )
      {
        resp.status = json_status::n_text_unclosed;
      }
    }
  };


} // end ns
