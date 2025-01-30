#include "modules/input/Input.hpp"
#include <cmath>

using namespace dank;

bool Touch::update(const float dt, const InputOptions &input) {
  actions = TouchActions::TA_NONE;
  if (touching) {
    actions = actions | TouchActions::TA_TOUCH;
    if (detectedTouched) {
      actions = actions | TouchActions::TA_TOUCHED;
    }
    if (detectedMove) {
      actions = actions | TouchActions::TA_DRAG;
      detectedDrag = true;
      // Cancel long press detection for 1000000 seconds
      // So it won't be detected after dragging
      longPressTime = -1000000;
    } else {
      longPressTime += dt;
      if (longPressTime > input.longPressThreshold) {
        // Once detected, only clear long press on touch up
        detectedLongPress = true;
      }
    }
    if (detectedLongPress) {
      actions = actions | TouchActions::TA_LONG_PRESS;
    }
  } else {
    if (detectedLongPress) {
      actions = actions | TouchActions::TA_LONG_PRESS;
    }
    if (detectedDrag) {
      actions = actions | TouchActions::TA_DROP;
    } else if (detectedMove) {
      actions = actions | TouchActions::TA_HOVER;
    }
    if (detectedDoubleTap) {
      actions = actions | TouchActions::TA_DOUBLE_TAP;
      lastTapTime = 0;
    }
    if (detectedRelease) {
      if (actions == 0) {
        actions = actions | TouchActions::TA_TAP;
      }
      actions = actions | TouchActions::TA_RELEASE;
    }
    // Clear long press
    longPressTime = 0;
    detectedLongPress = false;
    detectedDrag = false;
  }
  // Always clear move and tap
  detectedTouched = false;
  detectedMove = false;
  detectedRelease = false;
  detectedDoubleTap = false;
  return touching ||
         (actions & TouchActions::TA_RELEASE | TouchActions::TA_HOVER) != 0;
}

void Touch::handleTouchDown(const TouchData &data, const InputOptions &input) {
  x = data.x;
  y = data.y;
  button = data.button;
  if (!touching)
    detectedTouched = true;
  touching = true;
  startX = x;
  startY = y;
  lastX = x;
  lastY = y;
}

void Touch::handleTouchMove(const TouchData &data, const InputOptions &input) {
  x = data.x;
  y = data.y;

  // if (centerLock)
  // {
  //     velX = (x - input.centerX);
  //     velY = (y - input.centerY);
  //     startX = (float)input.centerX;
  //     startY = (float)input.centerY;
  //     lastX = x;
  //     lastY = y;
  //     if (!detectedMove)
  //     {
  //         detectedMove = abs(velX) > 0 || abs(velY) > 0;
  //     }
  // }
  // else
  {
    float dx = fabsf(startX - x);
    float dy = fabsf(startY - y);
    if (!detectedMove) {
      velX = x - lastX;
      velY = y - lastY;
      lastX = x;
      lastY = y;
      detectedMove = dx > input.dragThreshold || dy > input.dragThreshold;
    }
  }
}

void Touch::handleTouchUp(const TouchData &data, const InputOptions &input) {
  x = data.x;
  y = data.y;
  button = data.button;
  touching = false;
  detectedRelease = true;
  double delta = input.absoluteTime - lastTapTime;
  if (delta < input.doubleTapThreshold)
    detectedDoubleTap = true;
  lastTapTime = input.absoluteTime;
}

void Input::update(const float dt) {
  options.absoluteTime += dt;
  touchCount = 0;
  for (int i = 0; i <= pointers; i++) {
    if (touches[i].update(dt, options))
      touchCount++;
  }
  if (touchCount == 0)
    pointers = -1;

  keyCount = 0;
  for (auto &entry : states) {
    auto &state = entry.second;
    if ((state & InputKeyState::IKS_TRIGGERING) ==
        InputKeyState::IKS_TRIGGERING) {
      state = (state & InputKeyState::IKS_DOWN) | InputKeyState::IKS_TRIGGERED;
    } else if ((state & InputKeyState::IKS_TRIGGERED) ==
               InputKeyState::IKS_TRIGGERED) {
      state = state & InputKeyState::IKS_DOWN;
    }
    if (state != InputKeyState::IKS_UP)
      keyCount++;
  }

  if (wheelDeltaChanged)
    wheelDeltaChanged = false;
  else
    wheelDelta = 0;
}

void Input::handleTouchDown(const TouchData &data) {
  if (data.button < MAX_INPUT_TOUCHES) {
    touches[data.button].handleTouchDown(data, options);

    if (data.button > pointers)
      pointers = data.button;
  }
}
void Input::handleTouchMove(const TouchData &data) {
  if (data.button < MAX_INPUT_TOUCHES) {
    touches[data.button].handleTouchMove(data, options);
    if (data.button > pointers)
      pointers = data.button;
  }
}

void Input::handleTouchUp(const TouchData &data) {
  if (data.button < MAX_INPUT_TOUCHES) {
    touches[data.button].handleTouchUp(data, options);
    if (data.button > pointers)
      pointers = data.button;
  }
}

void Input::handleKeyDown(const InputKey &key) {
  states[key] = InputKeyState::IKS_TRIGGERING | InputKeyState::IKS_DOWN;
  if (keyInputListener != nullptr)
    keyInputListener->onKeyDown(key);
}

void Input::handleKeyUp(const InputKey &key) {
  states[key] = InputKeyState::IKS_TRIGGERING;
  if (keyInputListener != nullptr)
    keyInputListener->onKeyUp(key);
}

void Input::handleKeyTyped(const InputKey &key, wchar_t c) {
  if (keyInputListener != nullptr && !std::iscntrl(c))
    keyInputListener->onChar(c);
}

void Input::getTouchState(TouchState &touchState, int pointer) {
  const auto touch = touches[pointer];
  touchState.x = touch.x;
  touchState.y = touch.y;
  touchState.dx = touch.x - touch.startX;
  touchState.dy = touch.y - touch.startY;
  touchState.vx = touch.velX;
  touchState.vy = touch.velY;
  touchState.actions = touch.actions;
  touchState.button = touch.button;
}
