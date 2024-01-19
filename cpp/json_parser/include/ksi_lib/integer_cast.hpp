#pragma once

#include <concepts>
#include <type_traits>
#include <limits>
#include <algorithm>

namespace ksi::numbers {


  // left == right
  template<std::integral Left, std::integral Right>
  constexpr bool cmp_equal(Left left, Right right) noexcept
  {
    if constexpr( std::is_signed_v<Left> == std::is_signed_v<Right> )
    { 
      return (left == right);
    }
    else if constexpr( std::is_signed_v<Left> )
    {
      return (left >= 0) && (std::make_unsigned_t<Left>{left} == right);
    }
    else
    {
      return (right >= 0) && (std::make_unsigned_t<Right>{right} == left);
    }
  }


  // left != right
  template<class Left, class Right>
  constexpr bool cmp_not_equal(Left left, Right right) noexcept
  {
    return not cmp_equal(left, right);
  }


  // left < right
  template<class Left, class Right>
  constexpr bool cmp_less(Left left, Right right) noexcept
  {
    if constexpr( std::is_signed_v<Left> == std::is_signed_v<Right> )
    {
      return (left < right);
    }
    else if constexpr( std::is_signed_v<Left> )
    {
      return (left < 0) || (std::make_unsigned_t<Left>(left) < right);
    }
    else
    {
      return (right >= 0) && ( left < std::make_unsigned_t<Right>(right) );
    }
  }


  // left > right
  template<class Left, class Right>
  constexpr bool cmp_greater(Left left, Right right) noexcept
  {
    return cmp_less(right, left);
  }


  // left <= right
  template<class Left, class Right>
  constexpr bool cmp_less_equal(Left left, Right right) noexcept
  {
    return not cmp_less(right, left);
  }


  // left >= right
  template<class Left, class Right>
  constexpr bool cmp_greater_equal(Left left, Right right) noexcept
  {
    return not cmp_less(left, right);
  }


  template<std::integral Result, std::integral Param>
  constexpr bool in_range(Param number) noexcept
  {
    return (
      cmp_greater_equal( number, std::numeric_limits<Result>::min() ) &&
      cmp_less_equal( number, std::numeric_limits<Result>::max() )
    );
  }


} // end ns
namespace ksi::conv {


  template <std::integral Result, std::integral Param>
  constexpr Result integer_cast(Param number) noexcept
  {
    using lim_param = std::numeric_limits<Param>;
    using lim_result = std::numeric_limits<Result>;

    if constexpr( ksi::numbers::in_range<Result>( lim_param::min() ) )
    {
      if constexpr( ksi::numbers::in_range<Result>( lim_param::max() ) )
      {
        return static_cast<Result>(number);
      }
      else
      {
        return static_cast<Result>(
          std::min<Param>(lim_result::max(), number)
        );
      }
    }
    else
    {
      return static_cast<Result>(
        std::clamp<Param>( number, lim_result::min(), lim_result::max() )
      );
    }
  }


} // end ns
