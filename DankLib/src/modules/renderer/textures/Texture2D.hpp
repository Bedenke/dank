#pragma once

#include "libs/stb/stb_image.h"
#include "modules/os/OS.hpp"
#include "modules/renderer/textures/Texture.hpp"

namespace dank {
namespace texture {
class Texture2D : public Texture {
private:
  URI uri;

public:
  Texture2D(URI uri) :uri(uri) {}

  TextureType getType() { return TextureType::Color; }

  void getData(TextureData &output) {
    ResourceData resourceData;
    dank::os->getDataFromURI(uri, resourceData);

    assert(resourceData.size > 0);

    output.channels = 4; // RGBA / STBI_rgb_alpha

    int width, height, source_channels;
    const stbi_uc *buffer = stbi_load_from_memory(
        (stbi_uc *)(resourceData.data), static_cast<int>(resourceData.size),
        &width, &height, &source_channels, STBI_rgb_alpha);

    output.data.resize(width * height * output.channels);
    output.data.assign(buffer, buffer + width * height * output.channels);

    output.width = width;
    output.height = height;

    free(resourceData.data);
  }
};
} // namespace texture
} // namespace dank