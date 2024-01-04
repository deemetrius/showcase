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

    result_type make_integer(integer n) const
    {
      array tmp(*vm_pointer);
      tmp.push(n);
      return tmp.back<ssq::Object>();
    }

    result_type make_floating(floating n) const
    {
      array tmp(*vm_pointer);
      tmp.push(n);
      return tmp.back<ssq::Object>();
    }
  };

}
