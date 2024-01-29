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

    static ptr_node create(
      Maker * maker,
      json_params const * params,
      pos_type start_pos,
      state_data const & data
    )
    {
      return std::make_unique<node_comments>(start_pos);
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

        log_messages{ st.data.log, &resp, st.position.get() }.comments_wrong_char<>(
          { "Divide"s }
        );
        st.after_fn = &chain_actions::chain_unwind;
        return;
      }

      if( (req & kind_single_begin) != 0 )
      {
        if( ch == info::divide )
        {
          req = was_single_begin;
          return;
        }

        log_messages{ st.data.log, &resp, st.position.get() }.comments_wrong_char<>(
          { "Divide"s }
        );
        st.after_fn = &chain_actions::chain_unwind;
        return;
      }

      if( (req & kind_single_end) != 0 )
      {
        if( is_eq(ch, info::cr, info::lf) )
        {
          req = was_single_end;
          st.after_fn = &chain_actions::chain_up_only;
          return;
        }
      }
    }

    result_type get_result(parser_state & st, response_type & resp) override
    {
      throw exception_skip_result{ this->start_pos };
      return st.maker->make_null(
        st.data.path,
        this->start_pos
      );
    }
  };


} // end ns
