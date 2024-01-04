#pragma once

namespace parser {


  using index_t = std::ptrdiff_t;

  template <typename Result>
  struct parser_response
  {
    using position_type = ksi::files::position::data_type;

    std::optional<Result> value;
    index_t status;
    position_type position;
  };

} // end ns

namespace parser::detail {


  enum class action
  {
    n_continue,
    n_ask_parent,
    n_end_node
  };


  template <typename Char, typename Maker>
  struct nest_base
  {
    using reader_type = std::unique_ptr< ksi::lib::reader<Char> >;
    using result_type = Maker::result_type;
    using response_type = parser_response<result_type>;


    struct parser_state;


    struct node_base
    {
      virtual void parse(parser_state & st, response_type & resp, Char ch) = 0;
      virtual result_type get_result(parser_state & st) = 0;
      virtual void put_result(result_type && result) {}
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
        result_type result = st.nodes.back()->get_result();
        st.nodes.pop_back();
        st.nodes.back()->put_result( std::move(result) );
        ch = st.reader->read_char();
      }

      static void action_ask_parent(parser_state & st, response_type & resp, Char & ch)
      {
        result_type result = st.nodes.back()->get_result(st);
        st.nodes.pop_back();
        if( st.nodes.empty() )
        {
          resp.value = std::move(result);
          return;
        }
        st.nodes.back()->put_result( std::move(result) );
      }

      using action_type = decltype( &action_continue );

      Maker const * maker_pointer{ nullptr };
      chain nodes;
      reader_type reader;
      action_type next_action{ &action_continue };

      parser_state(Maker const * p_maker)
        : maker_pointer{ p_maker }
      {}

      void parse(response_type & resp, Char ch)
      {
        state & node = nodes.back();
        node->parse(*this, resp, ch);
      }

      bool is_recognized() const
      {
        return (next_action != &action_ask_parent);
      }
    };


  }; // end nest


} // end ns
