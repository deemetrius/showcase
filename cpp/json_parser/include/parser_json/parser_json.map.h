#pragma once

#include "parser_json.nest.h"

namespace parser::detail {


  template <typename Char, typename Maker, typename Log_string>
  class json_nest<Char, Maker, Log_string>::node_map
    : public node_base
  {
  public:
    static std::string get_name() { return "t_map"; }

    static bool condition(json_params const * params, Char ch)
    {
      return (ch == info::brace_open);
    }

    static ptr_node create(Maker * maker, json_params const * params, pos_type start_pos)
    {
      return std::make_unique<node_map>(start_pos, maker);
    }

    static constexpr choicer_type choicer{&get_name, &condition, &create};


    using map_type = Maker::map;
    using text_type = Maker::text;

    enum kind : std::size_t
    {
      kind_open      = (1 << 0),
      kind_close     = (1 << 1),
      kind_key       = (1 << 2),
      kind_delimiter = (1 << 3),
      kind_value     = (1 << 4),
      kind_separator = (1 << 5),

      was_open      = (kind_close | kind_key),
      was_key       = (kind_delimiter),
      was_delimiter = (kind_value),
      was_value     = (kind_close | kind_separator),
      was_separator = (kind_key),
      was_close     = 0,
    };

    map_type map;
    kind req{ kind_open };
    std::optional<result_type> key{};

    node_map(pos_type pos, Maker * maker)
      : node_base{ pos }
      , map{ maker->make_map(pos) }
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
        if( ch == info::brace_open )
        {
          req = was_open;
          return;
        }
        expected.push_back( log_conv_type{}("T_OPEN_BRACE") );
      }

      if( (req & kind_close) != 0 )
      {
        if( ch == info::brace_close )
        {
          st.after_fn = &parser_state::action_up_result;
          req = was_close;
          return;
        }
        expected.push_back( log_conv_type{}("T_CLOSE_BRACE") );
      }

      if( (req & kind_delimiter) != 0 )
      {
        if( ch == info::colon )
        {
          req = was_delimiter;
          return;
        }
        expected.push_back( log_conv_type{}("T_COLON") );
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

      if( (req & kind_key) != 0 )
      {
        if( node_text::condition(st.params, ch) )
        {
          st.add_node(
            node_text::create( st.maker, st.params, st.position.get() )
          );
          st.skip_read();
          return;
        }
        expected.push_back( log_conv_type{}("T_VALUE_STRING") );
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
      resp.change_status(json_status::n_map_unexpected_symbol);
    }

    result_type get_result(parser_state & st, response_type & resp) override
    {
      return map;
    }

    void put_result(result_type result, parser_state & st, response_type & resp) override
    {
      if( (req & kind_key) != 0 )
      {
        key = result;
        req = was_key;
        return;
      }

      if( (req & kind_value) != 0 )
      {
        if( key.has_value() == false )
        {
          throw json_error_map_key_empty{st.position.get()};
        }
        st.maker->map_insert(map, key.value(), result);
        key.reset();
        req = was_value;
        return;
      }

      st.data.log->inform(
        log_conv_type{}("Internal map error"),
        json_status::n_map_internal_error,
        st.position.get()
      );
      resp.change_status(json_status::n_map_internal_error);
    }

    void input_ended(parser_state & st, response_type & resp) override
    {
      st.data.log->inform(
        log_conv_type{}("Unexpected end of json inside map."),
        json_status::n_map_unclosed,
        st.position.get()
      );
      resp.change_status(json_status::n_map_unclosed);
    }
  };


} // end ns
