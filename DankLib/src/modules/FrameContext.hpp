#pragma once

#include "modules/Foundation.hpp"
#include "modules/renderer/meshes/Mesh.hpp"
#include "modules/renderer/textures/Texture.hpp"

namespace dank {
struct FrameContext {
  bool paused{false};
  float deltaTime = 0;
  uint32_t framesPerSecond = 0;
  float absoluteTime = 0;
  uint32_t absoluteFrame = 0;
  entt::registry draw{};
  mesh::MeshLibrary meshLibrary{};
  texture::TextureLibrary textureLibrary{};
};
} // namespace dank
