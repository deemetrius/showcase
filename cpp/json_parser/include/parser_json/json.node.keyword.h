#pragma once

#include "json.nest.h"

namespace parser::detail {


  template <typename Char, typename Maker, typename Log_string>
  class json_nest<Char, Maker, Log_string>::node_keyword
    : public node_base
  {
  public:
    static std::string get_name() { return "t_keyword"; }

    static bool condition(json_params const * params, Char ch)
    {
      return ksi::chars::is_letter_english_small(ch);
    }

    static ptr_node create(
      Maker * maker,
      json_params const * params,
      pos_type start_pos,
      state_data const & data
    )
    {
      return std::make_unique<node_keyword>(start_pos);
    }

    static constexpr choicer_type choicer{ &get_name, &condition, &create };


    // props
    std::basic_stringstream<Char> stream{};

    using node_base::node_base; // base ctor


    void parse(parser_state & st, response_type & resp, Char ch) override
    {
      if( condition(st.params, ch) )
      {
        stream << ch;
        return;
      }

      // not match
      st.skip_read();
      st.after_fn = &chain_actions::chain_up_result;
    }
    
    result_type get_result(parser_state & st, response_type & resp) override
    {
      typename state_data::map_make_function::const_iterator it = st.data.map_keywords.find( stream.str() );
      if( it == st.data.map_keywords.end() )
      {
        it = st.data.map_keywords.begin();
        resp.change_status(json_message_codes::n_keyword_unknown);
      }
      return it->second(st.maker, this->start_pos, st.data.path);
    }
  };


} // end ns
