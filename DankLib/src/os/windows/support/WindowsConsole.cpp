#include "modules/engine/Console.hpp"
#include "os/windows/WindowsFoundation.h"
#include <conio.h>
#include <iostream>
#include <stdarg.h>

using namespace std;

bool RedirectConsoleIO() {
  bool result = true;
  FILE *fp;

  // Redirect STDIN if the console has an input handle
  if (GetStdHandle(STD_INPUT_HANDLE) != INVALID_HANDLE_VALUE)
    if (freopen_s(&fp, "CONIN$", "r", stdin) != 0) {
      result = false;
    } else {
      setvbuf(stdin, NULL, _IONBF, 0);
    }

  // Redirect STDOUT if the console has an output handle
  if (GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE)
    if (freopen_s(&fp, "CONOUT$", "w", stdout) != 0) {
      result = false;
    } else {
      setvbuf(stdout, NULL, _IONBF, 0);
    }

  // Redirect STDERR if the console has an error handle
  if (GetStdHandle(STD_ERROR_HANDLE) != INVALID_HANDLE_VALUE)
    if (freopen_s(&fp, "CONOUT$", "w", stderr) != 0) {
      result = false;
    } else {
      setvbuf(stderr, NULL, _IONBF, 0);
    }

  // Make C++ standard streams point to console as well.
  ios::sync_with_stdio(true);

  // Clear the error state for each of the C++ standard streams.
  std::wcout.clear();
  std::cout.clear();
  std::wcerr.clear();
  std::cerr.clear();
  std::wcin.clear();
  std::cin.clear();

  return result;
}

bool ReleaseConsole() {
  bool result = true;
  FILE *fp;

  // Just to be safe, redirect standard IO to NUL before releasing.

  // Redirect STDIN to NUL
  if (freopen_s(&fp, "NUL:", "r", stdin) != 0)
    result = false;
  else
    setvbuf(stdin, NULL, _IONBF, 0);

  // Redirect STDOUT to NUL
  if (freopen_s(&fp, "NUL:", "w", stdout) != 0)
    result = false;
  else
    setvbuf(stdout, NULL, _IONBF, 0);

  // Redirect STDERR to NUL
  if (freopen_s(&fp, "NUL:", "w", stderr) != 0)
    result = false;
  else
    setvbuf(stderr, NULL, _IONBF, 0);

  // Detach from console
  if (!FreeConsole())
    result = false;

  return result;
}

void AdjustConsoleBuffer(int16_t minLength) {
  // Set the screen buffer to be big enough to scroll some text
  CONSOLE_SCREEN_BUFFER_INFO conInfo;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &conInfo);
  if (conInfo.dwSize.Y < minLength)
    conInfo.dwSize.Y = minLength;
  SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), conInfo.dwSize);
}

bool CreateNewConsole(int16_t minLength) {
  bool result = false;

  // Release any current console and redirect IO to NUL
  ReleaseConsole();

  // Attempt to create new console
  if (AllocConsole()) {
    AdjustConsoleBuffer(minLength);
    result = RedirectConsoleIO();
  }

  return result;
}

void dank::console::init() { CreateNewConsole(1024); }
void dank::console::release() { ReleaseConsole(); }

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
  std::string msg = "[warn] ";
  msg.append(format);
  msg.append("\n");

  va_list argptr;
  va_start(argptr, format);
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
  vfprintf(stdout, msg.c_str(), argptr);
  fflush(stdout);
  va_end(argptr);
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                          FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN);
}

// void dank::console::pause()
// {
//     system("pause");
// }