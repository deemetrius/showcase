#pragma once

namespace parser::detail {


  template <typename Char, typename Maker>
  class nest_json<Char, Maker>::node_top
    : public node_base
  {
    void parse(parser_state & st, response_type & resp, Char ch) override
    {}
  };


} // end ns
