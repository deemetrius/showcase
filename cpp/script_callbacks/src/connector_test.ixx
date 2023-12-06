module;

export module connector_test;

#if 0
export import test_script_engine;

  export import <optional>;
  export import <string>;

  import script_caller;

  import <iostream>;

export namespace connector_test {

  // callback

  struct script_params
  {
    using text_type = std::wstring;

    using function_type = std::optional<test_script_engine::fn>;
    using function_result_type = bool;

    using vm_pass_type = test_script_engine::vm &;
  };
  
  using callback = script_caller::callback<script_params>;

  // end ns
}

export namespace script_caller {

  template <>
  bool connector_test::callback::is_ready() const
  {
    return function.has_value();
  }

  template <>
  void connector_test::callback::find_in(vm_pass_type vm)
  {
    std::wcout << "\t" << this->name;
    try
    {
      this->function = vm.find_function(this->name);
      std::cout << " found\n";
    }
    catch( ... )
    {
      this->function.reset();
      std::cout << " ~ missing\n";
    }
  }

  // end ns
}
#endif