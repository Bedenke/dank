#pragma once

#include "libs/stb/stb_image.h"
#include "modules/Foundation.hpp"
#include "modules/engine/Console.hpp"
#include "modules/os/OS.hpp"
#include "modules/os/Thread.h"
#include "modules/renderer/textures/Texture.hpp"

namespace dank {
namespace texture {

class Texture2D : public Texture, Runnable {
private:
  TextureData cache;
  Thread *thread = nullptr;

public:
  URI uri;
  Texture2D(URI uri) : uri(uri) { cache.state = ResourceState::Idle; }

  TextureType getType() override { return TextureType::Color; }

  void fetchData(TextureData &output) override {
    if (cache.state == ResourceState::Idle) {
      cache.state = ResourceState::Loading;
      thread = new Thread();
      thread->start(this);
    }
    output = cache;
  }

  void releaseData(TextureData &output) override {
    if (cache.data != nullptr) {
      free(cache.data);
      cache.data = nullptr;
    }
  }
  
  void run() override {
    dank::console::log("[URITextureSource] run()");
    ResourceData resourceData;
    dank::os->getDataFromURI(uri, resourceData);

    if (resourceData.size <= 0) {
      dank::console::log("[URITextureSource] ResourceState::Invalid");
      cache.state = ResourceState::Invalid;
      return;
    }

    int width, height, source_channels;
    cache.data = stbi_load_from_memory(
        (stbi_uc *)(resourceData.data), static_cast<int>(resourceData.size),
        &width, &height, &source_channels, STBI_rgb_alpha);
    free(resourceData.data);

    cache.width = width;
    cache.height = height;
    cache.channels = 4; // RGBA / STBI_rgb_alpha
    cache.format = PixelFormat::RGBA8Unorm;
    cache.state = ResourceState::Ready;
    cache.lastModified++;

    dank::console::log("[URITextureSource] ResourceState::Ready");

    if (thread != nullptr) {
      delete thread;
      thread = nullptr;
    }
  }
};
} // namespace texture
} // namespace dank