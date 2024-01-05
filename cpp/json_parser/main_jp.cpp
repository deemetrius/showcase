
#include <optional>
#include <string>
#include <memory>
#include <list>
#include <iostream>

#include <simplesquirrel/simplesquirrel.hpp>

#include "include/ksi_lib/chars.info.hpp"
#include "include/ksi_lib/files.position.hpp"
#include "include/ksi_lib/lib.string_reader.hpp"
#include "include/nut_maker.h"
#include "include/parser_json/parser_json.h"

int main()
{
  using text = std::string;
  using maker_type = nut::nut_maker<text>;
  using parser = parser::json<maker_type>;

  std::string json = "5.1231";

  ssq::VM vm{1024, ssq::Libs::STRING | ssq::Libs::IO | ssq::Libs::MATH};
  maker_type maker{ &vm };
  parser::response_type resp = parser::from_string(maker, json);

  std::cout << "src: " << json.size() << '\n';
  std::cout << "parsed: " << resp.position.char_pos << '\n';
  std::cout << "seems done\n";

  return 0;
}
