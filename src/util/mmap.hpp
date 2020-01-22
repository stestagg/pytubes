#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include "mmap_win.hpp"
#else
#include "mmap_posix.hpp"
#endif