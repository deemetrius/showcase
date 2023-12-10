
#define _CRT_SECURE_NO_WARNINGS 1

#include <simplesquirrel/simplesquirrel.hpp>

#include <optional>
#include <memory>
#include <iostream>
#include <type_traits>
#include <cwchar>

namespace aux {

template <typename Type, typename ... Types>
constexpr inline bool any_of_v = (std::is_same_v<Type, Types> || ...);

// string convert

template <typename Result>
struct string_cast_fn;

template <>
struct string_cast_fn<std::wstring>
{
  std::wstring operator () (const std::string & str) const
  {
    std::mbstate_t state = std::mbstate_t();
    const char * src = str.c_str();
    std::size_t len = std::mbsrtowcs(nullptr, &src, 0, &state);
    std::wstring ret{ len, L'\0', std::allocator<wchar_t>{} };
    std::mbsrtowcs(&ret[0], &src, ret.size(), &state);
    return ret;
  }

  std::wstring operator () (std::wstring str) const
  {
    return str;
  }
};

template <>
struct string_cast_fn<std::string>
{
  std::string operator () (const std::wstring & str) const
  {
    std::mbstate_t state = std::mbstate_t();
    const wchar_t * src = str.c_str();
    std::size_t len = std::wcsrtombs(nullptr, &src, 0, &state);
    std::string ret{ len, '\0', std::allocator<char>{} };
    std::wcsrtombs(&ret[0], &src, ret.size(), &state);
    return ret;
  }

  std::string operator () (std::string str) const
  {
    return str;
  }
};

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

  static inline aux::string_cast_fn<text_type> string_to_sq;
};

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
  std::string name_plain;

  params::text_type name;
  params::function_type function;
  params::arg_type argument;

  callback(std::string p_name)
    : name_plain{ p_name }
    , name{ params::string_to_sq(name_plain) }
  {}

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
      throw ssq::NotFoundException( name_plain.c_str() );
    }
    return vm.callFunc(function.value(), argument->get(), std::forward<Args>(args) ...);
  }
};

// callback interface

struct interface
{
  using integer = decltype( std::declval<ssq::Object>().toInt() );

  callback
    on_paint{ "onPaint" },
    on_load { "onLoad" },
    on_error{ "onException" };

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
          case ssq::Type::INSTANCE : {
            throw ssq::TypeException( "not supported", "Table", object.getTypeStr() );
          }
          default:
            throw ssq::TypeException( "not supported", "Table", object.getTypeStr() );
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