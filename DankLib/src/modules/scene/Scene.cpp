#include "Scene.hpp"
#include "libs/glm/fwd.hpp"
#include "modules/engine/Console.hpp"
#include "modules/input/Controller.hpp"
#include "modules/input/Input.hpp"
#include "modules/input/InputEvent.hpp"
#include "modules/os/Capture.hpp"
#include "modules/renderer/Renderer.hpp"
#include "modules/renderer/meshes/Mesh.hpp"
#include "modules/renderer/meshes/RectangleMesh.hpp"
#include "modules/renderer/meshes/SpriteMesh.hpp"
#include "modules/renderer/meshes/TriangleMesh.hpp"
#include "modules/renderer/textures/DebugTexture.hpp"
#include "modules/renderer/textures/Texture.hpp"
#include "modules/renderer/textures/Texture2D.hpp"
#include "modules/renderer/textures/TextureScreenCapture.hpp"
#include <cstdint>

using namespace dank;

struct TextureIDs {
  uint32_t sprites;
  uint32_t starfield;
  uint32_t screen;
};

struct Starfield {
  uint32_t meshId;
  uint32_t textureId;
};

struct Spaceship {
  glm::vec3 pos;
  glm::vec3 scale{1, 1, 1};
  uint32_t meshId;
  uint32_t textureId;
};

struct ScreenView {
  bool initialized{false};
  uint32_t meshId;
  uint32_t textureId;
};

struct PlayerController {
  ControllerAction forward{ControllerActionOn::Hold,
                           {InputKey::KEY_W, InputKey::KEY_UP}};
  ControllerAction backward{ControllerActionOn::Hold,
                            {InputKey::KEY_S, InputKey::KEY_DOWN}};
};

struct SceneDescriptor {
  PlayerController playerController;
  TextureIDs textures;
  ScreenView screenView;
  Starfield starfield;
  Spaceship spaceship1;
  Spaceship spaceship2;
  uint32_t lastCaptureFrame = 0;
  uint32_t lastCaptureMicFrame = 0;
};

SceneDescriptor myScene;
CaptureConfig capture;
CaptureSharableContent sharableContent;

// Define the number of samples to analyze (e.g., 2205 samples for 50 ms at 44.1
// kHz)
const int samplesToAnalyze = 2205;

void Scene::init(FrameContext &ctx) {
  ctx.textureLibrary.clear();
  ctx.meshLibrary.clear();

  // Add textures
  myScene.textures.sprites = ctx.textureLibrary.add(
      new texture::Texture2D(URI{"file://Demo/Sprites.png"}));
  myScene.textures.starfield = ctx.textureLibrary.add(
      new texture::Texture2D(URI{"file://Demo/Starfield.png"}));
  myScene.textures.screen =
      ctx.textureLibrary.add(new texture::TextureScreenCapture(&capture));

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

  if (myScene.playerController.forward.isTriggered()) {
    myScene.spaceship1.pos.y += 10;
  }

  if (myScene.playerController.backward.isTriggered()) {
    myScene.spaceship1.pos.y -= 10;
  }

  TouchState ts1, ts2;
  dank::input.getTouchState(ts1, TouchButton::TB_LEFT);
  if (ts1.hasAction(TouchActions::TA_TOUCH)) {
    dank::console::log("TOUCH %.1f:%.1f b=%d", ts1.x, ts1.y, ts1.button);
  }
  if (ts1.hasAction(TouchActions::TA_TOUCHED)) {
    if (sharableContent.displayCount > 0) {
      capture.selectedDisplay = 0;
      capture.captureScreen = !capture.captureScreen;
      capture.captureMicrophone = capture.captureScreen;
      dank::os->setCaptureConfig(capture);
    } else {
      dank::os->getCaptureSharableContent(sharableContent);
    }
  }

  if (myScene.lastCaptureFrame != capture.screenOutput.frame) {
    myScene.lastCaptureFrame = capture.screenOutput.frame;

    if (!myScene.screenView.initialized) {
      myScene.screenView = {
          true,
          ctx.meshLibrary.add(new mesh::Sprite(
              {capture.screenOutput.width, capture.screenOutput.height},
              mesh::TextureRegion{0, 0, (float)capture.screenOutput.width,
                                  (float)capture.screenOutput.height})),
          myScene.textures.screen};
    }

    // dank::console::log("FRAME %d %d %d", capture.screenOutput.frame,
    //                    capture.screenOutput.width,
    //                    capture.screenOutput.height);
  }

  if (myScene.lastCaptureMicFrame != capture.micOutput.frame) {
    myScene.lastCaptureMicFrame = capture.micOutput.frame;

    float count = 0;
    float sum = 0;
    for (auto &buffer : capture.micOutput.buffers) {
      for (auto &audioBuffer : buffer.audioBuffers) {
        for (int i = 0; i < audioBuffer.numSamples; i++) {
          float signal = audioBuffer.get(i);
          sum += signal * signal;
        }
        count++;
      }
      buffer.consumed = true;
    }

    if (count > 0) {
      // float rms_dB_FS = 20 * log10(sqrt(sum / count) * sqrt(2.0));
      float avg = sum / (float)count;
      float rms_dB = 10 * log10(avg);

      myScene.spaceship1.pos.y = rms_dB * 10;
      // dank::console::log("Audio Buffer Size: %d | frame=%d | rms_dB=%.2f |
      // avg=%.2f",
      //                    capture.micOutput.buffers.size(),
      //                    capture.micOutput.frame, rms_dB, avg);
    }
  }

  if (ts1.hasAction(TouchActions::TA_DRAG)) {
    // dank::console::log("TA_DRAG");
  }
  if (ts1.hasAction(TouchActions::TA_HOVER)) {
    // dank::console::log("TA_HOVER");
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

  if (capture.captureScreen && myScene.screenView.initialized) {
    auto screenView = ctx.draw.create();
    ctx.draw.emplace<draw::Mesh>(
        screenView,
        draw::Mesh{glm::mat4(1.0f), glm::vec4(1, 1, 1, 1),
                   myScene.screenView.meshId, myScene.screenView.textureId});
  } else {
    auto starfield = ctx.draw.create();
    ctx.draw.emplace<draw::Mesh>(
        starfield,
        draw::Mesh{glm::mat4(1.0f), glm::vec4(1, 1, 1, 1),
                   myScene.starfield.meshId, myScene.starfield.textureId});
  }
}
