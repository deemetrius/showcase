#pragma once

#include "parser_base.h"
#include "parser_json.nest.h"
#include "parser_json.space.h"
#include "parser_json.keyword.h"
#include "parser_json.number.h"
#include "parser_json.text.h"
#include "parser_json.map.h"
#include "parser_json.top.h"

namespace parser {


  template <typename Maker>
  struct json
  {
    using result_type = Maker::result_type;
    using response_type = parser_response<result_type>;

    json_params params{};

    template <typename String>
    response_type from_string(Maker & maker, String source);
  };


  template <typename Maker>
  template <typename String>
  inline json<Maker>::response_type
    json<Maker>::from_string(Maker & maker, String source)
  {
    using reader_type = ksi::lib::string_reader<String>;
    using char_type = decltype( std::declval<reader_type>().read_char() );
    using state_type = detail::json_nest<char_type, Maker>::parser_state;
    using nest = detail::json_nest<char_type, Maker>;

    response_type response;

    state_type state{
      &maker,
      std::make_unique<reader_type>(source),
      &this->params
    };
    state.add_node(
      nest::node_top::create( &maker, &this->params, state.position.get() )
    );
    state.parser_loop(response);

    return response;
  }


} // end ns
