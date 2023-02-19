#ifndef UTIL
#define UTIL

#include <stdio.h>
#include <stdlib.h>

#define _stringify(x) #x
#define stringify(x) _stringify(x)

#define ERROR_MSG_LEN       100
void panic(const char* const message) {
  printf("\033[31m%s\033[0m\n", message);
  exit(-1);
}

#endif
