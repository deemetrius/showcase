#pragma once

#include "parser_json.nest.h"

namespace parser::detail {


  template <typename Char, typename Maker, typename Log_string>
  class json_nest<Char, Maker, Log_string>::node_array
    : public node_base
  {
  public:
    static std::string get_name() { return "t_array"; }

    static bool condition(json_params const * params, Char ch)
    {
      return (ch == info::bracket_open);
    }

    static ptr_node create(Maker * maker, json_params const * params, pos_type start_pos)
    {
      return std::make_unique<node_array>(start_pos, maker);
    }

    static constexpr choicer_type choicer{&get_name, &condition, &create};


    using array_type = Maker::array;
    using integer_type = Maker::integer;

    enum kind : std::size_t
    {
      kind_open      = (1 << 0),
      kind_close     = (1 << 1),
      kind_value     = (1 << 2),
      kind_separator = (1 << 3),

      was_open      = (kind_close | kind_value),
      was_value     = (kind_close | kind_separator),
      was_separator = (kind_value),
      was_close     = 0,
    };

    // data
    array_type array;
    integer_type index{0};
    kind req{kind_open};

    // ctor
    node_array(pos_type pos, Maker * maker)
      : node_base{pos}
      , array{maker->make_array(pos)}
    {}

    void parse(parser_state & st, response_type & resp, Char ch) override
    {
      if( node_space::condition(st.params, ch) )
      {
        return;
      }

      std::vector<Log_string> expected;

      if( (req & kind_open) != 0 )
      {
        if( ch == info::bracket_open )
        {
          req = was_open;
          return;
        }
        expected.push_back( log_conv_type{}("T_OPEN_BRACKET") );
      }

      if( (req & kind_close) != 0 )
      {
        if( ch == info::bracket_close )
        {
          st.after_fn = &parser_state::action_up_result;
          req = was_close;
          return;
        }
        expected.push_back( log_conv_type{}("T_CLOSE_BRACKET") );
      }

      if( (req & kind_separator) != 0 )
      {
        if( ch == info::comma )
        {
          req = was_separator;
          return;
        }
        expected.push_back( log_conv_type{}("T_COMMA") );
      }

      if( (req & kind_value) != 0 )
      {
        choicer_type const * it = find_from_all(st.params, ch);
        if( it != nullptr )
        {
          st.add_node(
            it->create( st.maker, st.params, st.position.get() )
          );
          st.skip_read();
          return;
        }
        expected.push_back( log_conv_type{}("T_VALUE") );
      }

      // not match
      st.data.log->inform(
        lib_string::join<Log_string>(expected, ", ", "Wrong symbol found; Expected: "),
        json_status::n_array_unexpected_symbol,
        st.position.get()
      );
      st.after_fn = &parser_state::action_unwind;
      resp.change_status(json_status::n_array_unexpected_symbol);
    }

    result_type get_result(parser_state & st, response_type & resp) override
    {
      return array;
    }

    void put_result(result_type result, parser_state & st, response_type & resp) override
    {
      if( (req & kind_value) != 0 )
      {
        st.maker->array_insert(array, index, result);
        req = was_value;
        ++index;
        return;
      }

      st.data.log->inform(
        log_conv_type{}("Internal array error"),
        json_status::n_array_internal_error,
        st.position.get()
      );
      resp.change_status(json_status::n_array_internal_error);
    }

    void input_ended(parser_state & st, response_type & resp) override
    {
      st.data.log->inform(
        log_conv_type{}("Unexpected end of json inside array."),
        json_status::n_array_unclosed,
        st.position.get()
      );
      resp.change_status(json_status::n_array_unclosed);
    }
  }; // end class


} // end ns
