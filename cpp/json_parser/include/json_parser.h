#pragma once

namespace parser {

  using index_t = std::ptrdiff_t;

  template <typename String>
  struct string_reader
  {
    using text = const String;
    using iterator = String::const_iterator;
    using char_type = String::value_type;
    using char_info = ksi::chars::info<char_type>;

    text string;
    iterator it{ string.cbegin() };

    bool is_end() const
    {
      return (string.cend() == it);
    }

    char_type read_char()
    {
      if( is_end() ) { return char_info::null; }
      char_type ret{ *it };
      ++it;
      return ret;
    }
  };

  struct json
  {
    template <typename Maker, typename String>
    static typename Maker::result_type from_string(Maker const & maker, String source, index_t tab_size = 4)
    {
      string_reader<String> reader{ source };
      using char_type = decltype( reader.read_char() );
      ksi::files::position position{ tab_size };
      while( reader.is_end() == false )
      {
        char_type ch = reader.read_char();
      }
      typename Maker::result_type ret;
      return ret;
    }
  };

} // end ns
