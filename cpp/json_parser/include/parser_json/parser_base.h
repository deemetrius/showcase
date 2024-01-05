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


  template <typename Char, typename Maker>
  struct nest_base
  {
    using reader_type = std::unique_ptr< ksi::lib::reader<Char> >;
    using result_type = Maker::result_type;
    using response_type = parser_response<result_type>;


    struct parser_state;


    struct node_base
    {
      index_t start_char_pos{-1};

      virtual void parse(parser_state & st, response_type & resp, Char ch) = 0;
      virtual result_type get_result(parser_state & st) = 0;
      virtual void put_result(result_type result) {}
    };


    struct parser_state
    {
      using state = std::unique_ptr<node_base>;
      using chain = std::list<state>;

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

      using action_type = decltype( &action_continue );

      Maker const * maker_pointer{ nullptr };
      chain nodes;
      reader_type reader;
      ksi::files::position position;
      action_type next_action{ &action_continue };

      parser_state(Maker const * p_maker, index_t tab_size)
        : maker_pointer{ p_maker }
        , position{tab_size}
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
        return (next_action != &action_ask_parent);
      }

      template <typename Node>
      void add_node()
      {
        state node = std::make_unique<Node>();
        node->start_char_pos = position->char_pos;
        nodes.push_back( std::move(node) );
      }

      bool empty() const
      {
        return nodes.empty();
      }
    };


  }; // end nest


} // end ns
