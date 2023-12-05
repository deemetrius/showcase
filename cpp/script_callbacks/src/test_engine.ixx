module;

  /* place #include directives here */

export module test_engine;

  import <iostream>; // import header like module
  // exprt import _name_; // export from imported module

export namespace some_script {
  // current namespace and its contents
  // are visible for translation unit
  // that imports this module

  struct vm
  {
  };
  
  struct fn
  {
  };

  // end ns
}