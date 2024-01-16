#pragma once

#include <cstddef>

namespace lib_log {


  /*
    // usually Kind is enum
    enum class log_kind { notice, warning, error };

    // however Kind can be struct
    struct log_kind_info
    {
      log_kind        type;
      std::intptr_t   code;
    };
  */

  template <typename String, typename Kind, typename Data>
  struct base
  {
    using string_type = String;
    using kind_type = Kind;
    using data_type = Data;

    using index_type = std::ptrdiff_t;
    enum index_status : index_type { not_implemented = -1 };

    // actions

    virtual void
      inform(string_type message, kind_type kind, data_type data) = 0;

    virtual index_type
      count() const
    {
      return not_implemented;
    }

    virtual index_type
      stat(kind_type kind) const
    {
      return not_implemented;
    }
  };


} // end ns
