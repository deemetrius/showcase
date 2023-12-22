
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
  using integer = decltype( std::declval<ssq::Object>().toInt() );

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

/*
struct arg_vm_ptr
  : public is_arg
{
  std::weak_ptr<ssq::VM> ptr_weak;

  arg_vm_ptr(std::weak_ptr<ssq::VM> ptr)
    : ptr_weak{ ptr }
  {}

  const ssq::Object & get() override
  {
    if( ptr_weak.expired() )
    {
      throw ssq::Exception("VM pointer is empty");
    }
    std::shared_ptr<ssq::VM> ptr = ptr_weak.lock();
    return *ptr;
  }
};
*/

struct arg_vm
  : public is_arg
{
  std::weak_ptr<ssq::VM> ptr;
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


struct callback
{
  std::string name_plain;


  params::text_type name;
  params::function_type function;
  params::arg_type argument;


  callback(std::string p_name)
    : name_plain{ p_name }
    , name{ params::string_to_sq(name_plain) }
  {}


  template <typename Type>
  bool inner_find_in(Type & param)
  {
    try
    {
      function = param.findFunc( name.c_str() );
      return true;
    }
    catch( const ssq::NotFoundException & )
    {
      function.reset();
      return false;
    }
  }

  /*
  bool find_in(std::shared_ptr<ssq::VM> param)
  {
    bool ret = inner_find_in(*param);

    if( ret )
    {
      argument = std::make_unique<arg_vm_ptr>(param);
    }

    return ret;
  }
  */

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
  params::function_result_type operator ()
    (params::vm_pass_type vm, Args && ... args)
  {
    if( is_ready() == false )
    {
      throw ssq::NotFoundException( name_plain.c_str() );
    }
    return vm.callFunc(function.value(), argument->get(), std::forward<Args>(args) ...);
  }

  void reset()
  {
    function.reset();
    argument.reset();
  }
};

// interface base

template <typename Derived>
struct is_interface
{
  Derived * derived()
  {
    return static_cast<Derived *>(this);
  }

  template <typename Type>
  void bind_as(params::raw_text_type name, Type & where)
  {
    static_assert( aux::any_of_v<Type, ssq::VM, ssq::Table>,
      "bind_as() param should be: ssq::Table (or) ssq::VM"
    );

    where.addFunc(name,
      [self = derived()](ssq::Object object) -> params::integer {
        switch( object.getType() )
        {
        case ssq::Type::TABLE : {
          ssq::Table var{ object };
          return self->rescan_in(var);
        }
        case ssq::Type::INSTANCE : {
          return -1;
        }
        default:
          return -1;
        }
      }
    );

  }

  template <typename Type>
  params::integer rescan_in(Type & param)
  {
    static_assert(aux::any_of_v<Type, ssq::VM /*std::shared_ptr<ssq::VM>*/, ssq::Table>);

    params::integer count{ 0 };

    for( callback Derived::* it : Derived::for_bind )
    {
      count += (derived()->*it).find_in(param);
    }

    return count;
  }

  /*
  void callbacks_change_argument(std::shared_ptr<ssq::VM> vm_ptr)
  {
    for( callback Derived::* it : Derived::for_bind )
    {
      (derived()->*it).argument = std::make_unique<arg_vm_ptr>(vm_ptr);
    }
  }
  */

  void callbacks_change_argument(ssq::VM & vm)
  {
    for( callback Derived::* it : Derived::for_bind )
    {
      (derived()->*it).argument = std::make_unique<arg_vm>(& vm);
    }
  }

  void callbacks_change_argument(ssq::Table & tb)
  {
    for( callback Derived::* it : Derived::for_bind )
    {
      (derived()->*it).argument = std::make_unique<arg_object>(& tb);
    }
  }

  void reset()
  {
    for( callback Derived::* it : Derived::for_bind )
    {
      (derived()->*it).reset();
    }
  }
};

struct on_leave
{
  std::function<void ()> fn;
  ~on_leave()
  {
    fn();
  }
};

} // end ns
