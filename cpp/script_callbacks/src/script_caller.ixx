module;

export module script_caller;

export namespace showcase {

  // params

  template <typename String_type, typename Function_type>
  struct is_script_params
  {
    using name_type = String_type;
    using function_type = Function_type;
  };

  // info: callback

  template <typename Script_params>
  struct callback_information
  {
    using params_type = Script_params;

    typename params_type::name_type name;
    typename params_type::function_type function;

    bool is_ready() const;
  };

  // callback

  template <typename Info_type, typename Vm_type>
  struct callback
    : public Info_type
  {
    using info_type = Info_type;
    using param_type = Vm_type &;

    void find(param_type vm);
  };

  // end ns
}