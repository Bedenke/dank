#pragma once

#include "modules/renderer/textures/Texture.hpp"
namespace dank {
namespace texture {
class DebugTexture : public Texture {
public:
  static const uint32_t ID = 1;
  TextureType getType() { return TextureType::Color; }

  void getData(TextureData &output) {

    const uint32_t tw = 128;
    const uint32_t th = 128;

    output.width = tw;
    output.height = th;
    output.channels = 4;

    output.data.resize(tw * th * output.channels);
    for (size_t y = 0; y < th; ++y) {
      for (size_t x = 0; x < tw; ++x) {
        bool isWhite = (x ^ y) & 0b1000000;
        uint8_t c = isWhite ? 0xFF : 0xA;

        size_t i = y * tw + x;

        output.data[i * output.channels + 0] = c;
        output.data[i * output.channels + 1] = c;
        output.data[i * output.channels + 2] = c;
        output.data[i * output.channels + 3] = 0xFF;
      }
    }
  }
};
} // namespace texture
} // namespace dank