#pragma once

#include "json.nest.h"

namespace parser::detail {


  template <typename Char, typename Maker, typename Log_string>
  class json_nest<Char, Maker, Log_string>::node_space
  {
  protected:
    node_space() = delete;

  public:
    static bool condition(json_params const * params, Char ch)
    {
      return is_eq(ch, info::space, info::tab, info::cr, info::lf);
    }
  };


} // end ns
