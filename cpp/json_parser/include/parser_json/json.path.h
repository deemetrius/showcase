#pragma once

#include "parser_std.h"
#include "../ksi_lib/conv.string.hpp"

#include <vector>
#include <sstream>

namespace parser {


  struct flags_has_all
  {
    size_t value{ 0 };
  };

  struct flags_has_any
  {
    size_t value{ 0 };
  };

  struct flags_absent_all
  {
    size_t value{ 0 };
  };

  constexpr inline bool operator * (size_t flags, flags_has_all has)
  {
    return ((flags & has.value) == has.value);
  }

  constexpr inline bool operator * (size_t flags, flags_has_any has)
  {
    return ((flags & has.value) != 0);
  }

  constexpr inline bool operator * (size_t flags, flags_absent_all has)
  {
    return ((flags & has.value) == 0);
  }

  template <typename String, typename Integer>
  struct json_path
  {
    using char_type = String::value_type;
    using conv_type = ksi::conv::string_cast::to<String>;
    using char_info = ksi::chars::info<char_type>;
    using flags_type = size_t;

    enum section_type { n_index, n_key };

    struct section
    {
      index_t depth;
      section_type type;
      Integer index;
      String key{};

      constexpr bool is_index() const
      {
        return (type == n_index);
      }

      constexpr bool is_key() const
      {
        return (type == n_key);
      }
    };

    // props
    std::vector<section> steps;

    index_t get_depth() const
    {
      return std::ssize(steps);
    }

    void append(Integer index)
    {
      steps.emplace_back(get_depth() + 1, n_index, index);
    }

    void append(String key)
    {
      steps.emplace_back(get_depth() + 1, n_key, -1, key);
    }

    void shift_up()
    {
      steps.pop_back();
    }

    /*
      (root)     -
      (sample)   1:some_key_1 2# 3:some_key_3
      (match?)   2:1 3~^some_key.\d+$
    */

    enum flags_t : flags_type
    {
      f_none = 0,
      f_index_empty = (1 << 0),
    };

    String full_path_string(flags_type flags) const
    {
      conv_type conv{};
      if( steps.empty() ) { return conv("-"); }

      std::basic_stringstream<char_type> stream;

      index_t depth{ 0 };
      for( section const & it : steps )
      {
        ++depth;
        if( depth > 1 ) { stream << char_info::space; }
        stream << conv(depth);
        if( it.is_key() )
        {
          stream << char_info::colon;
          stream << it.key;
        }
        else
        {
          stream << char_info::sharp;
          if( flags * flags_absent_all{ f_index_empty } )
          {
            stream << conv(it.index);
          }
        }
      }
      return std::move(stream).str();
    }
  };


} // end ns
