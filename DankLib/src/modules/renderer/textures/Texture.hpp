#pragma once
#include "modules/Foundation.hpp"
#include <cstdint>

namespace dank {
namespace texture {

struct TextureData {
  uint32_t width;
  uint32_t height;
  uint32_t channels;
  std::vector<uint8_t> data{};
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
  uint32_t nextId = 1;

public:
  std::map<uint32_t, TextureDescriptor> descriptors{};
  size_t lastModified = 0;

  ~TextureLibrary() {
    clear();
  }

  void clear() {
    for (auto &entry : descriptors) {
      delete entry.second.texture;
    }
    nextId = 1;
    descriptors.clear();
  }

  uint32_t add(Texture *texture) {
    TextureDescriptor descriptor{};
    descriptor.texture = texture;
    descriptor.index = descriptors.size();
    descriptors[nextId] = descriptor;
    nextId++;
    lastModified++;
    return nextId - 1;
  }

  const TextureDescriptor *get(const uint32_t id) const {
    return &descriptors.at(id);
  };
};

} // namespace texture

} // namespace dank