#include "Scene.hpp"
#include "modules/renderer/Renderer.hpp"
#include "modules/renderer/meshes/Mesh.hpp"
#include "modules/renderer/meshes/RectangleMesh.hpp"

using namespace dank;

void Scene::update(FrameContext &ctx) {
  ctx.draw.clear();

  auto t1 = ctx.draw.create();
  ctx.draw.emplace<draw::Mesh>(t1, draw::Mesh{mesh::Rectangle::ID});
}
