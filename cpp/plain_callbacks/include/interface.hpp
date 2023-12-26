#pragma once

struct interface
  : public connector_squirrel::is_interface<interface>
{
  connector_squirrel::callback
    on_paint{ "onPaint" },
    on_load{ "onLoad" },
    on_error{ "onException" };

  static inline std::vector<connector_squirrel::callback interface::*> for_bind{
    &interface::on_paint,
    &interface::on_load,
    &interface::on_error
  };

  connector_squirrel::params::integer wrap_paint(connector_squirrel::params::vm_pass_type vm)
  {
    // это пример функции - обёртки

    if (on_paint.is_ready())
    {
      // допустим тут вычисление параметров

      ssq::Object res = on_paint(vm, 1);

      return res.toInt();
    }
    return -1;
  }
};
