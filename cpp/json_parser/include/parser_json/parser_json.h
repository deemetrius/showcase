#pragma once

#include "parser_base.h"
#include "parser_json.nest.h"
#include "parser_json.space.h"
#include "parser_json.number.h"
#include "parser_json.top.h"

namespace parser {


  template <typename Maker>
  struct json
  {
    using result_type = Maker::result_type;
    using response_type = parser_response<result_type>;

    json_params params{};

    template <typename String>
    response_type from_string(Maker const & maker, String source);
  };


  template <typename Maker>
  template <typename String>
  inline json<Maker>::response_type
    json<Maker>::from_string(Maker const & maker, String source)
  {
    using reader_type = ksi::lib::string_reader<String>;
    using char_type = decltype( std::declval<reader_type>().read_char() );
    using state_type = detail::nest_base<char_type, Maker, json_params>::parser_state;
    using nest = detail::nest_json<char_type, Maker>;

    state_type state{ &maker, &this->params };
    state.reader = std::make_unique<reader_type>(source);
    state.add_node(
      nest::node_top::create( state.params, state.position.get() )
    );
    
    response_type response;
    char_type ch{};

    for( ;; )
    {
      if( state.reader->is_end() )
      {
        state_type::action_ask_parent(state, response, ch);
        break;
      }
      state.next_action(state, response, ch);

      if( state.empty() ) { break; }
      state.parse(response, ch);
    }

    response.position = state.position.get();
    return response;
  }


} // end ns
