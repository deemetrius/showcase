#pragma once

namespace parser {


  using index_t = std::ptrdiff_t;


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
      //index_t start_char_pos{-1};
      pos_type start_pos{-1, 0, 0};

      virtual void parse(parser_state & st, response_type & resp, Char ch) = 0;
      virtual result_type get_result(parser_state & st) = 0;
      virtual void put_result(result_type result) {}
    };


    using state = std::unique_ptr<node_base>;


    struct choicer
    {
      static bool condition_false(Params const *, Char) { return false; }
      static state create_none(Params const *) { return {}; }

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
      Maker const * maker{ nullptr };
      chain nodes;
      reader_type reader;
      ksi::files::position position;
      action_type next_action{ &action_continue };

      parser_state(Maker const * p_maker, Params const * h_params)
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
        //node->start_char_pos = position->char_pos;
        node->start_pos = position.get();
        nodes.push_back( std::move(node) );
      }

      bool empty() const
      {
        return nodes.empty();
      }
    };


  }; // end nest


} // end ns
