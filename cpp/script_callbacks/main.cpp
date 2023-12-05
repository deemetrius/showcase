
import script_caller;

export import <optional>;
export import <string_view>;

import <iostream>;

// script engine

struct some_vm {};
struct some_fn {};

// callback_information: name, value
// is_ready()

using my_callback_info = showcase::callback_information< std::string_view, std::optional<some_fn> >;

template <>
bool my_callback_info::is_ready() const
{
  return value.has_value();
}

/*
  callback
    find()
    operator ()
*/

using my_callback = showcase::callback<my_callback_info, some_vm>;

template <>
void my_callback::find(param_type vm)
{
}

/*
  interface
    rescan()
*/

template <typename Callback>
struct interface
{
  using callback_type = Callback;
  using param_type = typename callback_type::param_type;

  callback_type
    on_paint{ "onPaint" },
    on_load{ "onLoad" };

  void rescan(param_type vm)
  {
    on_paint.find(vm);
    on_load.find(vm);
  }
};

int main()
{
  some_vm vm;
  interface<my_callback> caller;
  
  caller.rescan(vm);

  std::cout << "Hello\n";
}
