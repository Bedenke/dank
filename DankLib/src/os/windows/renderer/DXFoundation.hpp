#pragma once

#include <cstdio>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <stdexcept>
#include <wrl/client.h>

using namespace Microsoft::WRL;

namespace dank {
namespace dx {
inline void ThrowIfFailed(HRESULT hr, const char *msg) {
  if (FAILED(hr)) {
    static char s_str[256] = {0};
    sprintf_s(s_str, "%s | failure with HRESULT of %08X", msg, hr);
    throw std::runtime_error(s_str);
  }
}
} // namespace dx
} // namespace dank
