
#include <simplesquirrel/simplesquirrel.hpp>

#include <optional>
#include <memory>
#include <iostream>
#include <type_traits>

namespace aux {

template <typename Type, typename ... Types>
constexpr inline bool any_of_v = (std::is_same_v<Type, Types> || ...);

} // end ns

namespace connector_squirrel {

struct is_arg
{
  virtual ~is_arg() = default;

  virtual const ssq::Object & get() = 0;
};

struct params
{
  using text_type = ssq::sqstring;
  using raw_text_type = const SQChar *;

  using function_type = std::optional<ssq::Function>;
  using function_result_type = ssq::Object;

  using vm_pass_type = ssq::VM &;
  using table_pass_type = ssq::Table &;

  using arg_type = std::unique_ptr<is_arg>;
};

namespace errors {

  struct data_no_callback
  {
    params::text_type name;
  };

  struct is_error {};

  struct no_callback
    : public data_no_callback
    , public is_error
  {};

} // end ns

// args

struct arg_vm
  : public is_arg
{
  ssq::VM * handle;

  arg_vm(ssq::VM * vm_pointer)
    : handle{ vm_pointer }
  {}

  const ssq::Object & get() override
  {
    return *handle;
  }
};

struct arg_object
  : public is_arg
{
  ssq::Object value;

  arg_object(ssq::Object * pointer)
    : value{ *pointer }
  {}

  const ssq::Object & get() override
  {
    return value;
  }
};

// callback

struct callback
{
  // data
  params::text_type name;
  params::function_type function;
  params::arg_type argument;

  // todo: add constructor (ssq::Object)

  // methods

  template <typename Type>
  bool inner_find_in(Type & param)
  {
    try
    {
      function = param.findFunc( name.c_str() );
      return true;
    }
    catch( ... )
    {
      function.reset();
      return false;
    }
  }

  bool find_in(params::vm_pass_type param)
  {
    bool ret = inner_find_in(param);

    if( ret )
    {
      argument = std::make_unique<arg_vm>(& param);
    }

    return ret;
  }

  bool find_in(params::table_pass_type param)
  {
    bool ret = inner_find_in(param);

    if( ret )
    {
      argument = std::make_unique<arg_object>(& param);
    }

    return ret;
  }

  bool is_ready() const
  {
    return function.has_value();
  }

  template <typename ... Args>
  params::function_result_type
    operator () (params::vm_pass_type vm, Args && ... args)
  {
    if( is_ready() == false )
    {
      throw errors::no_callback{ name };
    }
    return vm.callFunc(function.value(), argument->get(), std::forward<Args>(args) ...);
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

  void bind_as(params::raw_text_type name, params::vm_pass_type vm)
  {
    vm.addFunc(name,
      [this](ssq::Object object){
        switch( object.getType() )
        {
          case ssq::Type::TABLE : {
            ssq::Table var{ object };
            this->rescan_in(var);
            break;
          }
        } // end switch
      }
    );
  } // end fn

  template <typename Type>
  void rescan_in(Type & param)
  {
    static_assert(aux::any_of_v<Type, ssq::VM, ssq::Table>);

    on_paint.find_in(param);
    on_load.find_in(param);
    on_error.find_in(param);
  }

  integer wrap_paint(params::vm_pass_type vm);
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

} // end ns