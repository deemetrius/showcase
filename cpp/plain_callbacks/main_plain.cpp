
#include "include/connector_squirrel.hpp"

void testing_squirrel_nuts()
{
  using namespace connector_squirrel;

  // .nut script
  static ssq::sqstring file_body{ LR"(
function onLoad()
{
  return 10;
}
)" };
  // .nut script also
  static ssq::sqstring file_body_also{ LR"(
function onPaint(n)
{
  return 20 + n;
}
)" };

  // vm
  ssq::VM vm(1024, ssq::Libs::STRING | ssq::Libs::IO | ssq::Libs::MATH);

  // compile
  ssq::Script script = vm.compileSource( file_body.c_str() );
  ssq::Script script_also = vm.compileSource( file_body_also.c_str() );

  // run compiled
  vm.run(script);
  vm.run(script_also);

  // caller instance
  interface caller;

  // going to scan callbacks
  std::cout << "Searching squirrel functions\n";
  caller.rescan(vm);

  // what was found?
  interface::check(caller);

  std::cout << "calling callbacks:\n";

  // raise events
  ssq::Object res1 = caller.on_load(vm);
  ssq::Object res2 = caller.on_paint(vm, 1);

  // print results
  std::wcout << "\t" << caller.on_load.name << "() returns: " << res1.toInt() << "\n";
  std::wcout << "\t" << caller.on_paint.name << "() returns: " << res2.toInt() << "\n";

  try
  {
    // absent event
    ssq::Object res3 = caller.on_error(vm);
  }
  catch( const std::bad_optional_access & )
  {
    // callback was not found
    std::wcout << "\t" << caller.on_error.name << "() was not found\n";
  }

  // end testing fn
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
