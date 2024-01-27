
#include <iostream>

#include "include/parser_json/json.h"
#include "include/nut_maker.h"
#include "include/lib_log/log_list.h"
//#include "include/ksi_lib/integer_cast.hpp"

struct api
{
  using maker_type = nut::nut_maker;//<text>;
  using parser_type = parser::json<maker_type>;

  using log_node_type = parser::json_log_node<std::string>;
  using log_type = lib_log::list<log_node_type>;

  static inline std::unique_ptr<ssq::VM> vm{};
  static inline log_type log{};

  static ssq::Object parse_json(ssq::sqstring json)
  {
    maker_type maker{ vm.get() };

    parser_type parser;
    parser.params.number.nan_from_dot_only = true;
    parser.params.number.infinity_from_dot_signed = true;
    parser.params.comments.single_line = true;
    parser_type::response_type resp = parser.from_string(maker, json, &log);

    if( resp.result.has_value() )
    {
      return resp.result.value();
    }

    return maker.just_make_null();
  }

  static void print(ssq::Object o)
  {
    switch( o.getType() )
    {
      case ssq::Type::BOOL :
      std::cout << std::boolalpha << o.toBool();
      break;

      case ssq::Type::INTEGER :
      std::cout << o.toInt();
      break;

      case ssq::Type::FLOAT:
      std::cout << o.toFloat();
      break;

      case ssq::Type::STRING:
      std::wcout << o.toString();
      break;

      default:
      std::wcout << o.getTypeStr();
    }
  }

  static void print_line(ssq::Object o)
  {
    print(o);
    std::cout << "\n";
  }

  static void show_log()
  {
    for( typename log_type::node_type const & it : log.starage )
    {
      maker_type::show_pos(it.data);
      std::cout << it.message << '\n';
    }
  }
};


int main()
{
  //std::cout << ksi::conv::numbers::integer_cast<std::int16_t>(32769ui32).value << '\n';
  
  //using text = std::string;
  try
  {
    ssq::sqstring json = LR"(
{
  "items": [null, true, -9223372036854775808],
  "123\" 1": +05.01250,
  "a": null,
  "sub": [{
    // ""
    "1" : 10,
    "2" : 20
  }]
} )";

    api::vm = std::make_unique<ssq::VM>(1024, ssq::Libs::STRING | ssq::Libs::IO | ssq::Libs::MATH);

    api::vm->addFunc(_SC("json_to_value"), api::parse_json);
    api::vm->addFunc(_SC("con_print"), api::print);
    api::vm->addFunc(_SC("con_print_line"), api::print_line);

    ssq::Object result = api::parse_json(json);

    ssq::Script sc_lib = api::vm->compileFile(L"./nuts/inspect.nut");
    ssq::Script sc_run = api::vm->compileFile(L"./nuts/inspect_test.nut");

    api::vm->run(sc_lib);
    api::vm->run(sc_run);

    api::show_log();

    std::cout << "seems done\n";
  }
  catch( ssq::CompileException const & e )
  {
    std::cout << "Compile exception: " << e.what();
  }
  catch( ssq::RuntimeException const & e )
  {
    std::cout << "Runtime exception: " << e.what();
  }
  catch( ... )
  {
    std::cout << "unknown exception\n";
  }
  
  return 0;
}
