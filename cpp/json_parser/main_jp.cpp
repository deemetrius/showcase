
#include <iostream>

#include "include/parser_json/parser_json.h"
#include "include/nut_maker.h"
#include "include/lib_log/log_list.h"

int main()
{
  using text = std::string;
  using maker_type = nut::nut_maker<text>;
  using parser_type = parser::json<maker_type>;

  using log_type = lib_log::list<std::string, parser::index_t, ksi::files::position::data_type>;
  log_type log;

  std::string json = R"(
{
  "items": [null, true, -9223372036854775808],
  "123\" 1": +15.25,
  "a": null
} )";

  ssq::VM vm{1024, ssq::Libs::STRING | ssq::Libs::IO | ssq::Libs::MATH};
  maker_type maker{ &vm };

  parser_type parser;
  parser.params.number.nan_only_dot = true;
  parser.params.number.infinity_sign_dot = true;
  parser_type::response_type resp = parser.from_string(maker, json, &log);

  std::cout << "src: " << json.size() << '\n';
  std::cout << "parsed: " << resp.end_position.char_pos << '\n';
  std::cout << "status: " << resp.status << "\n\n";

  for( typename log_type::node_type const & it : log.starage )
  {
    maker_type::show(it.data);
    std::cout << it.message << '\n';
  }

  std::cout << "seems done\n";

  return 0;
}
