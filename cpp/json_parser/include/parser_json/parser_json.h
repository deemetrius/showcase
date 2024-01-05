#pragma once

#include "parser_base.h"
#include "parser_json.nest.h"
#include "parser_json.number.h"
#include "parser_json.top.h"

namespace parser {


  template <typename Maker>
  struct json
  {
    using result_type = Maker::result_type;
    using response_type = parser_response<result_type>;

    template <typename String>
    static response_type from_string(Maker const & maker, String source, index_t tab_size = 4);
  };


  template <typename Maker>
  template <typename String>
  inline json<Maker>::response_type
    json<Maker>::from_string(Maker const & maker, String source, index_t tab_size)
  {
    using reader_type = ksi::lib::string_reader<String>;
    using char_type = decltype( std::declval<reader_type>().read_char() );
    using state_type = detail::nest_base<char_type, Maker>::parser_state;
    using nest = detail::nest_json<char_type, Maker>;

    state_type state{ &maker };
    state.reader = std::make_unique<reader_type>(source);
    state.nodes.push_back( std::make_unique<nest::node_number>() );
    
    response_type response;
    ksi::files::position position{ tab_size };
    char_type ch{};

    while( state.reader->is_end() == false )
    {
      state.next_action(state, response, ch);
      state.parse(response, ch);
      if( state.is_recognized() )
      {
        position.recognized(ch);
      }
    }
    state_type::action_ask_parent(state, response, ch);
    response.position = position.get();
    return response;
  }


} // end ns
