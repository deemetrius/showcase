
local json =
@"{
  ""items"": [null, true, -9223372036854775808],
  ""123\"" 1"": +05.01250,
  ""a"": null,
  ""sub"": [{ // "" ""
    ""1"" : 10,
    ""2"" : 20
  }]
} "

con_print_line("\tjson string:")
con_print_line(json)
local result = json_to_value(json)

con_print_line("\tparsed result:")
inspect.print_line(result)
