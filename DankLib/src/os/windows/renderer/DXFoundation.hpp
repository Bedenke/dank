#pragma once

#include "modules/engine/Console.hpp"
#include "modules/input/InputEvent.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers.
#endif

#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>

#define NOMINMAX
#include <windows.h>

#include <cstdio>
#include <map>
#include <shellapi.h>
#include <stdexcept>
#include <string>
#include <tchar.h>
#include <wrl.h>

using namespace Microsoft::WRL;

namespace dank {
namespace dx {
inline void ThrowIfFailed(HRESULT hr, const char *msg) {
  if (FAILED(hr)) {
    static char s_str[256] = {0};
    sprintf_s(s_str, "%s | failure with HRESULT of %08X", msg, hr);
    dank::console::log(s_str);
    throw std::runtime_error(s_str);
  }
}
} // namespace dx
} // namespace dank
