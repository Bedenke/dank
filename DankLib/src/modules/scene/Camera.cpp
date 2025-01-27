#include "modules/scene/Camera.hpp"
#include "modules/FrameContext.hpp"
#include <cassert>

using namespace dank;

void Camera::onViewResize(float viewWidth, float viewHeight) {
  assert(viewWidth > 0);
  assert(viewHeight > 0);
  viewport = glm::vec4(0, 0, viewWidth, viewHeight);
}

const glm::vec3 Camera::screenToWorld(float screenX, float screenY,
                                      float depth) {
  const glm::vec3 coords(screenX, screenY, depth);
  return glm::unProject(coords, view, proj, viewport);
}

const glm::vec2 Camera::worldToScreen(glm::vec3 &offset, glm::mat4 &model) {
  return glm::project(offset, view * model, proj, viewport);
}

void Camera::update(FrameContext &ctx) {
  float width = viewport[2];
  float height = viewport[3];
  if (mode == ProjectionMode::Perspective) {
    float ratio = height > 0 ? width / height : 1.0f;
    proj = glm::perspective(glm::radians(fov), ratio, znear, zfar);
  } else {
    proj = glm::ortho(-width / scale, width / scale, -height / scale,
                      height / scale, znear, zfar);
    // proj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, znear, zfar);
  }
  view = glm::lookAt(pos, target, up);
  frustrum.update(proj * view);
  time += ctx.deltaTime;
}

void Camera::getCameraUBO(CameraUBO *output) {
  output->view = view;
  output->proj = proj;
  output->viewProj = proj * view;
  output->position = glm::vec4(pos, 0.0f);
  output->lightViewPos = output->position * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);
  output->nearPlane = znear;
  output->farPlane = zfar;
  output->gamma = gamma;
  output->time = time;
}
