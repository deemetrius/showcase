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
    , cr    = L'\r'
    , lf    = L'\n'
    , tab   = L'\t'
    , null  = L'\0'
    ;
  };

} // end ns
