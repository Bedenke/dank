#include "DankView.hpp"
#include "modules/engine/Engine.hpp"
#include "modules/input/Input.hpp"
#include "modules/input/InputEvent.hpp"
#include <stdexcept>
#include <winuser.h>

using namespace dank;

bool danking;

dank::Engine engine{};

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
    }
  } break;
  case WM_KEYDOWN: {
    wchar_t c = (wchar_t)wParam;
    InputEvent event{};
    event.type = InputEventType::KeyDown;
    event.key = dank::keyMap[c];
    input.handleInputEvent(event);
  } break;
  case WM_KEYUP: {
    wchar_t c = (wchar_t)wParam;
    InputEvent event{};
    event.type = InputEventType::KeyUp;
    event.key = dank::keyMap[c];
    input.handleInputEvent(event);
  } break;
  case WM_CHAR: {
    wchar_t c = (wchar_t)wParam;
    if (!std::iscntrl(c)) {
      InputEvent event{};
      event.type = InputEventType::KeyDown;
      event.key = dank::keyMap[c];
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

  hWnd = CreateWindowEx(0, className, title, WS_OVERLAPPEDWINDOW, 0, 0, 640,
                        400, NULL, NULL, hInstance, NULL);
}

void DankView::start() {
  danking = true;
  show();
  while (danking) {
    render();
  }
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
}