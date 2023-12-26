
//#include "include/connector_squirrel.hpp"
//#include "include/interface.hpp"
//#include <chrono>
//#include "include/interface_tester.hpp"

#include <simplesquirrel/simplesquirrel.hpp>
#include <iostream>

struct wrap
{
  using integer = std::int64_t;

  mutable integer index;

  wrap(integer id)
    : index{ id }
  {}

  wrap(const wrap & other)
    : index{ std::exchange(other.index, -1) }
  {
    std::cout << "wrap copy: " << index << "\n";
  }

  // not required
  wrap(wrap && other)
    : index{ std::exchange(other.index, -1) }
  {
    std::cout << "wrap move: " << index << "\n";
  }

  ~wrap()
  {
    std::cout << "~wrap index: " << index << "\n";
  }

  static wrap* make(integer index)
  {
    return new wrap{ index };
  }

  static ssq::Class expose(ssq::VM & vm)
  {
    ssq::Class cls = vm.addClass(L"wrap", &make, true);

    cls.addVar(L"index", &wrap::index);

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

ssq::sqstring file_script = LR"(

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
    wrap::expose(vm);
    ssq::Function func = vm.addFunc(L"cpp_function", &cpp_function);
    ssq::Function func_ptr = vm.addFunc(L"cpp_function_ptr", &cpp_function_ptr);

    ssq::Script sf = vm.compileSource( file_script.c_str() );

    vm.run(sf);

    std::cout << "seems done\n";
  }
  catch( ... )
  {
    std::cout << "exception\n";
  }
  return 0;
}
