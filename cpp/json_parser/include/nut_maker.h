#pragma once

namespace nut {

  using integer = std::intptr_t;

  template <typename String>
  struct nut_maker
  {
    using result_type = ssq::Object;

    using integer = SQInteger;
    using text = ssq::sqstring;
    using array = ssq::Array;
    using map = ssq::Table;
  };

}
