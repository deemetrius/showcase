#pragma once

#include "json.nest.h"

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

    static ptr_node create(
      Maker * maker,
      json_params const * params,
      pos_type start_pos,
      state_data const & data
    )
    {
      return std::make_unique<node_array>(start_pos, maker, data.path);
    }

    static constexpr choicer_type choicer{ &get_name, &condition, &create };


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

    // props
    array_type array;
    integer_type index{ 0 };
    kind req{ kind_open };

    // ctor
    node_array(pos_type pos, Maker * maker, path_type const & path)
      : node_base{pos}
      , array{maker->make_array(path, pos)}
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
        expected.push_back( log_conv_type{}("t_open_bracket") );
      }

      if( (req & kind_value) != 0 )
      {
        choicer_type const * it = find_from_all(st.params, ch);
        if( it != nullptr )
        {
          st.data.path.append(index);
          st.add_node(
            it->create(st.maker, st.params, st.position.get(), st.data)
          );
          st.skip_read();
          return;
        }
        expected.push_back(log_conv_type{}("t_value"));
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

      if( (req & kind_close) != 0 )
      {
        if( ch == info::bracket_close )
        {
          st.after_fn = &chain_actions::chain_up_result;
          req = was_close;
          return;
        }
        expected.push_back(log_conv_type{}("t_close_bracket"));
      }

      // not match
      st.data.log->inform({
        log_messages::array_unexpected(expected),
        json_message_type::n_error,
        st.position.get()
      });
      st.after_fn = &chain_actions::chain_unwind;
      resp.change_status(json_message_codes::n_array_unexpected_symbol);
    }

    result_type get_result(parser_state & st, response_type & resp) override
    {
      return array;
    }

    void put_result(result_type result, parser_state & st, response_type & resp) override
    {
      if( (req & kind_value) != 0 )
      {
        st.maker->array_insert(array, index, result, st.data.path);
        st.data.path.shift_up();
        req = was_value;
        ++index;
        return;
      }

      st.data.log->inform({
        log_messages::array_internal(),
        json_message_type::n_error,
        st.position.get()
      });
      resp.change_status(json_message_codes::n_array_internal_error);
    }

    void input_ended(parser_state & st, response_type & resp) override
    {
      st.data.log->inform({
        log_messages::array_unclosed(),
        json_message_type::n_error,
        st.position.get()
      });
      resp.change_status(json_message_codes::n_array_unclosed);
    }
  }; // end class


} // end ns
