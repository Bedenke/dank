#include "Scene.hpp"
#include "modules/engine/Console.hpp"
#include "modules/renderer/Renderer.hpp"
#include "modules/renderer/meshes/Mesh.hpp"
#include "modules/renderer/meshes/RectangleMesh.hpp"
#include "modules/renderer/meshes/SpriteMesh.hpp"
#include "modules/renderer/meshes/TriangleMesh.hpp"
#include "modules/renderer/textures/DebugTexture.hpp"
#include "modules/renderer/textures/Texture.hpp"

using namespace dank;

struct SceneIDs {
  uint32_t triangleMesh = 0;
  uint32_t rectangleMesh = 0;
  uint32_t debugTexture1 = 0;
  uint32_t debugTexture2 = 0;
  uint32_t spriteMesh1 = 0;
  uint32_t spriteMesh2 = 0;
};

SceneIDs sceneIDs;

void Scene::init(FrameContext &ctx) {
  ctx.textureLibrary.clear();
  ctx.meshLibrary.clear();

  // Add textures
  texture::Texture *texture1 = new texture::DebugTexture();
  texture::Texture *texture2 = new texture::DebugTexture();
  sceneIDs.debugTexture1 = ctx.textureLibrary.add(texture1);
  sceneIDs.debugTexture2 = ctx.textureLibrary.add(texture2);

  // Add meshes
  sceneIDs.triangleMesh = ctx.meshLibrary.add(new mesh::Triangle());
  sceneIDs.rectangleMesh = ctx.meshLibrary.add(new mesh::Rectangle());
  sceneIDs.spriteMesh1 = ctx.meshLibrary.add(
      new mesh::Sprite(texture1, mesh::TextureRegion{0, 0, 76, 100}));
  sceneIDs.spriteMesh2 = ctx.meshLibrary.add(
      new mesh::Sprite(texture1, mesh::TextureRegion{32, 0, 64, 28}));

  initialized = true;
  dank::console::log("Scene initialized");
}

void Scene::update(FrameContext &ctx) {
  if (!initialized) {
    init(ctx);
  }

  camera.mode = ProjectionMode::Perspective;
  camera.pos = glm::vec3(0.0f, 0.0f, 10.0f);
  // camera.scale = 200.0f;
  camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
  camera.update(ctx);

  ctx.draw.clear();

  glm::mat4 model =
      glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 1.0f, 0.0f));

  auto entity1 = ctx.draw.create();
  ctx.draw.emplace<draw::Mesh>(entity1, draw::Mesh{model, glm::vec4(1, 0, 0, 1),
                                                   sceneIDs.triangleMesh,
                                                   sceneIDs.debugTexture1});

  auto entity2 = ctx.draw.create();
  ctx.draw.emplace<draw::Mesh>(
      entity2, draw::Mesh{glm::mat4(1.0f), glm::vec4(0, 1, 0, 1),
                          sceneIDs.rectangleMesh, sceneIDs.debugTexture2});

  auto sprite1 = ctx.draw.create();
  ctx.draw.emplace<draw::Mesh>(
      sprite1,
      draw::Mesh{glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -1.0f, 0.0f)), {0.1f, 0.1f, 0.1f}),
                 glm::vec4(0, 0, 1, 1), sceneIDs.spriteMesh1,
                 sceneIDs.debugTexture1});

  auto sprite2 = ctx.draw.create();
  ctx.draw.emplace<draw::Mesh>(
      sprite2,
      draw::Mesh{glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.0f, 1.0f)), {0.1f, 0.1f, 0.1f}),
                 glm::vec4(1, 0, 1, 1), sceneIDs.spriteMesh2,
                 sceneIDs.debugTexture1});

}

