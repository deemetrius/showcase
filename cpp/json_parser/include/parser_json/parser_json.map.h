#pragma once


namespace parser::detail {


  template <typename Char, typename Maker>
  class nest_json<Char, Maker>::node_map
    : public node_base
  {
  public:
    static bool condition(json_params const * params, Char ch)
    {
      return (ch == info::brace_open);
    }

    static ptr_node create(Maker * maker, json_params const * params, pos_type start_pos)
    {
      return std::make_unique<node_map>(start_pos, maker);
    }

    static constexpr choicer_type choicer{&condition, &create};


    using map_type = Maker::map;
    using text_type = Maker::text;

    enum kind : std::size_t
    {
      kind_close     = (1 << 0),
      kind_key       = (1 << 1),
      kind_delimiter = (1 << 2),
      kind_value     = (1 << 3),
      kind_separator = (1 << 4),

      was_open      = (kind_close | kind_key),
      was_key       = (kind_delimiter),
      was_delimiter = (kind_value),
      was_value     = (kind_close | kind_separator),
      was_separator = (kind_key),
    };

    map_type map;
    kind req{ was_open };
    text_type key{};

    node_map(pos_type pos, Maker * maker)
      : node_base{ pos }
      , map{ maker->make_map(pos) }
    {}
  };


} // end ns
