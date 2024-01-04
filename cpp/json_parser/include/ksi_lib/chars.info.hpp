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
    , tab   = '\t'
    , null  = '\0'
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
    , tab   = L'\t'
    , null  = L'\0'
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

  using index_t = std::ptrdiff_t;

  template <typename Char>
  index_t digit_of(Char ch)
  {
    using info_t = info<Char>;
    return (ch - info_t::zero);
  }

} // end ns
