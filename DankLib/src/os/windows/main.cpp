#include "DankView.hpp"

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
  dank::DankView view(hInstance);
  view.start();
  return 0;
}
