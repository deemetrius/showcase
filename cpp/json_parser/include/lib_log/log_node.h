#pragma once

namespace lib_log {


  template <typename String, typename Kind, typename Data>
  struct node
  {
    using string_type = String;
    using kind_type = Kind;
    using data_type = Data;

    string_type message;
    kind_type kind;
    data_type data;
  };


} // end ns
