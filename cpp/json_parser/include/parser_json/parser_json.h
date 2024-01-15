#pragma once

#include <optional>
#include <list>
#include <memory>

#include "../lib_log/log_base.h"
#include "parser_base.h"

#include <initializer_list>
#include <string>
#include <sstream>
#include <map>
#include <limits>

#include "../ksi_lib/conv.string.hpp"
#include "parser_json.top.h"

namespace parser {


  template <typename Maker>
  struct json
  {
    using result_type = Maker::result_type;
    using response_type = parser_response<result_type>;

    json_params params{};

    template <typename String, typename Log_string>
    response_type from_string(Maker & maker, String source, json_log_pointer<Log_string> log);
  };


  template <typename Maker>
  template <typename String, typename Log_string>
  inline json<Maker>::response_type
    json<Maker>::from_string(Maker & maker, String source, json_log_pointer<Log_string> log)
  {
    using reader_type = ksi::lib::string_reader<String>;
    using char_type = decltype( std::declval<reader_type>().read_char() );
    using nest = detail::json_nest<char_type, Maker, Log_string>;
    using state_type = nest::parser_state;

    response_type response;

    state_type state{
      &maker,
      std::make_unique<reader_type>(source),
      &this->params,
      log
    };
    state.add_node(
      nest::node_top::create( &maker, &this->params, state.position.get() )
    );
    state.parser_loop(response);

    return response;
  }


} // end ns
