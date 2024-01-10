
#include <iostream>

#include <simplesquirrel/simplesquirrel.hpp>

#include "include/parser_json/parser_json.h"
#include "include/nut_maker.h"

int main()
{
  using text = std::string;
  using maker_type = nut::nut_maker<text>;
  using parser_type = parser::json<maker_type>;

  std::string json = R"(
{
  "items": [null, true, 55],
  "123\" 1": 15,
  "a": null
} )";

  ssq::VM vm{1024, ssq::Libs::STRING | ssq::Libs::IO | ssq::Libs::MATH};
  maker_type maker{ &vm };

  parser_type parser;
  parser.params.number.nan_only_dot = true;
  parser.params.number.infinity_sign_dot = true;
  parser_type::response_type resp = parser.from_string(maker, json);

  std::cout << "src: " << json.size() << '\n';
  std::cout << "parsed: " << resp.end_position.char_pos << '\n';
  std::cout << "status: " << resp.status << '\n';
  std::cout << "seems done\n";

  return 0;
}
