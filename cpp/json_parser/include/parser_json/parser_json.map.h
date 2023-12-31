#pragma once


namespace parser::detail {


  template <typename Char, typename Maker>
  class json_nest<Char, Maker>::node_map
    : public node_base
  {
  public:
    static std::string get_name() { return "t_number"; }

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

      if( (req & kind_open) != 0 )
      {
        if( ch == info::brace_open )
        {
          req = was_open;
          return;
        }
      }

      if( (req & kind_close) != 0 )
      {
        if( ch == info::brace_close )
        {
          st.after_fn = &parser_state::action_up_result;
          req = was_close;
          return;
        }
      }

      if( (req & kind_delimiter) != 0 )
      {
        if( ch == info::colon )
        {
          req = was_delimiter;
          return;
        }
      }

      if( (req & kind_separator) != 0 )
      {
        if( ch == info::comma )
        {
          req = was_separator;
          return;
        }
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
      }

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

      resp.change_status(json_status::n_map_internal_error);
    }

    void input_ended(parser_state & st, response_type & resp) override
    {
      resp.change_status(json_status::n_map_unclosed);
    }
  };


} // end ns
