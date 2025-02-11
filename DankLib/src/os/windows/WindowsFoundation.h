#pragma once

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers.
#define NOMINMAX
#include <Windowsx.h>
#include <windows.h>

#include <shellapi.h>
#include <string>
#include <tchar.h>

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
} // namespace dank
