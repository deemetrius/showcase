module;

export module script_caller;

export namespace script_caller {

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