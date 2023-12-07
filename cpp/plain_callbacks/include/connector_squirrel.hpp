
#include <simplesquirrel/simplesquirrel.hpp>

#include <array>
#include <optional>
#include <iostream>

namespace connector_squirrel {

  struct params
  {
    using text_type = ssq::sqstring;

    using function_type = std::optional<ssq::Function>;
    using function_result_type = ssq::Object;

    using vm_pass_type = ssq::VM &;
  };

  // callback

  struct callback
  {
    // data
    params::text_type name;
    params::function_type function;

    // methods

    bool find_in(params::vm_pass_type vm)
    {
      try
      {
        function = vm.findFunc( name.c_str() );
        return true;
      }
      catch( ... )
      {
        function.reset();
        return false;
      }
    }

    bool is_ready() const
    {
      return function.has_value();
    }

    template <typename ... Args>
    params::function_result_type
      operator () (params::vm_pass_type vm, Args && ... args)
    {
      return vm.callFunc(function.value(), vm, std::forward<Args>(args) ...);
    }
  };

  // callback interface

  struct interface
  {
    using integer = decltype( std::declval<ssq::Object>().toInt() );

    callback
      on_paint{ L"onPaint" },
      on_load { L"onLoad" },
      on_error{ L"onException" };

    void rescan(params::vm_pass_type vm)
    {
      on_paint.find_in(vm);
      on_load.find_in(vm);
      on_error.find_in(vm);
    }

    integer wrap_paint(params::vm_pass_type vm);

    static void check(const interface & self);
  };

  inline interface::integer
    interface::wrap_paint(params::vm_pass_type vm)
  {
    if( on_paint.is_ready() )
    {
      // calculate params

      ssq::Object res = on_paint(vm, 1);

      return res.toInt();
    }
    return -1;
  }

  inline void interface::check(const interface & self)
  {
    using namespace std::string_view_literals;

    std::array members{
      & interface::on_load,
      & interface::on_paint,
      & interface::on_error
    };
    for( auto it : members )
    {
      auto & m{ self.*it };
      std::wcout << '\t' << m.name << "() " << (m.is_ready() ? L"found"sv : L"missing"sv) << '\n';
    }
  }

  // end ns
}