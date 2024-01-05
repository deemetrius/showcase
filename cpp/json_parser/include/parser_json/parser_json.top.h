#pragma once

namespace parser::detail {


  template <typename Char, typename Maker>
  class nest_json<Char, Maker>::node_top
    : public node_base
  {
  protected:
    static constexpr std::array<choicer_type const *, 1> choicers{
      &node_number::choicer
    };

    std::optional<result_type> value;
    index_t count_tokens{0};

  public:

    static state create(json_params const * params)
    {
      return std::make_unique<node_top>();
    }

    void parse(parser_state & st, response_type & resp, Char ch) override
    {
      if( count_tokens > 0 )
      {
        resp.value = value;
        st.next_action = &parser_state::action_exit;
        return;
      }
      for( choicer_type const * it : choicers )
      {
        if( it->condition(st.params, ch) )
        {
          ++count_tokens;
          st.add_node( it->create(st.params) );
          st.next_action = &parser_state::action_none;
          return;
        }
      }
      st.next_action = &parser_state::action_exit;
    }
    
    result_type get_result(parser_state & st) override
    {
      if( value.has_value() )
      {
        return value.value();
      }
      return st.maker_pointer->make_null();
    }

    void put_result(result_type result) override
    {
      value = result;
    }
  };


} // end ns
