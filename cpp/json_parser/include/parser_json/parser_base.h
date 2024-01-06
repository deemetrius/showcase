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


  struct unexpected_result
  {
    ksi::files::position::data_type pos{-1, 0, 0};
  };


  template <typename Result>
  struct parser_response
  {
    using position_type = ksi::files::position::data_type;

    std::optional<Result> value;
    index_t status{0};
    position_type position{};
  };


} // end ns

namespace parser::detail {


  template <typename Char, typename Maker, typename Params>
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

      virtual result_type get_result(parser_state & st)
      {
        throw unexpected_result{ start_pos };
      }

      virtual void put_result(result_type result) {}
    };


    using state = std::unique_ptr<node_base>;


    struct choicer
    {
      static bool condition_false(Params const * params, Char ch)
      { return false; }

      static state create_none(Maker * maker, Params const * params, pos_type start_pos)
      { return std::make_unique<node_base>(start_pos); }

      using fn_condition = decltype( &condition_false );
      using fn_create = decltype( &create_none );

      fn_condition condition{ &condition_false };
      fn_create create{ &create_none };
      index_t type{1};
    };


    struct parser_state
    {
      using chain = std::list<state>;

      static void action_none(parser_state & st, response_type & resp, Char & ch)
      {
        st.next_action = &parser_state::action_continue;
      }

      static void action_continue(parser_state & st, response_type & resp, Char & ch)
      {
        ch = st.reader->read_char();
      }

      static void action_up(parser_state & st, response_type & resp, Char & ch)
      {
        st.next_action = &parser_state::action_continue;
        result_type result = st.nodes.back()->get_result(st);
        st.nodes.pop_back();
        st.nodes.back()->put_result(result);
        ch = st.reader->read_char();
      }

      static void action_ask_parent(parser_state & st, response_type & resp, Char & ch)
      {
        st.next_action = &parser_state::action_continue;
        result_type result = st.nodes.back()->get_result(st);
        st.nodes.pop_back();
        if( st.nodes.empty() )
        {
          resp.value = result;
          resp.status = is_status::e_unexpected_symbol;
          resp.position = st.position.get();
          return;
        }
        st.nodes.back()->put_result(result);
      }

      static void action_ask_parent_no_value(parser_state & st, response_type & resp, Char & ch)
      {
        st.next_action = &parser_state::action_continue;
        st.nodes.pop_back();
      }

      static void action_exit(parser_state & st, response_type & resp, Char & ch)
      {
        st.nodes.clear();
      }

      using action_type = decltype( &action_continue );

      Params const * params{ nullptr };
      Maker * maker{ nullptr };
      chain nodes;
      reader_type reader;
      ksi::files::position position;
      action_type next_action{ &action_continue };

      parser_state(Maker * p_maker, Params const * h_params)
        : params{ h_params }
        , maker{ p_maker }
        , position{ h_params->tab_size }
      {}

      void parse(response_type & resp, Char ch)
      {
        state & node = nodes.back();
        node->parse(*this, resp, ch);
        if( is_recognized() )
        {
          position.recognized(ch);
        }
      }

      bool is_recognized() const
      {
        return
           (next_action == &action_continue)
        || (next_action == &action_up)
        ;
      }

      void add_node(state node)
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
