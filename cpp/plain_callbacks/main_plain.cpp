
//#include "include/connector_squirrel.hpp"
//#include "include/interface.hpp"
//#include <chrono>
//#include "include/interface_tester.hpp"

#include <string>
#include <simplesquirrel/simplesquirrel.hpp>
#include <iostream>
#include <memory>

struct my_object
{
  ~my_object()
  {
    std::cout << "~my_object\n";
  }
};

struct wrap
{
  using integer = std::int64_t;

  mutable integer index;
  std::shared_ptr<my_object> ptr;

  wrap(integer id)
    : index{ id }
    , ptr{ std::make_shared<my_object>() }
  {}

  integer get_index() const
  {
    return index;
  }

  wrap(const wrap & other)
    : index{ std::exchange(other.index, -1) }
    , ptr{ other.ptr }
  {
    std::cout << "wrap copy: " << index << "\n";
  }

  // not required
  wrap(wrap && other)
    : index{ std::exchange(other.index, -1) }
    , ptr{ other.ptr }
  {
    std::cout << "wrap move: " << index << "\n";
  }

  ~wrap()
  {
    std::cout << "~wrap index: " << index << "\n";
  }

  static wrap * make(integer index)
  {
    return new wrap{ index };
  }

  static ssq::Class expose(ssq::VM & vm)
  {
    ssq::Class cls = vm.addClass(L"wrap", &make, true);

    //cls.addVar(L"index", &wrap::index);
    cls.addFunc(L"index", &wrap::get_index);

    return cls;
  }

};

wrap cpp_function(wrap::integer index)
{
  return wrap{ index };
}

wrap * cpp_function_ptr(wrap::integer index)
{
  return new wrap{ index };
}

ssq::VM * hvm{ nullptr };

ssq::Array make_array()
{
  std::vector vec{ wrap{5}, wrap{6} };
  ssq::Array ret = hvm->newArray(vec);
  ret.push( wrap{1} );
  return ret;
}

ssq::sqstring file_script_was = LR"(

local var = wrap(1)
slot <- var

print( "var: " + var.index.tostring() )
print( "slot: " + slot.index.tostring() )

var = null

function fn(param)
{
  print( "param: " + param.index.tostring() )
  //slot2 <- param
  param.index += 10
  param = null
}

fn(slot)
print( "slot: " + slot.index.tostring() )

slot = cpp_function(20)
print( "slot: " + slot.index.tostring() )
//slot = null

)";

ssq::sqstring file_script = LR"(
//local zz = (1/0)
local var = wrap(1)
slot <- var

print( "var: " + var.index().tostring() )
print( "slot: " + slot.index().tostring() )

var = null

function fn(param)
{
  print( "param: " + param.index().tostring() )
  //slot2 <- param
  //param.index += 10
  param = null
}

fn(slot)
print( "slot: " + slot.index().tostring() )

slot = cpp_function(20)
print( "slot: " + slot.index().tostring() )
//slot = null

local arr = get_array()
print( arr[0].index() )

)";

int main()
{
  std::setlocale(LC_ALL, "rus");
  try
  {
    //interface caller;
    //tester::go(caller);
    //std::cout << "\n * run again * \n\n";
    //tester::go(caller);

    ssq::VM vm{ 1024, ssq::Libs::STRING | ssq::Libs::IO | ssq::Libs::MATH };
    hvm = &vm;
    wrap::expose(vm);
    ssq::Function func = vm.addFunc(L"cpp_function", &cpp_function);
    //ssq::Function func_ptr = vm.addFunc(L"cpp_function_ptr", &cpp_function_ptr);
    ssq::Function func_arr = vm.addFunc(L"get_array", &make_array);

    ssq::Script sf = vm.compileSource( file_script.c_str() );

    vm.run(sf);
    std::cout << "\nrun again\n";

    vm.run(sf);
    std::cout << "seems done\n";
  }
  catch( std::runtime_error const & e )
  {
    std::cout << "std::runtime_error ~ " << e.what() << '\n';
  }
  catch( ssq::RuntimeException const & e )
  {
    std::cout << "ssq::RuntimeException ~ " << e.what() << '\n';
  }
  catch( ... )
  {
    std::cout << "exception\n";
  }
  return 0;
}
