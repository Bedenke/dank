#pragma once

#include <stdint.h>

namespace dank {
enum class InputKey : uint32_t {
  KEY_NONE = 0,

  KEY_UP = 10,
  KEY_DOWN = 11,
  KEY_LEFT = 12,
  KEY_RIGHT = 13,

  KEY_SHIFT = 14,
  KEY_CONTROL = 15,
  KEY_ALT = 16,
  KEY_COMMAND = 17,
  KEY_CAPSLOCK = 18,

  KEY_ESCAPE = 19,
  KEY_RETURN = 20,
  KEY_BACKSPACE = 21,
  KEY_DELETE = 22,
  KEY_TAB = 23,

  KEY_BACK_BUTTON = 30,
  KEY_MENU_BUTTON = 31,

  KEY_SPACE = ' ',
  KEY_A = 'A',
  KEY_B = 'B',
  KEY_C = 'C',
  KEY_D = 'D',
  KEY_E = 'E',
  KEY_F = 'F',
  KEY_G = 'G',
  KEY_H = 'H',
  KEY_I = 'I',
  KEY_J = 'J',
  KEY_K = 'K',
  KEY_L = 'L',
  KEY_M = 'M',
  KEY_N = 'N',
  KEY_O = 'O',
  KEY_P = 'P',
  KEY_Q = 'Q',
  KEY_R = 'R',
  KEY_S = 'S',
  KEY_T = 'T',
  KEY_U = 'U',
  KEY_V = 'V',
  KEY_X = 'X',
  KEY_Z = 'Z',
  KEY_W = 'W',
  KEY_Y = 'Y',
  KEY_0 = '0',
  KEY_1 = '1',
  KEY_2 = '2',
  KEY_3 = '3',
  KEY_4 = '4',
  KEY_5 = '5',
  KEY_6 = '6',
  KEY_7 = '7',
  KEY_8 = '8',
  KEY_9 = '9'
};

enum InputEventType {
  KeyDown,
  KeyUp,
  KeyTyped,
  MouseMove,
  MouseDown,
  MouseDrag,
  MouseUp,
  MouseScroll
};

struct InputEvent {
  InputEventType type;
  InputKey key;
  wchar_t character;
  int x;
  int y;
  int wheelDelta;
  int button;
};

} // namespace dank
