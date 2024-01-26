#pragma once

#include "parser_json/json.path.h"
#include <simplesquirrel/simplesquirrel.hpp>

namespace nut {


  //template <typename String>
  struct nut_maker
  {
    using result_type = ssq::Object;
    using pos_type = ksi::files::position;

    using integer = SQInteger;
    using floating = SQFloat;
    using text = ssq::sqstring;
    using array = ssq::Array;
    using map = ssq::Table;

    using path_type = parser::json_path<text, integer>;

    enum flags_t : typename path_type::flags_type { path_flags = path_type::f_none };

    static void show_pos(pos_type pos)
    {
      std::cout
        << "[line " << pos.line
        << "; column " << pos.column
        << "; char " << pos.char_pos << "] ";
    }

    static void show(pos_type pos, path_type const & path)
    {
      std::cout
        << "[line " << pos.line
        << "; column " << pos.column
        << "; char " << pos.char_pos << "] ";
      std::wcout << path.full_path_string(path_flags) << '\n';
    }

    // props
    ssq::VM * vm_pointer{ nullptr };

    // ctor
    nut_maker(ssq::VM * p_vm)
      : vm_pointer{ p_vm }
    {}

    result_type make_null(path_type const & path, pos_type pos) const
    {
      show(pos, path);
      std::cout << "null\n";
      array tmp = vm_pointer->newArray();
      tmp.reset();
      return tmp;
    }

    result_type make_bool(path_type const & path, pos_type pos, bool b) const
    {
      show(pos, path);
      std::cout << "bool: " << std::boolalpha << b << '\n';
      array tmp = vm_pointer->newArray();
      tmp.push(b);
      return tmp.back<ssq::Object>();
    }

    result_type make_integer(path_type const & path, pos_type pos, integer n) const
    {
      show(pos, path);
      std::cout << "int: " << n << '\n';
      array tmp = vm_pointer->newArray();
      tmp.push(n);
      return tmp.back<ssq::Object>();
    }

    result_type make_floating(path_type const & path, pos_type pos, floating n) const
    {
      show(pos, path);
      std::cout << "float: " << n << '\n';
      array tmp = vm_pointer->newArray();
      tmp.push(n);
      return tmp.back<ssq::Object>();
    }

    result_type make_text(path_type const & path, pos_type pos, text str) const
    {
      show(pos, path);
      std::wcout << "text: " << str << '\n';
      array tmp = vm_pointer->newArray();
      tmp.push(str);
      return tmp.back<ssq::Object>();
    }

    result_type make_text_key(path_type & path, pos_type pos, text str) const
    {
      path.append(str);
      show(pos, path);
      std::wcout << "text: " << str << '\n';
      array tmp = vm_pointer->newArray();
      tmp.push(str);
      return tmp.back<ssq::Object>();
    }

    array make_array(path_type const & path, pos_type pos) const
    {
      show(pos, path);
      std::cout << "array\n";
      return vm_pointer->newArray();
    }

    map make_map(path_type const & path, pos_type pos) const
    {
      show(pos, path);
      std::cout << "map\n";
      return vm_pointer->newTable();
    }

    void array_insert(array & ar, integer index, result_type value, path_type const & path)
    {
      std::wcout << path.full_path_string(path_flags) << '\n';
      std::cout << "array append: " << index << '\n';
      ar.push(value);
    }

    void map_insert(map & mp, result_type key, result_type value, path_type const & path)
    {
      std::wcout << path.full_path_string(path_flags) << '\n';
      std::cout << "map insert: ";
      if( key.getType() != ssq::Type::STRING )
      {
        std::cout << "wrong key\n";
        throw ssq::TypeException(
          "Wrong Table key type",
          "string",
          key.getTypeStr()
        );
      }
      text str_key = key.toString();
      std::wcout << str_key << '\n';
      mp.set(str_key.c_str(), value);
    }
  };


} // end ns
