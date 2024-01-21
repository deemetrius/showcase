#pragma once

#include "json.nest.h"

namespace parser::detail {


  template <typename Char, typename Maker, typename Log_string>
  class json_nest<Char, Maker, Log_string>::node_comments
    : public node_base
  {
  protected:
    using node_base::node_base; // base ctor

  public:
    static std::string get_name() { return "t_comments"; }

    static bool condition(json_params const * params, Char ch)
    {
      if( params->comments.single_line == false ) { return false; }
      return (ch == info::divide);
    }

    static ptr_node create(Maker * maker, json_params const * params, pos_type pos)
    {
      return std::make_unique<node_comments>(pos);
    }

    static constexpr choicer_type choicer{ &get_name, &condition, &create };


    enum kind : std::size_t
    {
      kind_begin         = (1 << 0),
      kind_single_begin  = (1 << 1),
      kind_single_inside = (1 << 2),
      kind_single_end    = (1 << 3),

      was_begin         = (kind_single_begin),
      was_single_begin  = (kind_single_inside | kind_single_end),
      was_single_inside = (was_single_begin),
      was_single_end    = 0,
    };

    // props
    kind req{ kind_begin };

    // actions

    void parse(parser_state & st, response_type & resp, Char ch) override
    {
      using namespace std::string_literals;

      if( (req & kind_begin) != 0 )
      {
        if( ch == info::divide )
        {
          req = was_begin;
          return;
        }

        st.data.log->inform({
          log_messages::template comments_wrong_char<
            std::initializer_list<std::string>
          >({ "t_divide"s }),
          json_message_type::n_error,
          st.position.get()
        });
        st.after_fn = &parser_state::action_unwind;
        resp.change_status(json_message_codes::n_comments_wrong_char);
        return;
      }

      if( (req & kind_single_begin) != 0 )
      {
        if( ch == info::divide )
        {
          req = was_single_begin;
          return;
        }

        st.data.log->inform({
          log_messages::template comments_wrong_char<
            std::initializer_list<std::string>
          >({ "t_divide"s }),
          json_message_type::n_error,
          st.position.get()
        });
        st.after_fn = &parser_state::action_unwind;
        resp.change_status(json_message_codes::n_comments_wrong_char);
        return;
      }

      if( (req & kind_single_end) != 0 )
      {
        if( is_eq(ch, info::cr, info::lf) )
        {
          req = was_single_end;
          st.after_fn = &parser_state::action_up_only;
          return;
        }
      }
    }

    result_type get_result(parser_state & st, response_type & resp) override
    {
      throw exception_skip_result{ this->start_pos };
      return st.maker->make_null(
        this->start_pos
      );
    }
  };


} // end ns
