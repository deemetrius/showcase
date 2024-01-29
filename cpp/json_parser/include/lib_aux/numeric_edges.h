#pragma once

namespace lib_number {


  /*
    вспомогательные классы дл€ проверки
    можно ли без переполнени€ цифру добавить в целое число
  */

  template <typename Integer, Integer Radix, bool Is_positive>
  struct numeric_edge
  {
    static constexpr Integer calc_last_digit(Integer number)
    {
      if constexpr( Is_positive )
      {
        return (number % Radix);
      }
      else
      {
        return -(number % Radix);
      }
    }

    // props
    Integer prelimit;
    Integer last_digit; // not negative

    // ctor
    constexpr numeric_edge(Integer limit)
      : prelimit{ limit / Radix}
      , last_digit{ calc_last_digit(limit) }
    {}

    constexpr bool can_be_added(Integer number, Integer digit) const
    {
      if constexpr( Is_positive )
      {
        return (
          (number < prelimit) || (
            (number == prelimit) && (digit <= last_digit)
          )
        );
      }
      else
      {
        return (
          (number > prelimit) || (
            (number == prelimit) && (digit <= last_digit)
          )
        );
      }
    }
  };

  template <typename Integer_target, typename Integer_storage, Integer_storage Radix>
  struct numeric_edges
  {
    static_assert(
      sizeof(Integer_target) <= sizeof(Integer_storage)
    );

    using limits = std::numeric_limits<Integer_target>;

    static constexpr numeric_edge<Integer_storage, Radix, false>
      edge_minus{ limits::min() };
    static constexpr numeric_edge<Integer_storage, Radix, true>
      edge_plus{ limits::max() };

    static constexpr bool clarify(Integer_storage number, Integer_storage digit)
    {
      return (
        (number < 0) ?
        edge_minus.can_be_added(number, digit) :
        edge_plus.can_be_added(number, digit)
      );
    }
  };


  template <typename Integer, Integer Radix, bool Is_low>
  struct numeric_edge_custom
  {
    static constexpr Integer calc_last_digit(Integer number)
    {
      if( number < 0 )
      {
        return -(number % Radix);
      }
      else
      {
        return (number % Radix);
      }
    }

    // props
    Integer prelimit;
    Integer last_digit; // not negative

    // ctor
    constexpr numeric_edge_custom(Integer limit)
      : prelimit{ limit / Radix }
      , last_digit{ calc_last_digit(limit) }
    {}

    constexpr bool can_be_added(Integer number, Integer digit) const
    {
      if constexpr( Is_low )
      {
        return (
          (number > prelimit) || (
            (number == prelimit) && (digit <= last_digit)
          )
        );
      }
      else
      {
        return (
          (number < prelimit) || (
            (number == prelimit) && (digit <= last_digit)
          )
        );
      }
    }
  };

  template <typename Integer_storage, Integer_storage Radix>
  struct numeric_edges_custom
  {
    numeric_edge_custom<Integer_storage, Radix, true>
      edge_low;
    numeric_edge_custom<Integer_storage, Radix, false>
      edge_high;

    Integer_storage
      number_low,
      number_high;

    constexpr numeric_edges_custom(Integer_storage low, Integer_storage high)
      : edge_low{ low }
      , edge_high{ high }
      , number_low{ low }
      , number_high{ high }
    {}

    constexpr bool clarify(
      Integer_storage number,
      Integer_storage digit,
      Integer_storage sign,
      Integer_storage & correct_value
    ) const
    {
      bool ret{ false };
      if( sign < 0 )
      {
        ret = edge_low.can_be_added(number, digit);
        if( ret == false ) { correct_value = number_low; }
      }
      else
      {
        ret = edge_high.can_be_added(number, digit);
        if( ret == false ) { correct_value = number_high; }
      }
      if( ret )
      {
        correct_value = (number * Radix + sign * digit);
      }
      return ret;
    }

    constexpr bool is_unfit_calc_correct(Integer_storage number, Integer_storage & correct_value) const
    {
      if( number < number_low )
      {
        correct_value = number_low;
        return true;
      }
      if( number > number_high )
      {
        correct_value = number_high;
        return true;
      }
      correct_value = number;
      return (number < number_low) || (number > number_high);
    }
  };


} // end ns
