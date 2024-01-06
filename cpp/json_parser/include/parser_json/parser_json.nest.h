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
  struct nest_json
  {
    using result_type = Maker::result_type;
    using nest = nest_base<Char, Maker, json_params>;
    using parser_state = nest::parser_state;
    using node_base = nest::node_base;
    using info = ksi::chars::info<Char>;
    using response_type = nest::response_type;
    using choicer_type = nest::choicer;
    using ptr_node = nest::ptr_node;
    using pos_type = ksi::files::position::data_type;


    class node_space;
    class node_number;
    class node_text;
    class node_map;
    class node_top;
  }; // end nest


} // end ns
