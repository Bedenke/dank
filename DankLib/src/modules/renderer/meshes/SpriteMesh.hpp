#pragma once
#include "modules/Foundation.hpp"
#include "modules/renderer/meshes/Mesh.hpp"
#include "modules/renderer/textures/Texture.hpp"

namespace dank {

namespace mesh {

struct TextureSize {
  uint32_t width;
  uint32_t height;
};

struct TextureRegion {
  float x;
  float y;
  float width;
  float height;
  float scale = 1.0f;
};

class Sprite : public Mesh {
private:
  TextureSize textureSize;
  TextureRegion region;

public:
  Sprite(TextureSize textureSize, TextureRegion region)
      : Mesh(), textureSize(textureSize), region(region) {}

  void getData(MeshData &output) override {
    float x0 = region.x / textureSize.width;
    float y0 = region.y / textureSize.height;
    float x1 = x0 + (region.width / textureSize.width);
    float y1 = y0 + (region.height / textureSize.height);
    float nx = region.width * region.scale / 2.0f;
    float ny = region.height * region.scale / 2.0f;

    output.vertices.push_back(VertexData{
        {-nx, -ny, 0.0f},   // vertices
        {0.0f, 0.0f, 1.0f}, // normals
        {x0, y1}            // uv
    });
    output.vertices.push_back(VertexData{
        {nx, -ny, 0.0f},    // vertices
        {0.0f, 0.0f, 1.0f}, // normals
        {x1, y1}            // uv
    });
    output.vertices.push_back(VertexData{
        {nx, ny, 0.0f},     // vertices
        {0.0f, 0.0f, 1.0f}, // normals
        {x1, y0}            // uv
    });
    output.vertices.push_back(VertexData{
        {-nx, ny, 0.0f},    // vertices
        {0.0f, 0.0f, 1.0f}, // normals
        {x0, y0}            // uv
    });

    // first triangle
    output.indices.emplace_back(0);
    output.indices.emplace_back(1);
    output.indices.emplace_back(2);

    // second triangle
    output.indices.emplace_back(2);
    output.indices.emplace_back(3);
    output.indices.emplace_back(0);
  }
};

} // namespace mesh

} // namespace dank
