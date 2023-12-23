# showcase

## find ways

### Поиск скриптовых колбэков

Перед компиляцией скопируйте в директорию: ``cpp/_libs``
<br>... следующие файлы:
```
_simplesquirrel.lib
_squirrel.lib
```

И поместите туда же папку с биндингом: ``simplesquirrel``

### Загрузчик шрифтов

Создайте каталог `fonts` внутри пути `cpp\font_loader`
<br>Поместите туда шрифт ttf

Придётся в файле `cpp\font_loader\main_font_loader.cpp` указать настройки
<br>Строка 34 и 35