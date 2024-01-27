#pragma once

#include "parser_std.h"
#include "../ksi_lib/lib.string_reader.hpp"

#include <optional>
#include <memory>
#include <list>

namespace parser {


  struct exception_skip_result
  {
    ksi::files::position pos{-1, 0, 0};
  };

  struct exception_result_unexpected
  {
    ksi::files::position pos{-1, 0, 0};
  };


  template <typename Result>
  struct parser_response
  {
    using position_type = ksi::files::position;

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
    using result_type = typename Maker::result_type;
    using response_type = parser_response<result_type>;
    using pos_type = ksi::files::position;


    struct parser_state;


    struct node_base
    {
      // props
      pos_type start_pos{-1, 0, 0};

      // ctor
      node_base(pos_type pos)
        : start_pos{ pos }
      {}

      // actions

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

      static ptr_node create_none(
        Maker * maker, Params const * params, pos_type start_pos, Data const & data
      )
      { return std::make_unique<node_base>(start_pos); }

      using fn_name = decltype( &get_name );
      using fn_condition = decltype( &condition_false );
      using fn_create = decltype( &create_none );


      fn_name name{ &get_name };
      fn_condition condition{ &condition_false };
      fn_create create{ &create_none };


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


    struct read_actions
    {
    private:
      static bool try_read_next(read_actions & st, Char & ch)
      {
        if( st.reader->is_end() ) { return true; }
        ch = st.reader->read_char();
        return false;
      }

      static bool keep_current(read_actions & st, Char & ch)
      {
        st.read_fn = &try_read_next;
        return false;
      }

      using fn_type = decltype(&try_read_next);

      // props
    public:
      reader_type reader;
      fn_type read_fn{ &try_read_next };

      void skip_read()
      {
        read_fn = &keep_current;
      }

      bool is_recognized() const
      {
        return (read_fn == &read_actions::try_read_next);
      }
    };

    struct chain_actions
    {
      static void chain_none(parser_state & st, response_type & resp)
      {}

      using action_type = decltype(&chain_none);

      static void chain_up_result(parser_state & st, response_type & resp)
      {
        st.after_fn = &chain_none;
        inner_result_up(st, resp);
      }

      static void chain_up_only(parser_state & st, response_type & resp)
      {
        st.after_fn = &chain_none;
        st.nodes.pop_back();
      }

      static void chain_unwind(parser_state & st, response_type & resp)
      {
        st.after_fn = &chain_none;
        while( st.nodes.empty() == false )
        {
          inner_result_up(st, resp);
        }
      }

      static void chain_discard(parser_state & st, response_type & resp)
      {
        st.after_fn = &chain_none;
        st.nodes.clear();
      }

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

      using chain = std::list<ptr_node>;

      // props
      action_type after_fn{ &chain_none };
      chain nodes;

      bool has_chain_action() const
      {
        return (after_fn != &chain_none);
      }

      bool empty() const
      {
        return nodes.empty();
      }

      void add_node(ptr_node node)
      {
        nodes.push_back(std::move(node));
      }
    };

    struct parser_state
      : public read_actions
      , public chain_actions
    {
      // props
      Params const * params{ nullptr };
      Maker * maker{ nullptr };
      ksi::files::position_counter position;
      Data data{};

      // ctor
      template <typename ... Args_data>
      parser_state(
        Maker * p_maker, reader_type p_reader, Params const * h_params, Args_data ... args_data
      )
        : read_actions{ std::move(p_reader) }
        , params{ h_params }
        , maker{ p_maker }
        , position{ h_params->tab_size }
        , data{ args_data ... }
      {}

      void parse(response_type & resp, Char ch)
      {
        ptr_node & node = this->nodes.back();
        node->parse(*this, resp, ch);
        if( this->is_recognized() )
        {
          position.recognized(ch);
        }
        while( this->has_chain_action() )
        {
          this->after_fn(*this, resp);
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
        if( this->nodes.empty() ) { return; }
        this->nodes.back()->input_ended(*this, resp);
        chain_actions::chain_unwind(*this, resp);
      }
    };


  }; // end nest


} // end ns
