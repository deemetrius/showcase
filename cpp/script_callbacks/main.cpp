
#include <simplesquirrel/simplesquirrel.hpp>

import connector_squirrel;
import connector_test;

import <array>;
import <string_view>;
import <iostream>;

template <typename Callback>
struct interface
{
  using callback_type = Callback;
  using vm_pass_type = typename callback_type::vm_pass_type;

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

  int wrap_paint(vm_pass_type vm)
  {
    if( on_paint.is_ready() )
    {
      // prepare something

      // and call fn
      ssq::Object res = on_paint(vm, 1);

      return res.toInt();
    }
    return -1;
  }

  void check()
  {
    using namespace std::string_view_literals;
    std::array members{
      & interface::on_load,
      & interface::on_paint,
      & interface::on_error
    };
    for( auto it : members )
    {
      auto & m{ this->*it };
      std::wcout << '\t' << m.name << "() " << (m.is_ready() ? L"found"sv : L"missing"sv) << '\n';
    }
  }
};

// test squirrel callbacks

void testing_sq()
{
  static ssq::sqstring file_body{ LR"(
function onLoad()
{
  return 10;
}
)" };
  static ssq::sqstring file_body_also{ LR"(
function onPaint(n)
{
  return 20 + n;
}
)" };

  ssq::VM vm(1024, ssq::Libs::STRING | ssq::Libs::IO | ssq::Libs::MATH);
  ssq::Script script = vm.compileSource( file_body.c_str() );
  ssq::Script script_also = vm.compileSource( file_body_also.c_str() );

  vm.run(script);
  vm.run(script_also);

  interface<connector_scquirrel::callback> caller;
  std::cout << "Searching squirrel functions\n";
  caller.rescan(vm);
  caller.check();

  std::cout << "calling callbacks:\n";

  ssq::Object res1 = caller.on_load(vm);
  std::wcout << "\t" << caller.on_load.name << "() returns: " << res1.toInt() << "\n";

  ssq::Object res2 = caller.on_paint(vm, 1);
  std::wcout << "\t" << caller.on_paint.name << "() returns: " << res2.toInt() << "\n";

  try
  {
    ssq::Object res3 = caller.on_error(vm);
  }
  catch( const std::bad_optional_access & )
  {
    std::wcout << "\t" << caller.on_error.name << "() is not ready\n";
  }
}

// entry

int main()
{
  try
  {
    testing_sq();
  }
  catch( ... )
  {
    std::cout << "exception\n";
  }
  return 0;
}

#if 0
void testing()
{
  // подстановка конкретного коннектора в интерфейс вызова
  interface<connector_test::callback> caller;

  // виртуальная машина для тестирования
  test_script_engine::vm vm;

  std::cout << "search for callbacks in script\n";
  caller.rescan(vm);

  // interface<> ~ можно тиражировать для произвольных коннекторов
  // к прочим скриптовым движкам
  // через параметр шаблона
}
#endif
