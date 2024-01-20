#pragma once

#include "json.node.space.h"

#include "json.node.keyword.h"
#include "json.node.number.h"
#include "json.node.text.h"
#include "json.node.array.h"
#include "json.node.map.h"

namespace parser::detail {

  template <typename Char, typename Maker, typename Log_string>
  inline json_nest<Char, Maker, Log_string>::choicer_type const *
    json_nest<Char, Maker, Log_string>::find_from_all(json_params const * params, Char ch)
  {
    return choicer_type::template find< std::initializer_list<choicer_type const *> >(
      {
        &node_text::choicer,
        &node_number::choicer,
        &node_map::choicer,
        &node_array::choicer,
        &node_keyword::choicer
      },
      params, ch
    );
  }

  template <typename Char, typename Maker, typename Log_string>
  class json_nest<Char, Maker, Log_string>::node_top
    : public node_base
  {
  protected:

    index_t count_tokens{0};

    using node_base::node_base;

  public:
    static ptr_node create(Maker * maker, json_params const * params, pos_type pos)
    {
      return std::make_unique<node_top>(pos);
    }

    void parse(parser_state & st, response_type & resp, Char ch) override
    {
      if( node_space::condition(st.params, ch) )
      {
        return;
      }

      if( count_tokens > 0 )
      {
        st.after_fn = &parser_state::action_exit;
        return;
      }

      choicer_type const * it = find_from_all(st.params, ch);
      if( it == nullptr )
      {
        st.after_fn = &parser_state::action_exit;
        resp.change_status(json_message_codes::n_top_unexpected_symbol);
        return;
      }
      ++count_tokens;
      st.add_node(
        it->create( st.maker, st.params, st.position.get() )
      );
      st.skip_read();
      
    }

    result_type get_result(parser_state & st, response_type & resp) override
    {
      throw exception_skip_result{ this->start_pos };
      return st.maker->make_null(
        this->start_pos
      );
    }

    void put_result(result_type result, parser_state & st, response_type & resp) override
    {
      resp.value = result;
      st.after_fn = &parser_state::action_exit;
    }
  };

} // end ns