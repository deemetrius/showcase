
inspect <- {}

inspect.post_init <- class
{
  function _mul(table)
  {
    if( typeof(table) != "table" ) { return this }
    
    foreach( key, val in table )
    {
      if( this.rawin(key) )
      {
        this[key] = val
      } else {
        con_print_line(key + " ~ absent property in class: " + this.class_name)
      }
    }
    
    return this
  }
  
  static class_name = "class_name"
  
  static apply = function (some, table)
  {
    if( typeof(table) != "table" ) { return some }
    
    foreach( key, val in table )
    {
      some[key] = val
    }
    
    return some
  }
}

inspect.string_repeat <- function (str, cnt)
{
  local ret = ""
  for( local i = 0; i < cnt; i += 1 )
  {
    ret += str
  }
  return ret
}

inspect.new_array_append <- function (arr, val)
{
  local ret = []
  ret.extend(arr)
  ret.append(val)
  return ret
}

inspect.printer <- class extends inspect.post_init
{
  print = con_print
  print_line = con_print_line
  
  log = con_print
  log_line = con_print_line
}

inspect.projection_default <- {
  "null"      : function (p) { return "null" },
  "bool"      : function (p) { return p.tostring() },
  "integer"   : function (p) { return p.tostring() },
  "float"     : function (p) { return p.tostring() },
  "string"    : function (p) { return ("\"" + p + "\"") },
  "class"     : function (p) { return ("" + p) },
  "instance"  : function (p) { return ("" + p) },
  "function"  : function (p) { return ("" + p) },
  
  "array_index" : function (p) { return ("#" + p) },
  
  "table_key" : function (p)
  {
    if( (typeof p) == "integer" ) { return ("#" + p) }
    if( (typeof p) == "string" ) { return ("\"" + p + "\"") }
    return "?"
  }
}

inspect.folder <- class extends inspect.post_init
{
  start = null
  end = null
  
  proj_key = null
  key_delimiter = null
  
  line_separator = ","
  line_start = ""
  
  function make_key_type_string(key, proj_fn)
  {
    return ((key_delimiter == null) ? "" : (proj_fn(key) + key_delimiter))
  }
}

inspect.params <- class extends inspect.post_init
{
  max_depth = 10
  
  type_ending = ": "  // null
  
  array_start = "["
  array_end   = "]"
  array_index_delimiter   = " ~ "   // null
  array_line_separator    = ""      // ","
  array_line_start        = "\n"    // ""
  
  table_start = "{"
  table_end   = "}"
  table_key_delimiter   = " ~ "
  table_line_separator  = ""
  table_line_start      = "\n"
  
  folders = null
  
  tab_string = "  "   // ""
  
  function make_tabs(depth)
  {
    return (
      (tab_string.len() > 0)
      ? inspect.string_repeat(tab_string, depth)
      : ""
    )
  }
  
  function make_type_string(type)
  {
    return (
      (type_ending == null)
      ? ""
      : (type + type_ending)
    )
  }
  
  function init_folders_maybe()
  {
    if( folders == null ) { init_folders() }
  }
  
  function init_folders()
  {
    folders = {
      "array": inspect.folder() * {
        start = this.array_start
        end = this.array_end
        
        proj_key = "array_index"
        key_delimiter = this.array_index_delimiter
        
        line_separator = this.array_line_separator
        line_start = this.array_line_start
      },
      "table": inspect.folder() * {
        start = this.table_start
        end = this.table_end
        
        proj_key = "table_key"
        key_delimiter = this.table_key_delimiter
        
        line_separator = this.table_line_separator
        line_start = this.table_line_start
      },
    }
  }
}

inspect.messages <- class
{
  function too_deep(depth, max_depth)
  {
    return (
      "(* depth: " + depth + "; max: " + max_depth + " *)"
    )
  }
  
  function unknown_type(type)
  {
    return (
      "(* not supported type: " + type + " *)"
    )
  }
}

inspect.inner_print_folder <- function (stack, printer, projection, params, folder)
{
  local depth = (stack.len() - 1)
  local items = stack.top()
  
  printer.print(folder.start)
  
  local is_first_line = true
  foreach( k, v in items )
  {
    if( is_first_line )
    {
      is_first_line = false
    }
    else
    {
      printer.print(folder.line_separator)
    }
    
    printer.print(folder.line_start)
    
    inspect.inner_print(
      inspect.new_array_append(stack, v),
      printer,
      projection,
      params,
      folder.make_key_type_string(k, projection[folder.proj_key])
    )
  }
  
  if( items.len() > 0 )
  {
    printer.print(folder.line_start)
    printer.print(
      params.make_tabs(depth)
    )
  }
  printer.print(folder.end)
}

inspect.inner_print <- function (stack, printer, projection, params, pre = "")
{
  local depth = (stack.len() - 1)
  
  if( depth > params.max_depth )
  {
    printer.log(
      inspect.messages.too_deep(depth, params.max_depth)
    )
    return
  }
  
  local value = stack.top()
  local type = (typeof value)
  
  printer.print(
    params.make_tabs(depth)
  )
  printer.print(pre)
  printer.print(
    params.make_type_string(type)
  )
  
  params.init_folders_maybe()
  if( type in params.folders )
  {
    inspect.inner_print_folder(stack, printer, projection, params, params.folders[type])
    return
  }
  
  if( type in projection )
  {
    printer.print(
      projection[type](value)
    )
    return
  }
  
  printer.log(
    inspect.messages.unknown_type(type)
  )
}

inspect.print <- function (value)
{
  inspect.inner_print(
    [value],
    inspect.printer(),
    inspect.projection_default,
    inspect.params() * {
      type_ending = null
      array_line_separator = ","
      array_index_delimiter = null
      table_key_delimiter = ": "
    }
  )
}

inspect.print_line <- function (value)
{
  inspect.print(value)
  con_print("\n")
}

inspect.print_typed <- function (value)
{
  inspect.inner_print(
    [value],
    inspect.printer(),
    inspect.projection_default,
    inspect.params()
  )
}
