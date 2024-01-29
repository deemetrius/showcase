#pragma once

#include "json.path.h"
#include "parser_base.h"
#include "json.messages.h"

#include "../lib_aux/numeric_edges.h"

#include <cstdint>
#include <initializer_list>
#include <string>
#include <map>
#include <limits>

namespace parser {


  struct json_error_map_key_empty
  {
    ksi::files::position pos{-1, 0, 0};
  };


  struct json_params
  {
    struct number_params
    {
      bool e_notation_lowercase_letter{ false };
      bool nan_from_dot_only{ false };
      bool infinity_from_dot_signed{ false };
    };

    struct comment_params
    {
      bool single_line{ false };
    };

    index_t tab_size{ 4 };
    number_params number{};
    comment_params comments{};
  };


} // end ns
namespace parser::detail {


  template <typename Type, typename ... Args>
  constexpr inline bool is_eq(Type value, Args ... args)
  {
    return (
      (value == args) || ...
    );
  }

  template <typename Char, typename Maker, typename Log_string>
  class json_state_data
  {
  public:
    using result_type = Maker::result_type;
    using pos_type = ksi::files::position;
    using path_type = typename Maker::path_type;
    using log_pointer = json_log_pointer<Log_string>;

    static result_type make_null(Maker * maker, pos_type pos, path_type const & path)
    {
      return maker->make_null(path, pos);
    }
    static result_type make_false(Maker * maker, pos_type pos, path_type const & path)
    {
      return maker->make_bool(path, pos, false);
    }
    static result_type make_true(Maker * maker, pos_type pos, path_type const & path)
    {
      return maker->make_bool(path, pos, true);
    }
    using fn_make = decltype(&make_null);

    using string = std::basic_string<Char>;
    using conv_string = ksi::conv::string_cast::to<string>;
    using map_make_function = std::map<string, fn_make>;

    // props
    log_pointer log{ nullptr };
    path_type path{};
    map_make_function const map_keywords{
      { conv_string{}("null"), &make_null },
      { conv_string{}("false"), &make_false },
      { conv_string{}("true"), &make_true }
    };
  };


  template <typename Char, typename Maker, typename Log_string>
  struct json_nest
  {
    using result_type = Maker::result_type;
    using state_data = json_state_data<Char, Maker, Log_string>;
    using nest = nest_base<Char, Maker, json_params, state_data>;
    using node_base = nest::node_base;
    using info = ksi::chars::info<Char>;
    using response_type = nest::response_type;
    using choicer_type = nest::choicer;
    using ptr_node = nest::ptr_node;
    using pos_type = ksi::files::position;
    using parser_state = nest::parser_state;
    using chain_actions = nest::chain_actions;
    using log_conv_type = ksi::conv::string_cast::to<Log_string>;
    using log_messages = json_messages<Log_string, result_type>;
    using path_type = typename Maker::path_type;


    class node_top;
    class node_space;

    class node_comments;
    class node_keyword;
    class node_number;
    template <bool Is_key>
    class node_text;
    class node_array;
    class node_map;


    static choicer_type const * find_from_all(json_params const * params, Char ch);
    static choicer_type const * find_from_key(json_params const * params, Char ch);
  }; // end nest


} // end ns
