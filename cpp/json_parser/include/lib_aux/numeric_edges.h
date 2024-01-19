#pragma once

namespace lib_number {


  /*
    ��������������� ������ ��� ��������
    ����� �� ��� ������������ ����� �������� � ����� �����

    ����� ����� ������������� ������������ ����� � ��������� ������
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

    static constexpr numeric_edge<Integer_storage, Radix, true>
      edge_plus{ limits::max() };
    static constexpr numeric_edge<Integer_storage, Radix, false>
      edge_minus{ limits::min() };

    static constexpr bool clarify(Integer_storage number, Integer_storage digit)
    {
      return (
        (number < 0) ?
        edge_minus.can_be_added(number, digit) :
        edge_plus.can_be_added(number, digit)
      );
    }
  };


} // end ns
