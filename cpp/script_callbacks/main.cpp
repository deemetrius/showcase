
import test_engine; // script engine: some_script

import <optional>;
import <string_view>;

import script_caller;

  // callback_information

// data members: name, value
using my_callback_info = showcase::callback_information<
  std::string_view,
  std::optional<some_script::fn>
>;

// check: the function was found and is ready to be called
template <>
bool my_callback_info::is_ready() const
{
  return value.has_value();
}

  // callback

// the layer around: callback_information
using my_callback = showcase::callback<
  my_callback_info,
  some_script::vm
>;

// find function in loaded script
template <>
void my_callback::find(param_type vm)
{
}

//
template <typename Callback>
struct interface
{
  using callback_type = Callback;
  using param_type = typename callback_type::param_type;

  callback_type
    on_paint{ "onPaint" },
    on_load{ "onLoad" },
    on_error{ "onException" };

  void rescan(param_type vm)
  {
    on_paint.find(vm);
    on_load.find(vm);
    on_error.find(vm);
  }
};

//

int main()
{
  some_script::vm vm;
  interface<my_callback> caller;
  
  caller.rescan(vm);
}
