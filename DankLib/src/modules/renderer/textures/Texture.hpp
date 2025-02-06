#pragma once
#include "modules/Foundation.hpp"
#include <cstdint>

namespace dank {
namespace texture {

struct TextureData {
  ResourceState state{ResourceState::Idle};
  uint32_t lastModified;

  uint32_t width;
  uint32_t height;
  uint32_t channels;
  uint8_t *data = nullptr;
  PixelFormat format{PixelFormat::RGBA8Unorm};

  TextureData &operator=(const TextureData &other) {
    if (this == &other)
      return *this;
    state = other.state;
    lastModified = other.lastModified;
    width = other.width;
    height = other.height;
    channels = other.channels;
    format = other.format;
    data = other.data;
    return *this;
  }
};

enum class TextureType { Color };

class Texture {
public:
  virtual TextureType getType() = 0;
  virtual void fetchData(TextureData &output) = 0;
  virtual void releaseData(TextureData &output) {};
  virtual ~Texture() = default;
};

class TextureLibrary {
private:
  uint32_t nextId = 1;

public:
  std::map<uint32_t, Texture *> textures{};

  ~TextureLibrary() { clear(); }

  void clear() {
    for (auto &entry : textures) {
      delete entry.second;
    }
    nextId = 1;
    textures.clear();
  }

  uint32_t add(Texture *texture) {
    nextId++;
    textures[nextId - 1] = texture;
    return nextId - 1;
  }

  const Texture *get(const uint32_t id) const { return textures.at(id); };
};

} // namespace texture

} // namespace dank