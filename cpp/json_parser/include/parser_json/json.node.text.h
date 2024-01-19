#pragma once

#include "json.nest.h"

namespace parser::detail {


  template <typename Char, typename Maker, typename Log_string>
  class json_nest<Char, Maker, Log_string>::node_text
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
    pos_type slash_pos{ -1, 0, 0 };

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
          case info::letter_n:
          stream << info::lf;
          return;

          case info::letter_r:
          stream << info::cr;
          return;

          case info::letter_t:
          stream << info::tab;
          return;

          case info::letter_f:
          stream << info::ff;
          return;

          case info::letter_b:
          stream << info::bs;
          return;

          case info::quote_double: { [[fallthrough]]; }
          case info::slash:
          stream << ch;
          return;

          default:
          stream << ch;
          st.data.log->inform({
            log_messages::text_escape_sequence_notice(),
            json_message_type::n_notice,
            slash_pos
          });
        }
        return;
      }

      if( ch == info::quote_double )
      {
        st.after_fn = &parser_state::action_up_result;
        return;
      }

      if( ch == info::slash )
      {
        was_slash = true;
        slash_pos = st.position.get();
        return;
      }

      stream << ch;
    }
    
    result_type get_result(parser_state & st, response_type & resp) override
    {
      return st.maker->make_text(
        this->start_pos,
        ksi::conv::from_string::to<text_type>{}(stream.str())
      );
    }

    void input_ended(parser_state & st, response_type & resp) override
    {
      st.data.log->inform({
        log_messages::text_unclosed(),
        json_message_type::n_error,
        st.position.get()
      });
      resp.change_status(json_message_codes::n_string_unclosed);
    }
  };


} // end ns
