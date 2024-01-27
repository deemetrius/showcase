#pragma once

namespace ksi::chars {


  struct info_char
  {
    static constexpr char
      minus = '-'
      , plus = '+'
      , divide = '/'
      , colon = ':'
      , comma = ','
      , dot = '.'
      , slash = '\\'
      , sharp = '#'

      , digit_0 = '0'
      , digit_9 = '9'

      , letter_a = 'a'
      , letter_b = 'b'
      , letter_f = 'f'
      , letter_n = 'n'
      , letter_r = 'r'
      , letter_t = 't'
      , letter_z = 'z'

      , ff = '\f'
      , cr = '\r'
      , lf = '\n'
      , space = ' '
      , tab = '\t'
      , bs = '\b'

      , null = '\0'

      , quote_single = '\''
      , quote_double = '"'

      , bracket_open = '['
      , bracket_close = ']'

      , brace_open = '{'
      , brace_close = '}'
      ;
  };


  struct info_wchar
  {
    static constexpr wchar_t
      minus = L'-'
      , plus = L'+'
      , divide = L'/'
      , colon = L':'
      , comma = L','
      , dot = L'.'
      , slash = L'\\'
      , sharp = L'#'

      , digit_0 = L'0'
      , digit_9 = L'9'

      , letter_a = L'a'
      , letter_b = L'b'
      , letter_f = L'f'
      , letter_n = L'n'
      , letter_r = L'r'
      , letter_t = L't'
      , letter_z = L'z'

      , ff = L'\f'
      , cr = L'\r'
      , lf = L'\n'
      , space = L' '
      , tab = L'\t'
      , bs = L'\b'

      , null = L'\0'

      , quote_single = L'\''
      , quote_double = L'"'

      , bracket_open = L'['
      , bracket_close = L']'

      , brace_open = L'{'
      , brace_close = L'}'
      ;
  };


  template <typename Char>
  struct info;

  template <> struct info<char> : public info_char
  {};
  template <> struct info<wchar_t> : public info_wchar
  {};


  template <typename Char>
  bool is_letter_english_small(Char ch)
  {
    using info_t = info<Char>;
    return (
      (ch >= info_t::letter_a) &&
      (ch <= info_t::letter_z)
    );
  }


  template <typename Char>
  bool is_digit(Char ch)
  {
    using info_t = info<Char>;
    return (
      (ch >= info_t::digit_0) &&
      (ch <= info_t::digit_9)
    );
  }


  template <typename Integer, typename Char>
  Integer digit_of(Char ch)
  {
    using info_t = info<Char>;
    return (ch - info_t::digit_0);
  }


} // end ns
