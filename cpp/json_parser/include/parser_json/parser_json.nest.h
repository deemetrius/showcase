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


} // end ns
namespace parser::detail {


  template <typename Char, typename Maker>
  struct nest_json
  {
    using result_type = Maker::result_type;
    using parser_state = nest_base<Char, Maker>::parser_state;
    using node_base = nest_base<Char, Maker>::node_base;
    using info = ksi::chars::info<Char>;
    using response_type = nest_base<Char, Maker>::response_type;


    class node_number;
    class node_top;


  }; // end nest


} // end ns
