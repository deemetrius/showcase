#pragma once


// returns milisec since day start
connector_squirrel::params::integer get_ms()
{
  std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
  auto ms = std::chrono::floor<std::chrono::milliseconds>(now);
  auto day = std::chrono::floor<std::chrono::days>(now);
  auto ret = ms - day;
  return ret.count();
}


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


print("passing callbacks")
local found_cnt = api.init_callbacks(pack) // pass callbacks
print( "count of found callbacks: " + found_cnt.tostring() + "\n" )

)raw" };

  static inline ssq::sqstring file_body_instance{
    LR"raw(
class something {
  value = 15

  onLoad = function ()
  {
    return "something::onLoad()" + " value= " + this.value.tostring()
  }
}

function something::onPaint(n)
{
  return "something::onPaint(" + n.tostring() + ")" + " value= " + this.value.tostring()
}


print("passing callbacks")
local found_cnt = api.init_callbacks( something() ) // pass callbacks
print( "count of found callbacks: " + found_cnt.tostring() + "\n" )

)raw" };

  static void go(interface& caller)
  {
    using namespace connector_squirrel;

    //std::shared_ptr<ssq::VM> vm_ptr =
    //  std::make_shared<ssq::VM>(1024, ssq::Libs::STRING | ssq::Libs::IO | ssq::Libs::MATH);
    //ssq::VM & vm = *vm_ptr;
    ssq::VM vm{ 1024, ssq::Libs::STRING | ssq::Libs::IO | ssq::Libs::MATH };

    on_leave guard{ [&caller]() { caller.reset(); } };

    ssq::Table api_table = vm.addTable(L"api");
    //interface caller;
    caller.bind_as(L"init_callbacks", api_table /* vm */);


    ssq::Script script_base = vm.compileSource(file_body_base.c_str());
    ssq::Script script_init = vm.compileSource(file_body_init.c_str());
    ssq::Script script_some = vm.compileSource(file_body_instance.c_str());


    vm.run(script_base);
    //caller.rescan_in(vm_ptr);
    caller.rescan_in(vm);


    check_found(caller);
    check_calls(caller, vm);


    std::cout << "\nGoing to rebind\n\n";
    vm.run(script_init);
    //caller.callbacks_change_argument(vm_ptr); // test


    check_found(caller);
    check_calls(caller, vm);


    std::cout << "\nRebind to instance\n\n";
    vm.run(script_some);
    //caller.callbacks_change_argument(vm_ptr); // test


    check_found(caller);
    check_calls(caller, vm);

    // end fn
  }

  static void check_found(const interface& caller)
  {
    using namespace connector_squirrel;
    using namespace std::string_view_literals;

    std::cout << "Searching squirrel functions\n";

    for (auto it : interface::for_bind)
    {
      auto& method{ caller.*it };
      std::wcout << '\t' << method.name << "() " << (method.is_ready() ? L"found"sv : L"missing"sv) << '\n';
    }
  }

  static void check_calls(interface& caller, connector_squirrel::params::vm_pass_type vm)
  {
    using namespace connector_squirrel;

    std::cout << "calling callbacks:\n";

    ssq::Object res;

    try
    {
      res = caller.on_load(vm);
      std::wcout << "\t" << caller.on_load.name << "() returns: " << res.toString() << "\n";
    }
    catch (const ssq::NotFoundException& e)
    {
      std::wcout << "\t" << e.what() << "()\n";
    }

    try
    {
      res = caller.on_paint(vm, 1);
      std::wcout << "\t" << caller.on_paint.name << "() returns: " << res.toString() << "\n";
    }
    catch (const ssq::NotFoundException& e)
    {
      std::wcout << "\t" << e.what() << "()\n";
    }

    try
    {
      res = caller.on_error(vm);
      std::wcout << "\t" << caller.on_error.name << "() returns: " << res.toString() << "\n";
    }
    catch (const ssq::NotFoundException& e)
    {
      std::wcout << "\t" << e.what() << "()\n";
    }
  }
};
