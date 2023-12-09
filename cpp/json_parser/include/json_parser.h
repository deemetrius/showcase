#pragma once

namespace pjson {

template <typename Maker, typename String>
typename Maker::result_type parse(Maker fn, String from)
{
  using char_type = decltype(from[0]);
  for( char_type ch : from )
  {}
}

} // end ns
