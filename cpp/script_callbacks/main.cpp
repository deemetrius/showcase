import connector_test;

/*
предполагаемый интерфейс вызова:
* определяется один раз
* не зависит от целевого скриптового движка
- пока только find() колбэков в скрипте
*/

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

// entry

int main()
{
  // подстановка конкретного коннектора в интерфейс вызова
  interface<connector_test::callback> caller;
  
  // виртуальная машина для тестирования
  test_script_engine::vm vm;
  
  std::cout << "search for callbacks in script\n";
  caller.rescan(vm);

  // interface<> ~ можно тиражировать для произвольных коннекторов
  // к прочим скриптовым движкам
  // через параметр шаблона 

  //interface<connector_squirrel::callback> caller_sq;
}

/*
планируется в дальнейшем:
* подключить биндинг белки
* добавить механизм вызова колбэков
*/
