module;

export module connector_test;

export import test_script_engine;

  export import <optional>;
  export import <string>;

  import script_caller;

  import <iostream>;

export namespace connector_test {

  // callback_information

  struct script_params
  {
    using name_type = std::string;
    using function_type = std::optional<test_script_engine::fn>;
  };
  
  using callback_info = script_caller::callback_information<script_params>;

  using callback = script_caller::callback<
    callback_info,
    test_script_engine::vm
  >;

  // end ns
}

export namespace script_caller {

  template <>
  bool connector_test::callback_info::is_ready() const
  {
    return function.has_value();
  }

  template <>
  void connector_test::callback::find(param_type vm)
  {
    std::cout << "\t" << this->name;
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