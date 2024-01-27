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

    // props
    ssq::VM * vm_pointer{ nullptr };
    bool debug_show_values{ false };
    bool debug_show_path{ false };
    bool debug_show_add_members{ false };

    void show(pos_type pos, path_type const & path) const
    {
      if( debug_show_path )
      {
        std::cout
          << "[line " << pos.line
          << "; column " << pos.column
          << "; char " << pos.char_pos << "] ";
        std::wcout << path.full_path_string(path_flags) << '\n';
      }
    }

    // ctor
    nut_maker(ssq::VM * p_vm)
      : vm_pointer{ p_vm }
    {}

    result_type just_make_null() const
    {
      array tmp = vm_pointer->newArray();
      tmp.reset();
      return tmp;
    }

    result_type make_null(path_type const & path, pos_type pos) const
    {
      show(pos, path);
      if( debug_show_values )
      {
        std::cout << "null\n";
      }
      return just_make_null();
    }

    result_type make_bool(path_type const & path, pos_type pos, bool b) const
    {
      show(pos, path);
      if( debug_show_values )
      {
        std::cout << "bool: " << std::boolalpha << b << '\n';
      }
      array tmp = vm_pointer->newArray();
      tmp.push(b);
      return tmp.back<ssq::Object>();
    }

    result_type make_integer(path_type const & path, pos_type pos, integer n) const
    {
      show(pos, path);
      if( debug_show_values )
      {
        std::cout << "int: " << n << '\n';
      }
      array tmp = vm_pointer->newArray();
      tmp.push(n);
      return tmp.back<ssq::Object>();
    }

    result_type make_floating(path_type const & path, pos_type pos, floating n) const
    {
      show(pos, path);
      if( debug_show_values )
      {
        std::cout << "float: " << n << '\n';
      }
      array tmp = vm_pointer->newArray();
      tmp.push(n);
      return tmp.back<ssq::Object>();
    }

    result_type make_text(path_type const & path, pos_type pos, text str) const
    {
      show(pos, path);
      if( debug_show_values )
      {
        std::wcout << "text: " << str << '\n';
      }
      array tmp = vm_pointer->newArray();
      tmp.push(str);
      return tmp.back<ssq::Object>();
    }

    result_type make_text_key(path_type & path, pos_type pos, text str) const
    {
      path.append(str);
      show(pos, path);
      if( debug_show_values )
      {
        std::wcout << "text: " << str << '\n';
      }
      array tmp = vm_pointer->newArray();
      tmp.push(str);
      return tmp.back<ssq::Object>();
    }

    array make_array(path_type const & path, pos_type pos) const
    {
      show(pos, path);
      if( debug_show_values )
      {
        std::cout << "array\n";
      }
      return vm_pointer->newArray();
    }

    map make_map(path_type const & path, pos_type pos) const
    {
      show(pos, path);
      if( debug_show_values )
      {
        std::cout << "map\n";
      }
      return vm_pointer->newTable();
    }

    void array_insert(array & ar, integer index, result_type value, path_type const & path)
    {
      if( debug_show_path )
      {
        std::wcout << "path: " << path.full_path_string(path_flags) << '\n';
      }
      if( debug_show_add_members )
      {
        std::cout << "array append: " << index << '\n';
      }
      ar.push(value);
    }

    void map_insert(map & mp, result_type key, result_type value, path_type const & path)
    {
      if( debug_show_path )
      {
        std::wcout << "path: " << path.full_path_string(path_flags) << '\n';
      }
      if( debug_show_add_members )
      {
        std::cout << "map insert: ";
      }
      if( key.getType() != ssq::Type::STRING )
      {
        if( debug_show_add_members )
        {
          std::cout << "wrong key\n";
        }
        throw ssq::TypeException(
          "Wrong Table key type",
          "string",
          key.getTypeStr()
        );
      }
      text str_key = key.toString();
      if( debug_show_add_members )
      {
        std::wcout << str_key << '\n';
      }
      mp.set(str_key.c_str(), value);
    }
  };


} // end ns
