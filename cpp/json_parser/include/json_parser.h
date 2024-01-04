#pragma once

#include "parser_base.h"

namespace parser {

  struct json_status
  {
    enum status
    {
      n_ok,
      n_number_double_dot
    };
  };


  template <typename Maker>
  struct json
  {
    using result_type = Maker::result_type;
    using response_type = parser_response<result_type>;

    template <typename String>
    static response_type from_string(Maker const & maker, String source, index_t tab_size = 4);
  };


} // end ns

namespace parser::detail {


  template <typename Char, typename Maker>
  struct nest_json
  {
    using result_type = Maker::result_type;
    using parser_state = nest_base<Char, Maker>::parser_state;
    using node_base = nest_base<Char, Maker>::node_base;
    using info = ksi::chars::info<Char>;
    using response_type = nest_base<Char, Maker>::response_type;


    class node_number : public node_base
    {
    public:
      static constexpr index_t radix{ 10 };

    protected:
      index_t before_dot{ 0 };
      index_t after_dot{ 0 };
      index_t dot{ -1 };
      bool is_negative{ false };

      void on_digit(index_t digit)
      {
        if( dot < 0 )
        {
          before_dot *= radix;
          before_dot += digit;
        }
        else
        {
          after_dot *= radix;
          after_dot += digit;
          dot *= radix;
        }
      }

      void on_dot(parser_state & st, response_type & resp, Char ch)
      {
        if( dot > -1 )
        {
          resp.status = json_status::n_number_double_dot;
          st.next_action = &parser_state::action_ask_parent;
          return;
        }
        dot = 1;
      }

    public:
      void parse(parser_state & st, response_type & resp, Char ch) override
      {
        if( ksi::chars::is_digit(ch) )
        {
          on_digit( ksi::chars::digit_of(ch) );
        }
        else if( ch == info::dot )
        {
          on_dot(st, resp, ch);
        }
        else
        {
          st.next_action = &parser_state::action_ask_parent;
        }
      }

      result_type get_result(parser_state & st) override
      {
        if( dot < 0 )
        {
          return st.maker_pointer->make_integer(before_dot);
        }
        else
        {
          typename Maker::floating part{ static_cast<Maker::floating>(before_dot) };
          typename Maker::floating frac{ static_cast<Maker::floating>(after_dot) };
          frac /= dot;
          part += frac;
          return st.maker_pointer->make_floating(is_negative ? -part : part);
        }
      }
    };


    struct node_top : public node_base
    {
      void parse(parser_state & st, response_type & resp, Char ch) override
      {}
    };
  };


} // end ns

namespace parser {


  template <typename Maker>
  template <typename String>
  inline json<Maker>::response_type
    json<Maker>::from_string(Maker const & maker, String source, index_t tab_size)
  {
    using reader_type = ksi::lib::string_reader<String>;
    using char_type = decltype( std::declval<reader_type>().read_char() );
    using state_type = detail::nest_base<char_type, Maker>::parser_state;
    using nest = detail::nest_json<char_type, Maker>;

    state_type state{ &maker };
    state.reader = std::make_unique<reader_type>(source);
    state.nodes.push_back( std::make_unique<nest::node_number>() );
    
    response_type response;
    ksi::files::position position{ tab_size };
    char_type ch{};

    while( state.reader->is_end() == false )
    {
      state.next_action(state, response, ch);
      state.parse(response, ch);
      if( state.is_recognized() )
      {
        position.recognized(ch);
      }
    }
    state_type::action_ask_parent(state, response, ch);
    response.position = position.get();
    std::cout << "parse done\n";
    return response;
  }


} // end ns
