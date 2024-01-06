#pragma once

namespace parser::detail {

  template <typename Char, typename Maker>
  inline nest_json<Char, Maker>::choicer_type const *
    nest_json<Char, Maker>::find_from_all(json_params const * params, Char ch)
  {
    return choicer_type::template find< std::initializer_list<choicer_type const *> >(
      {
        &node_number::choicer,
        &node_text::choicer,
        &node_map::choicer
      },
      params, ch
    );
  }

  template <typename Char, typename Maker>
  class nest_json<Char, Maker>::node_top
    : public node_base
  {
  protected:
    /*static constexpr std::array<choicer_type const *, 3> choicers{
      &node_number::choicer
    , &node_text::choicer
    , &node_table::choicer
    };*/

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
      /*for( choicer_type const * it : choicers )
      {
        if( it->condition(st.params, ch) )
        {
          if( it->type > 0 ) { ++count_tokens; }
          st.add_node(
            it->create( st.maker, st.params, st.position.get() )
          );
          st.skip_read();
          return;
        }
      }
      st.after_fn = &parser_state::action_exit;
      resp.change_status(json_status::e_unexpected_symbol);*/
    }
    
    result_type get_result(parser_state & st) override
    {
      throw skip_result{ this->start_pos };
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
