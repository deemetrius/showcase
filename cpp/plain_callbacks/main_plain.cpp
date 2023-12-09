
#include "include/connector_squirrel.hpp"

#include <array>

struct tester
{
  static inline ssq::sqstring file_body_base{
    LR"raw(
function onLoad()
{
  return "onLoad()"
}

function onPaint(n)
{
  return "onPaint(" + n.tostring() + ")"
}
)raw" };

  static inline ssq::sqstring file_body_init{
    LR"raw(
pack <- {
  value = 5
  onLoad = function ()
  {
    return "pack::onLoad()" + " value= " + this.value.tostring()
  }
}

function pack::onPaint(n)
{
  return "pack::onPaint(" + n.tostring() + ")" + " value= " + this.value.tostring()
}

init_callbacks(pack)
)raw" };

  static void go()
  {
    using namespace connector_squirrel;

    // vm
    ssq::VM vm(1024, ssq::Libs::STRING | ssq::Libs::IO | ssq::Libs::MATH);

    // compile
    ssq::Script script_base = vm.compileSource( file_body_base.c_str() );
    ssq::Script script_init = vm.compileSource( file_body_init.c_str() );

    // run compiled
    vm.run(script_base);

    // caller instance
    interface caller;

    caller.bind_as(L"init_callbacks", vm);

    // going to scan callbacks
    caller.rescan_in(vm);

    // what was found?
    check_found(caller);
    check_calls(caller, vm);

    //
    std::cout << "\nGoing to rebind\n\n";
    vm.run(script_init);

    // what was found?
    check_found(caller);
    check_calls(caller, vm);

    // end testing fn
  }
  
  static void check_found(const connector_squirrel::interface & caller)
  {
    using namespace connector_squirrel;
    using namespace std::string_view_literals;

    std::array members{
      & interface::on_load,
      & interface::on_paint,
      & interface::on_error
    };

    std::cout << "Searching squirrel functions\n";

    for( auto it : members )
    {
      auto & m{ caller.*it };
      std::wcout << '\t' << m.name << "() " << (m.is_ready() ? L"found"sv : L"missing"sv) << '\n';
    }
  }

  static void check_calls(connector_squirrel::interface & caller, connector_squirrel::params::vm_pass_type vm)
  {
    using namespace connector_squirrel;

    std::cout << "calling callbacks:\n";

    // raise events
    ssq::Object res;

    try
    {
      res = caller.on_load(vm);
      std::wcout << "\t" << caller.on_load.name << "() returns: " << res.toString() << "\n";

      res = caller.on_paint(vm, 1);
      std::wcout << "\t" << caller.on_paint.name << "() returns: " << res.toString() << "\n";

      res = caller.on_error(vm);
      std::wcout << "\t" << caller.on_error.name << "() returns: " << res.toString() << "\n";
    }
    catch( const errors::no_callback & e )
    {
      // callback was not found
      std::wcout << "\t" << e.name << "() was not found\n";
    }
  }
};

// entry

int main()
{
  try
  {
    tester::go();
  }
  catch( ... )
  {
    std::cout << "exception\n";
  }
  return 0;
}
