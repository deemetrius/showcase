#pragma once

namespace parser {


  struct json_status
  {
    enum status
    {
      n_ok = 0
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
    using state = nest::state;


    class node_number;
    class node_top;


  }; // end nest


} // end ns
