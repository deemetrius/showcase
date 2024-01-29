#pragma once

#include "../lib_log/log_base.h"
#include "../lib_log/log_node.h"

#include "../lib_aux/string_join.h"

namespace parser {


  enum class json_message_type
  {
    n_notice,
    n_warning,
    n_error,
  };

  struct json_message_codes
  {
    enum status : index_t
    {
      n_ok = 0,

      n_top_unexpected_symbol,

      n_comments_wrong_char,

      n_keyword_unknown,

      n_number_unexpected_symbol,
      n_number_loss_of_digits,
      n_number_loss_of_fractional_digits,
      n_number_huge,
      n_number_not_complete,
      n_number_extent_should_be_zero,
      n_number_extent_part_overflow,
      n_number_extent_part_was_unfit,
      n_number_extent_part_should_be_negative,
      n_number_extent_part_should_be_positive,

      n_string_unclosed,
      n_string_unk_esc_seq,

      n_array_unclosed,
      n_array_unexpected_symbol,
      n_array_internal_error,

      n_map_unclosed,
      n_map_unexpected_symbol,
      n_map_internal_error,
    };
  };


  template <typename Log_string>
  using json_log_node = lib_log::node<
    lib_log::node_info<Log_string, index_t>,
    json_message_type,
    ksi::files::position
  >;

  template <typename Log_string>
  using json_log_pointer = lib_log::base<
    json_log_node<Log_string>
  > *;


} // end ns
namespace parser::detail {


  using namespace std::string_literals;


  template <typename Log_string, typename Result>
  struct json_messages
  {
    using log_node_info = lib_log::node_info<Log_string, index_t>;
    using log_conv_type = ksi::conv::string_cast::to<Log_string>;
    using log_pointer = json_log_pointer<Log_string>;
    using response_pointer = parser_response<Result> *;
    using pos_type = ksi::files::position;

    // props
    log_pointer log{ nullptr };
    response_pointer resp{ nullptr };
    pos_type pos;

    void inform_and_status(index_t code, json_message_type type, Log_string message)
    {
      log->inform({ { code, message }, type, pos });
      resp->change_status(code);
    }

    void inform_only(index_t code, json_message_type type, Log_string message)
    {
      log->inform({ { code, message }, type, pos });
    }

    //

    template < typename Expected = std::initializer_list<std::string> >
    void comments_wrong_char(Expected const & expected)
    {
      inform_and_status(
        json_message_codes::n_comments_wrong_char,
        json_message_type::n_error,
        lib_string::join<Log_string>(
          expected, ", "s, "Wrong symbol found inside comments; Expected: "s
        )
      );
    }

    // number

    void number_too_much_digits(index_t n, int limit)
    {
      inform_only(
        json_message_codes::n_number_loss_of_digits,
        json_message_type::n_warning,
        lib_string::concat< Log_string, std::initializer_list<std::string> >({
          "Precision of number may be lost; Count of digits exceeds its limit: "s,
          std::to_string(limit),
          " (got "s,
          std::to_string(n),
          ")"s
        })
      );
    }

    void number_fractional_part_too_long(index_t limit)
    {
      inform_only(
        json_message_codes::n_number_loss_of_fractional_digits,
        json_message_type::n_notice,
        log_conv_type{}(
          "Precision of number is lost; Fractional part is too long; Limit: "s +
          std::to_string(limit)
        )
      );
    }

    void number_huge(index_t n)
    {
      inform_only(
        json_message_codes::n_number_huge,
        json_message_type::n_warning,
        log_conv_type{}(
          "Number is too long; Huge count of digits occured: "s + std::to_string(n)
        )
      );
    }

    void number_e_notation_not_zero()
    {
      inform_only(
        json_message_codes::n_number_extent_should_be_zero,
        json_message_type::n_warning,
        lib_string::concat< Log_string, std::initializer_list<std::string> >({
          "Extent part for current number should be zero;"
          " So zero is used as extent; Further extent digits will be ignored."s
        })
      );
    }

    template <typename Number>
    void number_e_notation_range(Number number, Number digit, Number low, Number high, Number used)
    {
      inform_only(
        json_message_codes::n_number_extent_part_overflow,
        json_message_type::n_warning,
        lib_string::concat< Log_string, std::initializer_list<std::string> >({
          "Extent part overflow: "s,
          ((number != 0) ? std::to_string(number) : ""s),
          std::to_string(digit),
          " ; Allowed range for current number is [ "s,
          std::to_string(low),
          " .. "s,
          std::to_string(high),
          " ] ; Extent was corrected to: "s,
          std::to_string(used),
          " ; Further extent digits will be ignored."
          })
      );
    }

    template <typename Number>
    void number_e_was_unfit(Number given, Number low, Number high, Number used)
    {
      inform_only(
        json_message_codes::n_number_extent_part_was_unfit,
        json_message_type::n_warning,
        lib_string::concat< Log_string, std::initializer_list<std::string> >({
          std::to_string(given),
          " is inappropriate extent part for current number; Allowed range is [ "s,
          std::to_string(low),
          " .. "s,
          std::to_string(high),
          " ] ; Extent was corrected to: "s,
          std::to_string(used)
        })
      );
    }

    void number_e_notation_not_negative()
    {
      inform_only(
        json_message_codes::n_number_extent_part_should_be_negative,
        json_message_type::n_warning,
        log_conv_type{}(
          "The extent part for given number hould be negative; The following extent part will be ignored."
        )
      );
    }

    void number_e_notation_not_positive()
    {
      inform_only(
        json_message_codes::n_number_extent_part_should_be_positive,
        json_message_type::n_warning,
        log_conv_type{}(
          "The extent part for given number hould be positive; The following extent part will be ignored."
        )
      );
    }

    template <typename Expected>
    void number_unexpected_symbol(Expected const & expected)
    {
      inform_and_status(
        json_message_codes::n_number_unexpected_symbol,
        json_message_type::n_warning,
        lib_string::join<Log_string>(
          expected, ", "s, "Wrong symbol found while number recognition; Expected: "s
        )
      );
    }

    template <typename Expected>
    void number_not_complete(Expected const & expected)
    {
      inform_and_status(
        json_message_codes::n_number_not_complete,
        json_message_type::n_warning,
        lib_string::join<Log_string>(
          expected, ", ", "Unexpected end of json; Number is not complete; Expected: "
        )
      );
    }

    // string

    static log_node_info text_escape_sequence_notice()
    {
      return {
        json_message_codes::n_string_unk_esc_seq,
        log_conv_type{}(
          "Unrecognized escape sequence in string; Slash char was ignored; Next char was used normally."
        )
      };
    }

    static log_node_info text_unclosed()
    {
      return {
        json_message_codes::n_string_unclosed,
        log_conv_type{}("Unexpected end of json inside string.")
      };
    }

    // array messages

    template <typename Expected>
    static log_node_info array_unexpected(Expected const & expected)
    {
      return {
        json_message_codes::n_array_unexpected_symbol,
        lib_string::join<Log_string>(expected, ", ", "Wrong symbol found inside array; Expected: ")
      };
    }

    static log_node_info array_internal()
    {
      return {
        json_message_codes::n_array_internal_error,
        log_conv_type{}("Internal array error")
      };
    }

    static log_node_info array_unclosed()
    {
      return {
        json_message_codes::n_array_unclosed,
        log_conv_type{}("Unexpected end of json inside array.")
      };
    }

    // map messages

    template <typename Expected>
    static log_node_info map_unexpected(Expected const & expected)
    {
      return {
        json_message_codes::n_map_unexpected_symbol,
        lib_string::join<Log_string>(expected, ", ", "Wrong symbol found inside map; Expected: ")
      };
    }

    static log_node_info map_internal()
    {
      return {
        json_message_codes::n_map_internal_error,
        log_conv_type{}("Internal map error")
      };
    }

    static log_node_info map_unclosed()
    {
      return {
        json_message_codes::n_map_unclosed,
        log_conv_type{}("Unexpected end of json inside map.")
      };
    }
  }; // end struct


} // end ns
