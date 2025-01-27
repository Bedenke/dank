#include "Scene.hpp"
#include "modules/renderer/Renderer.hpp"
#include "modules/renderer/meshes/Mesh.hpp"
#include "modules/renderer/meshes/RectangleMesh.hpp"
#include "modules/renderer/meshes/TriangleMesh.hpp"

using namespace dank;

void Scene::update(FrameContext &ctx) {
  camera.mode = ProjectionMode::Orthographic;
  camera.pos = glm::vec3(0.0f, 0.0f, -1.0f);
  camera.scale = 200.0f;
  camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
  camera.update(ctx);

  ctx.draw.clear();

  auto entity1 = ctx.draw.create();
  ctx.draw.emplace<draw::Mesh>(entity1, draw::Mesh{mesh::Triangle::ID});

  auto entity2 = ctx.draw.create();
  ctx.draw.emplace<draw::Mesh>(entity2, draw::Mesh{mesh::Rectangle::ID});
}
