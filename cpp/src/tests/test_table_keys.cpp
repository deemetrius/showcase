
#include <simplesquirrel/simplesquirrel.hpp>
#include <iostream>

ssq::Object make_integer(ssq::VM & vm, SQInteger n)
{
  ssq::Array tmp = vm.newArray();
  tmp.push(n);
  return tmp.back<ssq::Object>();
}

struct Printer
{
  std::wostream * stream;

  template <typename Type>
  void operator () (Type v) const
  {
    (*stream) << v;
  }

  void operator () (ssq::Object const & v) const
  {
    (*stream) << v.getTypeStr();
  }
};

std::wostream & operator << (std::wostream & o, ssq::TableKey key)
{
  Printer printer = {&o};
  std::visit(printer, key);
  return o;
}

template <typename Result, typename Visitor>
Result visit(Visitor v, ssq::Object const & obj)
{
  if( obj.getType() == ssq::Type::INTEGER )
  {
    return v(obj.toInt());
  }
  else if( obj.getType() == ssq::Type::STRING )
  {
    return v(obj.toString());
  }
  return v(obj);
}

std::wostream & operator << (std::wostream & o, ssq::Object const & obj)
{
  Printer printer = { &o };
  visit<void>(printer, obj);
  return o;
}

int main()
{
  try
  {
    ssq::VM vm{ 1024, ssq::Libs::STRING | ssq::Libs::IO | ssq::Libs::MATH };
  
    ssq::Table tb = vm.newTable();
    tb.set(L"key1", make_integer(vm, 1));
    tb.set(L"key2", make_integer(vm, 2));
    tb.set(L"key0", make_integer(vm, 0));
    tb.set(11, make_integer(vm, 11));

    std::wcout << "getKeys():\n";
    for( ssq::Object const & it : tb.getKeys() )
    { 
      std::wcout << it << '\n';
    }

    std::wcout << "\ngetMap():\n";
    ssq::TableMap mp = tb.getMap();
    for( typename ssq::TableMap::value_type const & it : mp )
    {
      std::wcout << it.first << " : " << it.second << '\n';
    }
  }
  catch( ... )
  {
    std::cout << "Exception\n";
  }

  return 0;
}
