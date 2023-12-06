import connector_test;

/*
�������������� ��������� ������:
* ������������ ���� ���
* �� ������� �� �������� ����������� ������
- ���� ������ find() �������� � �������
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
  // ����������� ����������� ���������� � ��������� ������
  interface<connector_test::callback> caller;
  
  // ����������� ������ ��� ������������
  test_script_engine::vm vm;
  
  std::cout << "search for callbacks in script\n";
  caller.rescan(vm);

  // interface<> ~ ����� ������������ ��� ������������ �����������
  // � ������ ���������� �������
  // ����� �������� ������� 

  //interface<connector_squirrel::callback> caller_sq;
}

/*
����������� � ����������:
* ���������� ������� �����
* �������� �������� ������ ��������
*/
