#pragma once

#include <type_traits>

namespace lib_bits {


  template <typename Type>
  constexpr inline bool flags_absent(Type flags, std::type_identity_t<Type> absent)
  {
    return ((flags & absent) == 0);
  }

  template <typename Type>
  constexpr inline bool flags_has_all(Type flags, std::type_identity_t<Type> has_all)
  {
    return ((flags & has_all) == has_all);
  }

  template <typename Type>
  constexpr inline bool flags_has_any(Type flags, std::type_identity_t<Type> has_any)
  {
    return ((flags & has_any) != 0);
  }

  /*
  template <typename Uint>
  struct flags_check
  {
    using type = Uint;

    struct absent
    {
      type value{ ~ type{ 0 } };

      friend constexpr inline bool operator * (type flags, absent has)
      {
        return ((flags & has.value) == 0);
      }
    };

    struct has_all
    {
      type value{ 0 };

      friend constexpr inline bool operator * (type flags, has_all has)
      {
        return ((flags & has.value) == has.value);
      }
    };

    struct has_any
    {
      type value{ 0 };

      friend constexpr inline bool operator * (type flags, has_any has)
      {
        return ((flags & has.value) != 0);
      }
    };
  };
  */


} // end ns
