#pragma once

namespace parser {

  using index_t = std::ptrdiff_t;

  namespace detail {

    struct parser_base
    {

    };

    struct parser_state
    {
      using state = std::unique_ptr<parser_base>;
      using chain = std::list<state>;

      chain nodes;
    };

  } // end ns

  struct json
  {
    template <typename Maker, typename String>
    static typename Maker::result_type
      from_string(Maker const & maker, String source, index_t tab_size = 4)
    {
      using reader_type = ksi::lib::string_reader<String>;
      using char_type = decltype( std::declval<reader_type>().read_char() );

      reader_type reader{ source };
      ksi::files::position position{ tab_size };

      while( reader.is_end() == false )
      {
        char_type ch = reader.read_char();
        position.recognized(ch);
      }
      typename Maker::result_type ret{};
      return ret;
    }
  };

} // end ns
