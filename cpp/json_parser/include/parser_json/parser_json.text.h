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
    bool was_slash{ false };

    using node_base::node_base; // base ctor

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

      // escape sequences
      // todo: \uFFFF
      if( was_slash )
      {
        was_slash = false;
        switch( ch )
        {
        case info::letter_f:
          stream << info::ff;
          return;

        case info::letter_r:
          stream << info::cr;
          return;

        case info::letter_n:
          stream << info::lf;
          return;

        case info::letter_t:
          stream << info::tab;
          return;

        case info::letter_b:
          stream << info::bs;
          return;

        default:
          stream << ch;
          return;
        }
      }

      if( ch == info::quote_double )
      {
        st.after_fn = &parser_state::action_up_result;
        return;
      }

      if( ch == info::slash )
      {
        was_slash = true;
        return;
      }

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
      resp.change_status(json_status::n_text_unclosed);
    }
  };


} // end ns
