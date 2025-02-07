#pragma once

#include "modules/input/InputEvent.hpp"
#include <map>

#define NOMINMAX
#include <Windowsx.h>
#include <string>
#include <tchar.h>
#include <windows.h>

#ifdef UNICODE
typedef wchar_t TCHAR;
typedef std::wstring String;
#else
typedef char TCHAR;
typedef std::string String;
#endif

namespace dank {
struct SystemSpecifics {
  HINSTANCE hInstance = 0;
};

static std::wstring s2ws(const std::string &s) {
  int len;
  int slength = (int)s.length() + 1;
  len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
  wchar_t *buf = new wchar_t[len];
  MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
  std::wstring r(buf);
  delete[] buf;
  return r;
}

// https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
static std::map<wchar_t, InputKey> keyMap = {
    {0x08, InputKey::KEY_BACKSPACE}, {0x09, InputKey::KEY_TAB},
    {0x0A, InputKey::KEY_RETURN},    {0x0D, InputKey::KEY_RETURN},
    {0x1B, InputKey::KEY_ESCAPE},    {0x20, InputKey::KEY_SPACE},
    {0x25, InputKey::KEY_LEFT},      {0x26, InputKey::KEY_UP},
    {0x27, InputKey::KEY_RIGHT},     {0x28, InputKey::KEY_DOWN},
    {0x2E, InputKey::KEY_DELETE},    {0x30, InputKey::KEY_0},
    {0x31, InputKey::KEY_1},         {0x32, InputKey::KEY_2},
    {0x33, InputKey::KEY_3},         {0x34, InputKey::KEY_4},
    {0x35, InputKey::KEY_5},         {0x36, InputKey::KEY_6},
    {0x37, InputKey::KEY_7},         {0x38, InputKey::KEY_8},
    {0x39, InputKey::KEY_9},         {0x41, InputKey::KEY_A},
    {0x42, InputKey::KEY_B},         {0x43, InputKey::KEY_C},
    {0x44, InputKey::KEY_D},         {0x45, InputKey::KEY_E},
    {0x46, InputKey::KEY_F},         {0x47, InputKey::KEY_G},
    {0x48, InputKey::KEY_H},         {0x49, InputKey::KEY_I},
    {0x4A, InputKey::KEY_J},         {0x4B, InputKey::KEY_K},
    {0x4C, InputKey::KEY_L},         {0x4D, InputKey::KEY_M},
    {0x4E, InputKey::KEY_N},         {0x4F, InputKey::KEY_O},
    {0x50, InputKey::KEY_P},         {0x51, InputKey::KEY_Q},
    {0x52, InputKey::KEY_R},         {0x53, InputKey::KEY_S},
    {0x54, InputKey::KEY_T},         {0x55, InputKey::KEY_U},
    {0x56, InputKey::KEY_V},         {0x57, InputKey::KEY_W},
    {0x58, InputKey::KEY_X},         {0x59, InputKey::KEY_Y},
    {0x5A, InputKey::KEY_Z}};
} // namespace dank
