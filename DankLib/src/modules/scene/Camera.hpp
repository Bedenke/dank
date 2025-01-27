#include "modules/Foundation.hpp"
#include "modules/FrameContext.hpp"
#include "modules/scene/Frustrum.h"

namespace dank {

struct alignas(16) CameraUBO {
  glm::mat4 viewProj;
  glm::mat4 view;
  glm::mat4 proj;
  glm::vec4 position;
  glm::vec4 lightViewPos;
  float nearPlane;
  float farPlane;
  float gamma;
  float time;
};

enum ProjectionMode : uint32_t { Perspective = 0, Orthographic = 1 };

class Camera {
public:
  float fov = 70;
  float znear = 0.1f;
  float zfar = 100.0f;
  float scale = 1.0f;
  float gamma = 1.0f;
  float time = 0;

  ProjectionMode mode = ProjectionMode::Orthographic;

  glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::vec3 pos;
  glm::vec3 target;
  glm::vec4 viewport;
  glm::mat4 view;
  glm::mat4 proj;
  Frustum frustrum{};

  void onViewResize(float viewWidth, float viewHeight);

  const glm::vec3 screenToWorld(float screenX, float screenY, float depth);
  const glm::vec2 worldToScreen(glm::vec3 &offset, glm::mat4 &model);

  void update(FrameContext &ctx);
  void getCameraUBO(CameraUBO *output);
};

} // namespace dank