module;

  /* place #include directives here */

export module test_engine;

  import <string_view>;
  import <map>;
  import <iostream>; // import header like module

  export import <string>; // export from imported module

export namespace some_script {
  // current namespace and its contents
  // are visible for translation unit
  // that imports this module

  using index_t = std::ptrdiff_t;

  using text_view = std::string_view;
  using text = std::string;

  using functions_map = std::map<text, index_t>;
  
  //

  struct e_any {};
  struct e_not_found : public e_any {};

  //

  struct fn
  {
    text_view name{};
    index_t index{};

    bool run() const { std::cout << this->name << "()\n"; }
  };

  struct vm
  {
    static inline functions_map
      map
    {
      {"app::onLoad",   1},
      {"app::onPaint",  2}
    };

    fn find_function(text name) const
    {
      typename functions_map::iterator it{ map.find(name) };
      
      if( it == map.end() )
      {
        throw e_not_found{};
      }
      
      return {it->first, it->second};
    }
  };

  // end ns
}