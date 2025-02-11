#include "Console.hpp"
#include <iostream>
#include <stdarg.h>
#include <string>

using namespace std;

#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"

void dank::console::init() {}
void dank::console::release() {}

void dank::console::log(const char *format, ...) {
  std::string msg = "[log] ";
  msg.append(format);
  msg.append("\n");

  va_list argptr;
  va_start(argptr, format);
  vfprintf(stdout, msg.c_str(), argptr);
  fflush(stdout);
  va_end(argptr);
}

void dank::console::warn(const char *format, ...) {
  std::string msg = "\x1B[33m[warn] ";
  msg.append(format);
  msg.append("\n\x1B[0m");

  va_list argptr;
  va_start(argptr, format);
  vfprintf(stdout, msg.c_str(), argptr);
  fflush(stdout);
  va_end(argptr);
}
