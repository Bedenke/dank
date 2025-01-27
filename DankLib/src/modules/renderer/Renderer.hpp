#pragma once
#include "modules/FrameContext.hpp"
#include "modules/scene/Scene.hpp"

namespace dank {
class Renderer {
public:
  virtual void render(FrameContext &ctx, Scene *scene) = 0;
};

namespace draw {
struct Mesh {
  glm::mat4 transform;
  glm::vec4 color;
  uint32_t meshId{0};
  uint32_t textureId{0};
  glm::vec2 _pad;
};

} // namespace draw

} // namespace dank
