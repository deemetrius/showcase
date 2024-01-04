
#include <list>
#include <string>

#include <simplesquirrel/simplesquirrel.hpp>

#include "include/ksi_lib/chars.info.hpp"
#include "include/ksi_lib/files.position.hpp"
#include "include/nut_maker.h"
#include "include/json_parser.h"

int main()
{
  using text = std::string;
  using maker_type = nut::nut_maker<text>;
  std::string json = "123";
  maker_type maker;
  parser::json::from_string(maker, json);

  return 0;
}
