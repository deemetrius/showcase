module;

export module script_caller;

  import <utility>;

export namespace script_caller {

  template <typename Script_params>
  struct callback
  {
    using params_type = Script_params;

    using text_type = typename params_type::text_type;

    using function_type = typename params_type::function_type;
    using function_result_type = typename params_type::function_result_type;

    using vm_pass_type = typename params_type::vm_pass_type;

    text_type name;
    function_type function;

    bool find_in(vm_pass_type vm)
    {
      function = params_type::function_find(vm, name);
      return is_ready();
    }

    bool is_ready() const
    {
      return params_type::function_is_ready(function);
    }

    template <typename ... Args>
    function_result_type operator () (vm_pass_type vm, Args && ... args)
    {
      return params_type::function_call(function, vm, std::forward<Args>(args) ...);
    }
  };

  // end ns
}