
#include <iostream>

#include "include/parser_json/json.h"
#include "include/nut_maker.h"
#include "include/lib_log/log_list.h"
//#include "include/ksi_lib/integer_cast.hpp"

int main()
{
  //std::cout << ksi::conv::numbers::integer_cast<std::int16_t>(32769ui32).value << '\n';
  
  //using text = std::string;
  try
  {
    using maker_type = nut::nut_maker;//<text>;
    using parser_type = parser::json<maker_type>;

    using log_node_type = parser::json_log_node<std::string>;
    using log_type = lib_log::list<log_node_type>;
    log_type log;

    std::string json = R"(
{
  "items": [null, true, -9223372036854775808],
  "123\" 1": +05.01250,
  "a": null,
  "sub": [{
    "1" : 1,
    "2" : 2
  }]
} )";

    ssq::VM vm{ 1024, ssq::Libs::STRING | ssq::Libs::IO | ssq::Libs::MATH };
    maker_type maker{ &vm };

    parser_type parser;
    parser.params.number.nan_from_dot_only = true;
    parser.params.number.infinity_from_dot_signed = true;
    parser.params.comments.single_line = true;
    parser_type::response_type resp = parser.from_string(maker, json, &log);

    std::cout << "src: " << json.size() << '\n';
    std::cout << "parsed: " << resp.end_position.char_pos << '\n';
    std::cout << "status: " << resp.status << "\n\n";

    for( typename log_type::node_type const & it : log.starage )
    {
      maker_type::show_pos(it.data);
      std::cout << it.message << '\n';
    }
  }
  catch( ... )
  {
    std::cout << "exception\n";
  }
  std::cout << "seems done\n";
  
  return 0;
}
