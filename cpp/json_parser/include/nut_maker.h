#pragma once

namespace nut {

  using integer = std::intptr_t;

  template <typename String>
  struct nut_maker
  {
    using result_type = ssq::Object;

    using integer = SQInteger;
    using floating = SQFloat;
    using text = ssq::sqstring;
    using array = ssq::Array;
    using map = ssq::Table;

    ssq::VM * vm_pointer{ nullptr };

    nut_maker(ssq::VM * p_vm)
      : vm_pointer{ p_vm }
    {}

    result_type make_null() const
    {
      array tmp = vm_pointer->newArray();
      tmp.reset();
      return tmp;
    }

    result_type make_integer(integer n) const
    {
      std::cout << "int: " << n << '\n';
      array tmp = vm_pointer->newArray();
      tmp.push(n);
      return tmp.back<ssq::Object>();
    }

    result_type make_floating(floating n) const
    {
      std::cout << "float: " << n << '\n';
      array tmp = vm_pointer->newArray();
      tmp.push(n);
      return tmp.back<ssq::Object>();
    }
  };

}
