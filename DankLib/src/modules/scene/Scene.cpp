#include "Scene.hpp"
#include "modules/renderer/Renderer.hpp"
#include "modules/renderer/meshes/Mesh.hpp"
#include "modules/renderer/meshes/RectangleMesh.hpp"
#include "modules/renderer/meshes/TriangleMesh.hpp"
#include "modules/renderer/textures/DebugTexture.hpp"

using namespace dank;

void Scene::update(FrameContext &ctx) {
  camera.mode = ProjectionMode::Perspective;
  camera.pos = glm::vec3(0.0f, 0.0f, -10.0f);
  //camera.scale = 200.0f;
  camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
  camera.update(ctx);

  ctx.draw.clear();

  glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

  auto entity1 = ctx.draw.create();
  ctx.draw.emplace<draw::Mesh>(
      entity1, draw::Mesh{model, glm::vec4(1, 0, 0, 1),
                          mesh::Triangle::ID, texture::DebugTexture::ID});

  auto entity2 = ctx.draw.create();
  ctx.draw.emplace<draw::Mesh>(
      entity2, draw::Mesh{glm::mat4(1.0f), glm::vec4(0, 1, 0, 1),
                          mesh::Rectangle::ID, texture::DebugTexture::ID});
}
