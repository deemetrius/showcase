#pragma once

#include "parser_json.nest.h"
#include "parser_json.space.h"
#include "parser_json.keyword.h"
#include "parser_json.number.h"
#include "parser_json.text.h"
#include "parser_json.map.h"

namespace parser::detail {

  template <typename Char, typename Maker>
  inline json_nest<Char, Maker>::choicer_type const *
    json_nest<Char, Maker>::find_from_all(json_params const * params, Char ch)
  {
    return choicer_type::template find< std::initializer_list<choicer_type const *> >(
      {
        &node_number::choicer,
        &node_text::choicer,
        &node_map::choicer,
        &node_keyword::choicer
      },
      params, ch
    );
  }

  template <typename Char, typename Maker>
  class json_nest<Char, Maker>::node_top
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
        resp.change_status(json_status::e_unexpected_symbol);
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
