#define CATCH_CONFIG_RUNNER

#include <catch2.hpp>

#include <Python.h>

int main( int argc, char* argv[] ) {
  Py_Initialize();
  int result = Catch::Session().run(argc, argv);
  return result;
}