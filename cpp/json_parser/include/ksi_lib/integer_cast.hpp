#pragma once

#include <concepts>
#include <limits>

namespace ksi::conv::numbers {


  struct min_keeper
  {
    template <std::integral Type>
    consteval operator Type () const
    {
      return std::numeric_limits<Type>::min();
    }

    template <std::integral Type>
    static consteval Type get()
    {
      return std::numeric_limits<Type>::min();
    }
  };
  constexpr inline min_keeper min_value{};


  struct max_keeper
  {
    template <std::integral Type>
    consteval operator Type () const
    {
      return std::numeric_limits<Type>::max();
    }

    template <std::integral Type>
    static consteval Type get()
    {
      return std::numeric_limits<Type>::max();
    }
  };
  constexpr inline max_keeper max_value{};


  template <typename Type>
  consteval bool is_negative_allowed()
  {
    return (min_keeper::get<Type>() < 0);
  }


  template <typename Desired, typename Param>
    requires (is_negative_allowed<Desired>() == is_negative_allowed<Param>())
  consteval bool low_border_is_higher()
  {
    return (min_keeper::get<Desired>() > min_keeper::get<Param>());
  }

  template <typename Desired, typename Param>
    requires (is_negative_allowed<Desired>() != is_negative_allowed<Param>())
  consteval bool low_border_is_higher()
  {
    return (is_negative_allowed<Desired>() < is_negative_allowed<Param>());
  }


  enum class adapt_status
  {
    was_preserved,
    was_lowered,
    was_raised,
  };

  template <typename Type>
  struct adapt_result
  {
    Type value;
    adapt_status status;
  };


  template <typename Desired, typename Param>
  constexpr adapt_result<Param> adapt(Param number)
  {
    if constexpr( low_border_is_higher<Desired, Param>() )
    {
      Param min = static_cast<Param>(min_keeper::get<Desired>());
      if( number < min )
      {
        return { min, adapt_status::was_raised };
      }
    }

    if constexpr( max_keeper::get<Desired>() < max_keeper::get<Param>() )
    {
      Param max = static_cast<Param>(max_keeper::get<Desired>());
      if( number > max )
      {
        return { max, adapt_status::was_lowered };
      }
    }

    return { number, adapt_status::was_preserved };
  }


  template <typename Result, typename Param>
  constexpr adapt_result<Result> integer_cast(Param number)
  {
    adapt_result<Param> result = adapt<Result>(number);
    return { static_cast<Result>(result.value), result.status };
  }


} // end ns
