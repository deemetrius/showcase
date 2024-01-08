#pragma once

namespace parser {


  using index_t = std::ptrdiff_t;


  struct is_status
  {
    enum status_base : index_t
    {
      e_unexpected_symbol = -1
    };
  };


  struct exception_skip_result
  {
    ksi::files::position::data_type pos{-1, 0, 0};
  };

  struct exception_result_unexpected
  {
    ksi::files::position::data_type pos{-1, 0, 0};
  };


  template <typename Result>
  struct parser_response
  {
    using position_type = ksi::files::position::data_type;

    std::optional<Result> value;
    index_t status{0};
    position_type end_position{};

    void change_status(index_t new_status)
    {
      if( status != 0 ) { return; }
      status = new_status;
    }
  };


} // end ns

namespace parser::detail {


  template <typename Char, typename Maker, typename Params, typename Data>
  struct nest_base
  {
    using reader_type = std::unique_ptr< ksi::lib::reader<Char> >;
    using result_type = Maker::result_type;
    using response_type = parser_response<result_type>;
    using pos_type = ksi::files::position::data_type;


    struct parser_state;


    struct node_base
    {
      pos_type start_pos{-1, 0, 0};

      node_base(pos_type pos)
        : start_pos{ pos }
      {}

      virtual void parse(parser_state & st, response_type & resp, Char ch) {}

      virtual result_type get_result(parser_state & st, response_type & resp)
      {
        throw exception_result_unexpected{ start_pos };
      }

      virtual void put_result(result_type result, parser_state & st, response_type & resp) {}

      virtual void input_ended(parser_state & st, response_type & resp) {}
    };


    using ptr_node = std::unique_ptr<node_base>;


    struct choicer
    {
      static std::string get_name() { return "no_name?"; }

      static bool condition_false(Params const * params, Char ch)
      { return false; }

      static ptr_node create_none(Maker * maker, Params const * params, pos_type start_pos)
      { return std::make_unique<node_base>(start_pos); }

      using fn_name = decltype( &get_name );
      using fn_condition = decltype( &condition_false );
      using fn_create = decltype( &create_none );


      fn_name name{ &get_name };
      fn_condition condition{ &condition_false };
      fn_create create{ &create_none };
      index_t type{1};


      template <typename Array>
      static choicer const * find(Array const & array, Params const * params, Char ch)
      {
        for( choicer const * it : array )
        {
          if( it->condition(params, ch) ) { return it; }
        }
        return nullptr;
      }
    };


    struct parser_state
    {
      using chain = std::list<ptr_node>;

      // read actions

      static bool read_action(parser_state & st, Char & ch)
      {
        if( st.reader->is_end() ) { return true; }
        ch = st.reader->read_char();
        return false;
      }

      static bool read_action_none(parser_state & st, Char & ch)
      {
        st.read_fn = &read_action;
        return false;
      }

      // post actions

      static void action_none(parser_state & st, response_type & resp)
      {}

      static void action_up_result(parser_state & st, response_type & resp)
      {
        st.after_fn = &parser_state::action_none;
        inner_result_up(st, resp);
      }

      static void action_up_only(parser_state & st, response_type & resp)
      {
        st.after_fn = &parser_state::action_none;
        st.nodes.pop_back();
      }

      static void action_unwind(parser_state & st, response_type & resp)
      {
        st.after_fn = &parser_state::action_none;
        while( st.nodes.empty() == false )
        {
          st.nodes.back()->input_ended(st, resp);
          inner_result_up(st, resp);
        }
      }

      static void action_exit(parser_state & st, response_type & resp)
      {
        st.after_fn = &parser_state::action_none;
        st.nodes.clear();
      }

      // action types

      using read_action_type = decltype( &read_action );
      using action_type = decltype( &action_none );

      static void inner_result_up(parser_state & st, response_type & resp)
      {
        try
        {
          result_type result = st.nodes.back()->get_result(st, resp);
          st.nodes.pop_back();
          if( st.nodes.empty() )
          {
            resp.value = result;
          }
          else
          {
            st.nodes.back()->put_result(result, st, resp);
          }
        }
        catch( exception_skip_result const & )
        {
          st.nodes.pop_back();
        }
      }

      // data
      Params const * params{ nullptr };
      Maker * maker{ nullptr };
      chain nodes;
      reader_type reader;
      ksi::files::position position;
      action_type after_fn{ &action_none };
      read_action_type read_fn{ &read_action };
      Data data{};

      // ctor
      parser_state(Maker * p_maker, reader_type p_reader, Params const * h_params)
        : params{ h_params }
        , maker{ p_maker }
        , reader{ std::move(p_reader) }
        , position{ h_params->tab_size }
      {}

      void skip_read()
      {
        read_fn = &read_action_none;
      }

      void parse(response_type & resp, Char ch)
      {
        ptr_node & node = nodes.back();
        node->parse(*this, resp, ch);
        if( is_recognized() )
        {
          position.recognized(ch);
        }
        while( after_fn != &action_none )
        {
          after_fn(*this, resp);
        }
      }

      void parser_loop(response_type & response)
      {
        Char ch{};
        for( ;; )
        {
          if( this->read_fn(*this, ch) )
          {
            this->when_done(response);
            break;
          }

          if( this->empty() ) { break; }
          this->parse(response, ch);
          if( this->empty() ) { break; }
        }
        response.end_position = this->position.get();
      }

      void when_done(response_type & resp)
      {
        if( nodes.empty() ) { return; }
        action_unwind(*this, resp);
      }

      bool is_recognized() const
      {
        return (read_fn == &read_action);
      }

      void add_node(ptr_node node)
      {
        nodes.push_back( std::move(node) );
      }

      bool empty() const
      {
        return nodes.empty();
      }
    };


  }; // end nest


} // end ns
