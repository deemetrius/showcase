#pragma once

namespace parser {


  struct json_error_map_key_empty
  {
    ksi::files::position::data_type pos{-1, 0, 0};
  };


  struct json_status : public is_status
  {
    enum status : index_t
    {
      n_ok = 0,
      n_keyword_unknown,
      n_text_unclosed,
      n_map_unclosed,
      n_map_unexpected_symbol,
      n_map_internal_error,
    };
  };


  struct json_params
  {
    struct number_params
    {
      bool nan_only_dot{false};
      bool infinity_sign_dot{false};
    };

    index_t tab_size{4};
    number_params number{};
  };


} // end ns
namespace parser::detail {


  template <typename Char, typename Maker>
  class json_state_data
  {
  public:
    using result_type = Maker::result_type;
    using pos_type = ksi::files::position::data_type;

    static result_type make_null(Maker * maker, pos_type pos)
    {
      return maker->make_null(pos);
    }
    static result_type make_false(Maker * maker, pos_type pos)
    {
      return maker->make_bool(pos, false);
    }
    static result_type make_true(Maker * maker, pos_type pos)
    {
      return maker->make_bool(pos, true);
    }
    using fn_make = decltype(&make_null);
    using string = std::basic_string<Char>;
    using conv_string = ksi::conv::from_string::to<string>;
    using map_make_function = std::map<string, fn_make>;

    // data
    map_make_function const map_keywords{
      {conv_string{}("null"), &make_null},
      {conv_string{}("false"), &make_false},
      {conv_string{}("true"), &make_true}
    };
  };


  template <typename Char, typename Maker>
  struct json_nest
  {
    using result_type = Maker::result_type;
    using state_data = json_state_data<Char, Maker>;
    using nest = nest_base<Char, Maker, json_params, state_data>;
    using node_base = nest::node_base;
    using info = ksi::chars::info<Char>;
    using response_type = nest::response_type;
    using choicer_type = nest::choicer;
    using ptr_node = nest::ptr_node;
    using pos_type = ksi::files::position::data_type;
    using parser_state = nest::parser_state;


    class node_space;
    class node_keyword;
    class node_number;
    class node_text;
    class node_map;
    class node_top;
    class state_json;


    static choicer_type const * find_from_all(json_params const * params, Char ch);
  }; // end nest


} // end ns
