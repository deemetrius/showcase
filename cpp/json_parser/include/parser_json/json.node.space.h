#pragma once

#include "json.nest.h"

namespace parser::detail {


  template <typename Char, typename Maker, typename Log_string>
  class json_nest<Char, Maker, Log_string>::node_space
    //: public node_base
  {
  protected:
    //using node_base::node_base;
    node_space() = delete;
  public:
    //static std::string get_name() { return "t_space"; }

    static bool condition(json_params const * params, Char ch)
    {
      return
           (ch == info::space)
        || (ch == info::tab)
        || (ch == info::cr)
        || (ch == info::lf)
        ;
    }
    /*
    static ptr_node create(Maker * maker, json_params const * params, pos_type pos)
    {
      return std::make_unique<node_space>(pos);
    }

    static constexpr choicer_type choicer{&get_name, &condition, &create, 0};


    void parse(parser_state & st, response_type & resp, Char ch) override
    {
      if( condition(st.params, ch) == false )
      {
        st.skip_read();
        st.after_fn = &parser_state::action_up_only;
      }
    }

    result_type get_result(parser_state & st) override
    {
      throw skip_result{ this->start_pos };
      return st.maker->make_null(
        this->start_pos
      );
    }*/
  };


} // end ns
