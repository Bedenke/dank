#pragma once

#include "WindowsFoundation.h"

namespace dank {
class DankView {
private:
  void show();
  void render();

public:
  HINSTANCE hInstance;
  HWND hWnd;

  DankView(HINSTANCE hInstance);

  ~DankView() { DestroyWindow(hWnd); }

  void start();
};

} // namespace dank
