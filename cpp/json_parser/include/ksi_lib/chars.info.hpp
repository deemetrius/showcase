#pragma once

namespace ksi::chars {

  template <typename Char>
  struct info;

  template <>
  struct info<char>
  {
    static constexpr char
      minus = '-'
    , plus  = '+'

    , zero  = '0'
    , nine  = '9'

    , dot   = '.'

    , cr    = '\r'
    , lf    = '\n'
    , space = ' '
    , tab   = '\t'

    , null  = '\0'

    , quote_single  = '\''
    , quote_double  = '"'

    , bracket_open  = '['
    , bracket_close = ']'

    , brace_open  = '{'
    , brace_close = '}'
    ;
  };

  template <>
  struct info<wchar_t>
  {
    static constexpr wchar_t
      minus = L'-'
    , plus  = L'+'

    , zero  = L'0'
    , nine  = L'9'

    , dot   = L'.'

    , cr    = L'\r'
    , lf    = L'\n'
    , space = L' '
    , tab   = L'\t'

    , null  = L'\0'

    , quote_single  = L'\''
    , quote_double  = L'"'

    , bracket_open  = L'['
    , bracket_close = L']'

    , brace_open  = L'{'
    , brace_close = L'}'
    ;
  };

  template <typename Char>
  bool is_digit(Char ch)
  {
    using info_t = info<Char>;
    return (
      (ch >= info_t::zero) &&
      (ch <= info_t::nine)
    );
  }

  template <typename Integer, typename Char>
  Integer digit_of(Char ch)
  {
    using info_t = info<Char>;
    return (ch - info_t::zero);
  }

} // end ns
