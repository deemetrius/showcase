
#include <simplesquirrel/simplesquirrel.hpp>

import connector_squirrel;

import <array>;
import <string_view>;
import <iostream>;

// callback interface

template <typename Callback>
struct interface
{
  using self_type = interface;

  using callback_type = Callback;
  using vm_pass_type = typename callback_type::vm_pass_type;

  using integer = decltype( std::declval<ssq::Object>().toInt() );

  callback_type
    on_paint{ L"onPaint" },
    on_load { L"onLoad" },
    on_error{ L"onException" };

  void rescan(vm_pass_type vm)
  {
    on_paint.find_in(vm);
    on_load.find_in(vm);
    on_error.find_in(vm);
  }

  integer wrap_paint(vm_pass_type vm);

  static void check(const self_type & self);
};

using squirrel_interface = interface<connector_squirrel::callback>;

// wrap aorund paint
squirrel_interface::integer
  squirrel_interface::wrap_paint(vm_pass_type vm)
{
  if( on_paint.is_ready() )
  {
    // prepare data, calculate params

    // and call fn
    ssq::Object res = on_paint(vm, 1);

    return res.toInt();
  }
  return -1;
}

// check callbacks
void squirrel_interface::check(const self_type & self)
{
  using namespace std::string_view_literals;

  std::array members{
    & self_type::on_load,
    & self_type::on_paint,
    & self_type::on_error
  };
  for( auto it : members )
  {
    auto & m{ self.*it };
    std::wcout << '\t' << m.name << "() " << (m.is_ready() ? L"found"sv : L"missing"sv) << '\n';
  }

  // end check fn
}

// test squirrel callbacks

void testing_squirrel_nuts()
{
  // .nut script 1
  static ssq::sqstring file_body{ LR"(
function onLoad()
{
  return 10;
}
)" };
  // .nut script 2
  static ssq::sqstring file_body_also{ LR"(
function onPaint(n)
{
  return 20 + n;
}
)" };

  // vm and compile
  ssq::VM vm(1024, ssq::Libs::STRING | ssq::Libs::IO | ssq::Libs::MATH);
  ssq::Script script = vm.compileSource( file_body.c_str() );
  ssq::Script script_also = vm.compileSource( file_body_also.c_str() );

  // run compiled
  vm.run(script);
  vm.run(script_also);

  // caller instance
  squirrel_interface caller;

  // going to scan callbacks
  std::cout << "Searching squirrel functions\n";
  caller.rescan(vm);

  // what was found?
  squirrel_interface::check(caller);

  std::cout << "calling callbacks:\n";

  // raise events
  ssq::Object res1 = caller.on_load(vm);
  ssq::Object res2 = caller.on_paint(vm, 1);

  // print results
  std::wcout << "\t" << caller.on_load.name << "() returns: " << res1.toInt() << "\n";
  std::wcout << "\t" << caller.on_paint.name << "() returns: " << res2.toInt() << "\n";

  try
  {
    ssq::Object res3 = caller.on_error(vm);
  }
  catch( const std::bad_optional_access & )
  {
    // callback was not found
    std::wcout << "\t" << caller.on_error.name << "() was not found\n";
  }

  // end testing function
}

// entry

int main()
{
  try
  {
    testing_squirrel_nuts();
  }
  catch( ... )
  {
    std::cout << "exception\n";
  }
  return 0;
}

#if 0
//import connector_some;

void testing()
{
  // подстановка конкретного коннектора в интерфейс вызова
  interface<connector_some::callback> caller;

  // виртуальная машина для тестирования
  some_script_engine::vm vm;

  std::cout << "search for callbacks in script\n";
  caller.rescan(vm);
}
#endif
