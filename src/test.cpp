#define CATCH_CONFIG_RUNNER

#include <catch2.hpp>

#include <Python.h>

struct LogPyErrors : Catch::TestEventListenerBase {

    using TestEventListenerBase::TestEventListenerBase; // inherit constructor
    
    void testCaseStarting( Catch::TestCaseInfo const& testInfo ) override {
        if (PyErr_Occurred()) {
          
          PyErr_PrintEx(0);
          printf("\x1b[0m");
        }
    }

    void testCaseEnded( Catch::TestCaseStats const& testCaseStats ) override {
        if (PyErr_Occurred()) {
          printf("\n┌\x1b[31mPython Exception left unhandled\x1b[0m\n");
          printf("├\x1b[34mTest: \x1b[1m%s\x1b[0m\n", testCaseStats.testInfo.name.c_str());
          printf("├\x1b[33mFile: \x1b[1m%s:%lu\x1b[0m\n", 
                 testCaseStats.testInfo.lineInfo.file, 
                 testCaseStats.testInfo.lineInfo.line
          );
        	printf("┴\x1b[31;1m");
        	PyErr_PrintEx(0);
        	printf("\x1b[0m\n");
        }
    }
};
CATCH_REGISTER_LISTENER( LogPyErrors )


int main( int argc, char* argv[] ) {
  Py_Initialize();
  int result = Catch::Session().run(argc, argv);
  return result;
}