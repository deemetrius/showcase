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
    , colon = ':'
    , comma = ','
    , dot   = '.'
    , slash = '\\'

    , zero  = '0'
    , nine  = '9'

    , letter_b = 'b'
    , letter_f = 'f'
    , letter_n = 'n'
    , letter_r = 'r'
    , letter_t = 't'

    , ff    = '\f'
    , cr    = '\r'
    , lf    = '\n'
    , space = ' '
    , tab   = '\t'
    , bs    = '\b'

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
    , colon = L':'
    , comma = L','
    , dot   = L'.'
    , slash = L'\\'

    , zero  = L'0'
    , nine  = L'9'

    , letter_b = L'b'
    , letter_f = L'f'
    , letter_n = L'n'
    , letter_r = L'r'
    , letter_t = L't'

    , ff    = L'\f'
    , cr    = L'\r'
    , lf    = L'\n'
    , space = L' '
    , tab   = L'\t'
    , bs    = L'\b'

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
