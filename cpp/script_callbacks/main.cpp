
import test_engine; // script engine: some_script

import <optional>;
import <string>;

import script_caller;

import <iostream>;

  // callback_information

struct test_script_params
  : public showcase::is_script_params
  <
  std::string,
  std::optional<some_script::fn>
  >
{};

// data members: name, value
using my_callback_info = showcase::callback_information<test_script_params>;

// check: the function was found and is ready to be called
template <>
bool my_callback_info::is_ready() const
{
  return function.has_value();
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

//
template <typename Callback>
struct interface
{
  using callback_type = Callback;
  using param_type = typename callback_type::param_type;

  callback_type
    on_paint{ "app::onPaint" },
    on_load { "app::onLoad" },
    on_error{ "app::onException" };

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
  
  std::cout << "rescan started\n";
  caller.rescan(vm);
}
