#pragma once

#include "modules/input/Input.hpp"
#include "modules/input/InputEvent.hpp"
#include <map>
#include <string>
#include <vector>

namespace dank {

enum class ControllerActionOn { Press, Hold, Release };

struct ControllerActionBinding {
  ControllerActionOn on;
  std::vector<InputKey> keys;
};

typedef std::map<std::string, ControllerActionBinding> ControllerDescriptor;

class ControllerAction {
private:
  ControllerActionOn on;
  std::vector<InputKey> keys;

public:
  ControllerAction(const ControllerActionOn on,
                   const std::vector<InputKey> keys)
      : on(on), keys(keys) {}

  const bool isTriggered() {
    InputKeyState expectedState;
    switch (on) {
    case ControllerActionOn::Press:
      expectedState = IKS_TRIGGERING;
      break;
    case ControllerActionOn::Release:
      expectedState = IKS_UP;
      break;
    case ControllerActionOn::Hold:
      expectedState = IKS_DOWN;
      break;
    };
    for (auto key : keys) {
      if (input.hasInputState(key, expectedState))
        return true;
    }
    return false;
  }
};

} // namespace dank