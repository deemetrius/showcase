#pragma once

#include <cstddef>

namespace lib_log {


  template <typename Node>
  struct base
  {
    using node_type = Node;

    using index_type = std::ptrdiff_t;
    enum index_status : index_type { not_implemented = -1 };

    // actions

    virtual void
      inform(node_type notification) = 0;

    virtual index_type
      count() const
    {
      return not_implemented;
    }
  };


} // end ns
