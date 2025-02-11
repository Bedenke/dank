#include "DankView.h"
#include "modules/engine/Engine.hpp"
#include "modules/input/Input.hpp"
#include "modules/input/InputEvent.hpp"
#include "os/windows/renderer/DXRenderer.hpp"
#include <stdexcept>
#include <winuser.h>

using namespace dank;

bool danking;

dank::Engine engine{};
dank::dx::DXRenderer renderer{};

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

LRESULT CALLBACK WindowsSurfaceWndProc(HWND hwnd, UINT msg, WPARAM wParam,
                                       LPARAM lParam) {
  TouchData touchData{};
  touchData.x = (float)GET_X_LPARAM(lParam);
  touchData.y = (float)GET_Y_LPARAM(lParam);

  switch (msg) {
  case WM_CLOSE: {
    danking = false;
    return 0;
  } break;
  case WM_DESTROY:
    return DefWindowProc(hwnd, msg, wParam, lParam);

  case WM_SIZE: {
    RECT rect;
    if (GetWindowRect(hwnd, &rect)) {
      int width = rect.right - rect.left;
      int height = rect.bottom - rect.top;
      engine.onViewResize(width, height);
      // TODO: restart renderer
    }
  } break;
  case WM_KEYDOWN: {
    wchar_t c = (wchar_t)wParam;
    InputEvent event{};
    event.type = InputEventType::KeyDown;
    event.key = keyMap[c];
    input.handleInputEvent(event);
  } break;
  case WM_KEYUP: {
    wchar_t c = (wchar_t)wParam;
    InputEvent event{};
    event.type = InputEventType::KeyUp;
    event.key = keyMap[c];
    input.handleInputEvent(event);
  } break;
  case WM_CHAR: {
    wchar_t c = (wchar_t)wParam;
    if (!std::iscntrl(c)) {
      InputEvent event{};
      event.type = InputEventType::KeyDown;
      event.key = keyMap[c];
      input.handleInputEvent(event);
    }
  } break;
  case WM_LBUTTONDOWN: {
    InputEvent event{};
    event.type = InputEventType::MouseDown;
    event.x = touchData.x;
    event.y = touchData.y;
    event.button = TouchButton::TB_LEFT;
    input.handleInputEvent(event);
  } break;
  case WM_LBUTTONUP: {
    InputEvent event{};
    event.type = InputEventType::MouseUp;
    event.x = touchData.x;
    event.y = touchData.y;
    event.button = TouchButton::TB_LEFT;
    input.handleInputEvent(event);
  } break;
  case WM_MBUTTONDOWN: {
    InputEvent event{};
    event.type = InputEventType::MouseDown;
    event.x = touchData.x;
    event.y = touchData.y;
    event.button = TouchButton::TB_MIDDLE;
    input.handleInputEvent(event);
  } break;
  case WM_MBUTTONUP: {
    InputEvent event{};
    event.type = InputEventType::MouseUp;
    event.x = touchData.x;
    event.y = touchData.y;
    event.button = TouchButton::TB_MIDDLE;
    input.handleInputEvent(event);
  } break;
  case WM_RBUTTONDOWN: {
    InputEvent event{};
    event.type = InputEventType::MouseDown;
    event.x = touchData.x;
    event.y = touchData.y;
    event.button = TouchButton::TB_RIGHT;
    input.handleInputEvent(event);
  } break;
  case WM_RBUTTONUP: {
    InputEvent event{};
    event.type = InputEventType::MouseUp;
    event.x = touchData.x;
    event.y = touchData.y;
    event.button = TouchButton::TB_RIGHT;
    input.handleInputEvent(event);
  } break;
  case WM_MOUSEWHEEL: {
    InputEvent event{};
    event.type = InputEventType::MouseScroll;
    event.wheelDelta = (int)GET_WHEEL_DELTA_WPARAM(wParam);
    input.handleInputEvent(event);
  } break;
  case WM_MOUSEMOVE: {
    InputEvent event{};
    event.type = InputEventType::MouseUp;
    event.x = touchData.x;
    event.y = touchData.y;
    input.handleInputEvent(event);
  } break;
  }

  return DefWindowProc(hwnd, msg, wParam, lParam);
}

DankView::DankView(HINSTANCE hInstance) {
  this->hInstance = hInstance;

  LPCSTR className = "DankView";
  LPCSTR title = "Dank";

  WNDCLASSEX wc{};
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.style = 0;
  wc.lpfnWndProc = WindowsSurfaceWndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = className;
  wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

  if (!RegisterClassEx(&wc))
    throw std::runtime_error("Failed to register window");

  hWnd = CreateWindowEx(0, className, title, WS_OVERLAPPEDWINDOW, 0, 0, 800,
                        600, NULL, NULL, hInstance, NULL);
}

void DankView::start() {
  danking = true;
  show();
  renderer.init(hWnd, 800, 600);
  while (danking) {
    render();
  }
  renderer.release();
}

void DankView::show() {
  ShowWindow(hWnd, SW_SHOW);
  UpdateWindow(hWnd);
}

void DankView::render() {
  MSG msg;
  if (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  engine.update();
  renderer.render(engine.ctx, engine.scene);
}