#pragma once

#include "log_base.h"
#include <list>

namespace lib_log {


  template <typename Node>
  struct list
    : public base<Node>
  {
    using base_type = base<Node>;

    using node_type = Node;
    using starage_type = std::list<node_type>;

    using typename base_type::index_type;

    // props
    starage_type starage;

    // actions

    void
      inform(node_type notification) override
    {
      starage.push_back(notification);
    }

    index_type
      count() const override
    {
      return std::ssize(starage);
    }
  };


} // end ns
