module;

export module script_caller;

export namespace showcase {

  template <typename String_type, typename Value_type>
  struct callback_information
  {
    using name_type = String_type;
    using value_type = Value_type;

    name_type name;
    value_type value;

    bool is_ready() const;
  };

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