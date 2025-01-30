#include "Scene.hpp"
#include "libs/glm/fwd.hpp"
#include "modules/engine/Console.hpp"
#include "modules/input/Input.hpp"
#include "modules/renderer/Renderer.hpp"
#include "modules/renderer/meshes/Mesh.hpp"
#include "modules/renderer/meshes/RectangleMesh.hpp"
#include "modules/renderer/meshes/SpriteMesh.hpp"
#include "modules/renderer/meshes/TriangleMesh.hpp"
#include "modules/renderer/textures/DebugTexture.hpp"
#include "modules/renderer/textures/Texture.hpp"
#include "modules/renderer/textures/Texture2D.hpp"

using namespace dank;

struct TextureIDs {
  uint32_t sprites = 0;
  uint32_t starfield = 0;
};

struct Starfield {
  uint32_t meshId = 0;
  uint32_t textureId = 0;
};

struct Spaceship {
  glm::vec3 pos;
  glm::vec3 scale{1, 1, 1};
  uint32_t meshId = 0;
  uint32_t textureId = 0;
};

struct SceneDescriptor {
  TextureIDs textures;
  Starfield starfield;
  Spaceship spaceship1;
  Spaceship spaceship2;
};

SceneDescriptor myScene;

void Scene::init(FrameContext &ctx) {
  ctx.textureLibrary.clear();
  ctx.meshLibrary.clear();

  // Add textures
  myScene.textures.sprites = ctx.textureLibrary.add(
      new texture::Texture2D(URI{"file://Demo/Sprites.png"}));
  myScene.textures.starfield = ctx.textureLibrary.add(
      new texture::Texture2D(URI{"file://Demo/Starfield.png"}));

  // Add entities
  myScene.spaceship1 = {
      {0, 0, 0},
      {1, 1, 1},
      ctx.meshLibrary.add(new mesh::Sprite(
          {1024, 1024}, mesh::TextureRegion{200, 500, 200, 200})),
      myScene.textures.sprites};

  myScene.spaceship2 = {
      {0, 0, 0},
      {1, 1, 1},
      ctx.meshLibrary.add(new mesh::Sprite(
          {1024, 1024}, mesh::TextureRegion{400, 500, 200, 200})),
      myScene.textures.sprites};

  myScene.starfield = {
      ctx.meshLibrary.add(new mesh::Sprite(
          {2048, 2048}, mesh::TextureRegion{0, 0, 2048, 2048})),
      myScene.textures.starfield};

  initialized = true;
  dank::console::log("Scene initialized");
}

void Scene::update(FrameContext &ctx) {
  if (!initialized) {
    init(ctx);
  }

  if (ctx.input.hasInputState(InputKey::KEY_UP, InputKeyState::IKS_DOWN)) {
    myScene.spaceship1.pos.y += 10;
  } else if (ctx.input.hasInputState(InputKey::KEY_DOWN,
                                     InputKeyState::IKS_DOWN)) {
    myScene.spaceship1.pos.y -= 10;
  }

  TouchState ts1, ts2;
  ctx.input.getTouchState(ts1, TouchButton::TB_LEFT);
  ctx.input.getTouchState(ts2, TouchButton::TB_RIGHT);
  if (ts1.hasAction(TouchActions::TA_TOUCH)) {
    dank::console::log("TOUCH %.1f:%.1f b=%d", ts1.x, ts1.y, ts1.button);
  }
  if (ts2.hasAction(TouchActions::TA_TOUCH)) {
    dank::console::log("TOUCH %.1f:%.1f b=%d", ts2.x, ts2.y, ts2.button);
  }
  if (ts1.hasAction(TouchActions::TA_TOUCHED)) {
    dank::console::log("TOUCHED");
  }
  if (ts1.hasAction(TouchActions::TA_DRAG)) {
    dank::console::log("TA_DRAG");
  }
  if (ts1.hasAction(TouchActions::TA_HOVER)) {
    dank::console::log("TA_HOVER");
  }

  camera.mode = ProjectionMode::Orthographic;
  camera.pos = glm::vec3(0.0f, 0.0f, 10.0f);
  // camera.scale = 200.0f;
  camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
  camera.update(ctx);

  ctx.draw.clear();

  auto spaceship1 = ctx.draw.create();
  ctx.draw.emplace<draw::Mesh>(
      spaceship1, draw::Mesh{glm::scale(glm::translate(glm::mat4(1.0f),
                                                       myScene.spaceship1.pos),
                                        myScene.spaceship1.scale),
                             glm::vec4(1, 1, 1, 1), myScene.spaceship1.meshId,
                             myScene.spaceship1.textureId});

  myScene.spaceship2.pos = glm::vec3(-100, -100, 0);
  auto spaceship2 = ctx.draw.create();
  ctx.draw.emplace<draw::Mesh>(
      spaceship2, draw::Mesh{glm::scale(glm::translate(glm::mat4(1.0f),
                                                       myScene.spaceship2.pos),
                                        myScene.spaceship2.scale),
                             glm::vec4(1, 1, 1, 1), myScene.spaceship2.meshId,
                             myScene.spaceship2.textureId});

  auto starfield = ctx.draw.create();
  ctx.draw.emplace<draw::Mesh>(
      starfield,
      draw::Mesh{glm::mat4(1.0f), glm::vec4(1, 1, 1, 1),
                 myScene.starfield.meshId, myScene.starfield.textureId});
}
