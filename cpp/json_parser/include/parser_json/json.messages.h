#pragma once

#include "../lib_log/log_base.h"
#include "../lib_log/log_node.h"

#include "../ksi_lib/conv.string.hpp"
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

      n_keyword_unknown,

      n_number_loss_of_digits,
      n_number_huge,

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


} // end ns
namespace parser::detail {


  template <typename Log_string>
  struct json_messages
  {
    using log_node_info = lib_log::node_info<Log_string, index_t>;
    using log_conv_type = ksi::conv::from_string::to<Log_string>;

    // number

    static log_node_info number_too_much_digits(index_t n, int limit)
    {
      using namespace std::string_literals;

      return {
        json_message_codes::n_number_loss_of_digits,
        log_conv_type{}(
          "Number precision may be lost; Count of digits exceeds its limit: "s +
          std::to_string(limit) + " (got "s + std::to_string(n) + ")"s
        )
      };
    }

    static log_node_info number_huge(index_t n)
    {
      using namespace std::string_literals;

      return {
        json_message_codes::n_number_huge,
        log_conv_type{}(
          "Number is too long; Huge count of digits occured: "s + std::to_string(n)
        )
      };
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
namespace parser {


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
