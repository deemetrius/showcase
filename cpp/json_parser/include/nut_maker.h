#pragma once

namespace nut {

  using integer = std::intptr_t;

  template <typename String>
  struct nut_maker
  {
    using result_type = ssq::Object;
    using pos_type = ksi::files::position::data_type;

    static void show(pos_type pos)
    {
      std::cout << "[line " << pos.line << "; column " << pos.column << "; char " << pos.char_pos << "] ";
    }

    using integer = SQInteger;
    using floating = SQFloat;
    using text = ssq::sqstring;
    using array = ssq::Array;
    using map = ssq::Table;

    ssq::VM * vm_pointer{ nullptr };

    nut_maker(ssq::VM * p_vm)
      : vm_pointer{ p_vm }
    {}

    result_type make_null(pos_type pos) const
    {
      show(pos);
      std::cout << "null\n";
      array tmp = vm_pointer->newArray();
      tmp.reset();
      return tmp;
    }

    result_type make_integer(pos_type pos, integer n) const
    {
      show(pos);
      std::cout << "int: " << n << '\n';
      array tmp = vm_pointer->newArray();
      tmp.push(n);
      return tmp.back<ssq::Object>();
    }

    result_type make_floating(pos_type pos, floating n) const
    {
      show(pos);
      std::cout << "float: " << n << '\n';
      array tmp = vm_pointer->newArray();
      tmp.push(n);
      return tmp.back<ssq::Object>();
    }

    result_type make_text(pos_type pos, text str) const
    {
      show(pos);
      std::wcout << "text: " << str << '\n';
      array tmp = vm_pointer->newArray();
      tmp.push(str);
      return tmp.back<ssq::Object>();
    }

    map make_map(pos_type pos) const
    {
      show(pos);
      std::cout << "map\n";
      return vm_pointer->newTable();
    }
  };

}
