#pragma once

namespace parser {


  struct json_status
  {
    enum status
    {
      n_ok,
      n_number_double_dot
    };
  };


  struct json_params
  {
    index_t tab_size{4};
    bool number_dot_nan{false};
  };


} // end ns
namespace parser::detail {


  template <typename Char, typename Maker>
  struct nest_json
  {
    using result_type = Maker::result_type;
    using parser_state = nest_base<Char, Maker, json_params>::parser_state;
    using node_base = nest_base<Char, Maker, json_params>::node_base;
    using info = ksi::chars::info<Char>;
    using response_type = nest_base<Char, Maker, json_params>::response_type;


    class node_number;
    class node_top;


  }; // end nest


} // end ns
