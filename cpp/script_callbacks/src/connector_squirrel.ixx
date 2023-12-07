module;

  #include <simplesquirrel/simplesquirrel.hpp>

export module connector_squirrel;

  export import <optional>;
  export import script_caller;

export namespace connector_squirrel {

  struct script_params
  {
    using text_type = ssq::sqstring;

    using function_type = std::optional<ssq::Function>;
    using function_result_type = ssq::Object;

    using vm_pass_type = ssq::VM &;

    static function_type function_find(vm_pass_type vm, const text_type & name)
    {
      try
      {
        return vm.findFunc( name.c_str() );
      }
      catch( ... )
      {
        return std::nullopt;
      }
    }

    static bool function_is_ready(const function_type & fn)
    {
      return fn.has_value();
    }

    template <typename ... Args>
    static function_result_type function_call(function_type fn, vm_pass_type vm, Args && ... args)
    {
      return vm.callFunc(fn.value(), vm, std::forward<Args>(args) ...);
    }
  };

  using callback = script_caller::callback<script_params>;

  // end ns
}