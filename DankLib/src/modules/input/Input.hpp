#pragma once
#include "modules/input/InputEvent.hpp"
#include <map>

namespace dank {

enum TouchButton : int { TB_LEFT = 0, TB_RIGHT = 1, TB_MIDDLE = 2 };

struct TouchData {
  float x;
  float y;
  int button;
};

typedef unsigned short InputFlags;

enum TouchActions : unsigned short {
  TA_NONE = 0,
  TA_HOVER = 1u << 0,
  TA_TOUCH = 1u << 1,
  TA_DRAG = 1u << 2,
  TA_DROP = 1u << 3,
  TA_LONG_PRESS = 1u << 4,
  TA_RELEASE = 1u << 5,
  TA_TAP = 1u << 6,
  TA_DOUBLE_TAP = 1u << 7,
  TA_TOUCHED = 1u << 8
};

struct TouchState {
  float x = 0;
  float y = 0;
  float dx = 0;
  float dy = 0;
  float vx = 0;
  float vy = 0;
  int button;
  InputFlags actions;
  bool hasAction(TouchActions action) const {
    return (actions & action) == action;
  }
  bool hasActions(InputFlags action) const { return (actions & action) != 0; }
};

struct InputOptions {
  float absoluteTime = 0;
  // Threshold in Pixels for hover and drag detected
  int dragThreshold = 10;
  // Threshold in Seconds to detect long press
  float longPressThreshold = 1.0f;
  float doubleTapThreshold = 0.3f;

  int centerX = 0;
  int centerY = 0;
};

class Touch {
private:
  bool touching = false;
  bool detectedTouched = false;
  bool detectedRelease = false;
  bool detectedDoubleTap = false;
  bool detectedMove = false;
  bool detectedLongPress = false;
  bool detectedDrag = false;
  double longPressTime = 0;
  double lastTapTime = 0;
  float lastX = 0, lastY = 0;

public:
  float startX = 0, startY = 0;
  float velX = 0, velY = 0;
  float x = 0;
  float y = 0;
  int button = 0;
  unsigned short actions = TouchActions::TA_NONE;
  void handleTouchMove(const TouchData &data, const InputOptions &input);
  void handleTouchDown(const TouchData &data, const InputOptions &input);
  void handleTouchUp(const TouchData &data, const InputOptions &input);
  bool update(const float dt, const InputOptions &input);
};

class KeyInputListener {
public:
  virtual void onKeyDown(InputKey key) = 0;
  virtual void onKeyUp(InputKey key) = 0;
  virtual void onChar(wchar_t c) = 0;
};

const static int MAX_INPUT_TOUCHES = 10;

enum InputKeyState : unsigned short {
  IKS_UP = 0,
  IKS_DOWN = 1u << 0,
  IKS_TRIGGERING = 1u << 1,
  IKS_TRIGGERED = 1u << 2
};

class Input {
private:
  Touch touches[MAX_INPUT_TOUCHES];
  int touchCount = 0;
  int keyCount = 0;
  int pointers = -1;
  int wheelDelta = 0;
  bool wheelDeltaChanged{false};

  std::map<InputKey, InputFlags> states;

public:
  InputOptions options{};
  KeyInputListener *keyInputListener = nullptr;
  void update(const float dt);
  void handleInputEvent(const InputEvent &event);
  void handleTouchMove(const TouchData &data);
  void handleTouchDown(const TouchData &data);
  void handleTouchUp(const TouchData &data);
  void handleKeyDown(const InputKey &key);
  void handleKeyUp(const InputKey &key);
  void handleKeyTyped(const InputKey &key, wchar_t c);
  void handleWheel(const int wheelDelta) {
    this->wheelDelta = wheelDelta;
    wheelDeltaChanged = true;
  }

  int getTouchCount() { return touchCount; }
  void getTouchState(TouchState &touchState, int pointer);

  int getWheelDelta() { return wheelDelta; }

  const int getInputStateCount() { return keyCount; }

  InputFlags getState(const InputKey &key) { return states[key]; }

  bool hasInputState(const InputKey &key, const InputKeyState state) {
    return (states[key] & state) == state;
  }

  bool hasInputStates(const InputKey &key, const InputFlags f) {
    return (states[key] & f) == f;
  }
};
extern Input input;
} // namespace dank
