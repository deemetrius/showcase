#pragma once

namespace ksi::lib {

  template <typename String>
  struct string_reader
  {
    using text = const String;
    using iterator = String::const_iterator;
    using char_type = String::value_type;
    using char_info = ksi::chars::info<char_type>;

    text string;
    iterator it{string.cbegin()};

    bool is_end() const
    {
      return (string.cend() == it);
    }

    char_type read_char()
    {
      if( is_end() ) { return char_info::null; }
      char_type ret{*it};
      ++it;
      return ret;
    }
  };

} // end ns
