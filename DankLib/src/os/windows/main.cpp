#include "DankView.h"
#include "modules/engine/Console.hpp"

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
  dank::console::init();
  dank::console::warn("DANK!");
  // dank::DankView view(hInstance);
  // view.start();
  dank::console::release();
  return 0;
}
