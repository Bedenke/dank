#pragma once
#include "modules/Foundation.hpp"
#include <cstdint>

namespace dank {
namespace texture {

struct TextureData {
  std::vector<uint8_t> data{};
  size_t size;
  uint32_t channels;
  uint32_t width;
  uint32_t height;
};

enum TextureType { Color };

class Texture {
public:
  virtual TextureType getType() = 0;
  virtual void getData(TextureData &output) = 0;
  virtual ~Texture() = default;
};

struct TextureDescriptor {
  Texture *texture;
  uint32_t index;
};

class TextureLibrary {
private:
public:
  std::map<uint32_t, TextureDescriptor> descriptors{};
  size_t lastModified = 0;

  ~TextureLibrary() {
    for (auto &entry : descriptors) {
      delete entry.second.texture;
    }
  }

  void add(uint32_t id, Texture *texture) {
    TextureDescriptor descriptor{};
    descriptor.texture = texture;
    descriptor.index = descriptors.size();
    descriptors[id] = descriptor;

    lastModified++;
  }

  const TextureDescriptor *get(const uint32_t id) const {
    return &descriptors.at(id);
  };
};

} // namespace texture

} // namespace dank