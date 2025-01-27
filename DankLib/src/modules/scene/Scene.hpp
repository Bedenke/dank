#pragma once

#include "modules/Foundation.hpp"
#include "modules/FrameContext.hpp"
#include "modules/scene/Camera.hpp"

namespace dank {
class Scene {
  private:
  bool initialized = false;
  void init(FrameContext &ctx);
public:
  Camera camera{};
  entt::registry entities{};
  void update(FrameContext &ctx);
};
} // namespace dank
