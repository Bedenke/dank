#pragma once

#include "modules/Foundation.hpp"
#include "modules/os/Capture.hpp"
#include "modules/renderer/textures/Texture.hpp"

namespace dank {
namespace texture {
class TextureScreenCapture : public Texture {
private:
  CaptureConfig *captureConfig;

public:
  TextureScreenCapture(CaptureConfig *captureConfig)
      : captureConfig(captureConfig) {}

  TextureType getType() override { return TextureType::Color; }

  void fetchData(TextureData &output) override {
    if (!captureConfig->captureScreen) {
      output.state = ResourceState::Idle;
      return;
    }

    output.lastModified = captureConfig->screenOutput.frame;
    output.state = ResourceState::Ready;
    output.channels = captureConfig->screenOutput.channels;
    output.width = captureConfig->screenOutput.width;
    output.height = captureConfig->screenOutput.height;
    output.data = captureConfig->screenOutput.data;
    output.format = captureConfig->screenOutput.format;
  }
};
} // namespace texture
} // namespace dank