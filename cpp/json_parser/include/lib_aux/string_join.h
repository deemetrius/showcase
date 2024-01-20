#pragma once

#include <sstream>
#include "../ksi_lib/conv.string.hpp"

namespace lib_string {


  template <typename Result, typename Prefix, typename Strings, typename Delimiter>
  Result join(Strings const & strings, Delimiter delimiter, Prefix prefix = {})
  {
    ksi::conv::from_string::to<Result> conv{};

    if( strings.size() == 0 ) { return conv(prefix); }

    std::basic_stringstream<typename Result::value_type> stream;
    stream << conv(prefix);

    typename Strings::const_iterator it = strings.begin();
    stream << conv(*it);
    ++it;
    while( it != strings.end() )
    {
      stream << conv(delimiter) << conv(*it);
      ++it;
    }

    return std::move(stream).str();
  }


  template <typename Result, typename Strings>
  Result concat(Strings const & strings)
  {
    if( strings.size() == 0 ) { return {}; }

    ksi::conv::from_string::to<Result> conv{};

    std::basic_stringstream<typename Result::value_type> stream;

    for( typename Strings::value_type const & it : strings )
    {
      stream << conv(it);
    }

    return std::move(stream).str();
  }


} // end ns
