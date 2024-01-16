#pragma once

#include "log_base.h"
#include "log_node.h"
#include <list>

namespace lib_log {


  template <typename String, typename Kind, typename Data>
  struct list
    : public base<String, Kind, Data>
  {
    using base_type = base<String, Kind, Data>;
    using node_type = node<String, Kind, Data>;

    using starage_type = std::list<node_type>;
    using typename base_type::index_type;

    using string_type = String;
    using kind_type = Kind;
    using data_type = Data;

    // data
    starage_type starage;

    // actions

    void
      inform(string_type message, kind_type kind, data_type data) override
    {
      starage.push_back({ message, kind, data });
    }

    index_type
      count() const
    {
      return std::ssize(starage);
    }
  };


} // end ns
