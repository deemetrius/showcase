#pragma once

#include "json.nest.h"

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

    static ptr_node create(
      Maker * maker,
      json_params const * params,
      pos_type start_pos,
      state_data const & data
    )
    {
      return std::make_unique<node_map>(start_pos, maker, data.path);
    }

    static constexpr choicer_type choicer{ &get_name, &condition, &create };


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

    node_map(pos_type pos, Maker * maker, path_type const & path)
      : node_base{ pos }
      , map{ maker->make_map(path, pos) }
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
        expected.push_back( log_conv_type{}("t_open_brace") );
      }

      if( (req & kind_close) != 0 )
      {
        if( ch == info::brace_close )
        {
          st.after_fn = &chain_actions::chain_up_result;
          req = was_close;
          return;
        }
        expected.push_back( log_conv_type{}("t_close_brace") );
      }

      if( (req & kind_delimiter) != 0 )
      {
        if( ch == info::colon )
        {
          req = was_delimiter;
          return;
        }
        expected.push_back( log_conv_type{}("t_colon") );
      }

      if( (req & kind_separator) != 0 )
      {
        if( ch == info::comma )
        {
          req = was_separator;
          return;
        }
        expected.push_back( log_conv_type{}("t_comma") );
      }

      if( (req & kind_key) != 0 )
      {
        choicer_type const * it = find_from_key(st.params, ch);
        if( it != nullptr )
        {
          st.add_node(
            it->create(st.maker, st.params, st.position.get(), st.data)
          );
          st.skip_read();
          return;
        }
        expected.push_back( log_conv_type{}("t_value_string") );
      }

      if( (req & kind_value) != 0 )
      {
        choicer_type const * it = find_from_all(st.params, ch);
        if( it != nullptr )
        {
          st.add_node(
            it->create(st.maker, st.params, st.position.get(), st.data)
          );
          st.skip_read();
          return;
        }
        expected.push_back( log_conv_type{}("t_value") );
      }

      // not match
      st.data.log->inform({
        log_messages::map_unexpected(expected),
        json_message_type::n_error,
        st.position.get()
      });
      st.after_fn = &chain_actions::chain_unwind;
      resp.change_status(json_message_codes::n_map_unexpected_symbol);
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
        st.maker->map_insert(map, key.value(), result, st.data.path);
        st.data.path.shift_up();
        key.reset();
        req = was_value;
        return;
      }

      st.data.log->inform({
        log_messages::map_internal(),
        json_message_type::n_error,
        st.position.get()
      });
      resp.change_status(json_message_codes::n_map_internal_error);
    }

    void input_ended(parser_state & st, response_type & resp) override
    {
      st.data.log->inform({
        log_messages::map_unclosed(),
        json_message_type::n_error,
        st.position.get()
      });
      resp.change_status(json_message_codes::n_map_unclosed);
    }
  };


} // end ns
