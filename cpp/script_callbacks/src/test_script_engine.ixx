module;

  /* #include directives goes here */

export module some_script_engine;

#if 0
  import <string_view>;
  import <map>;
  import <iostream>;

  export import <string>;

export namespace test_script_engine {

  using index_t = std::ptrdiff_t;

  using text_view = std::wstring_view;
  using text = std::wstring;

  using functions_map = std::map<text, index_t>;
  
  // errors

  struct e_any {};
  struct e_not_found : public e_any {};

  // function

  struct fn
  {
    text_view name{};
    index_t index{};

    bool run() const { std::wcout << this->name << "()\n"; }
  };

  // Vm

  struct vm
  {
    static inline functions_map
      map
    {
      {L"onLoad",   1},
      {L"onPaint",  2}
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
#endif