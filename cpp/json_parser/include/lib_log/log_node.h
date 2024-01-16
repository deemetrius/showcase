#pragma once

namespace lib_log {


  template <typename String, typename Code>
  struct node_info
  {
    using string_type = String;
    using code_type = Code;

    code_type code;
    string_type message;
  };


  template <typename Info, typename Kind, typename Data>
  struct node
    : public Info
  {
    using info_type = Info;

    using kind_type = Kind;
    using data_type = Data;

    // props
    kind_type kind;
    data_type data;
  };


} // end ns
