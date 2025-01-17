#pragma once
#include "libs/glm/fwd.hpp"
#include "modules/Foundation.hpp"
#include <cstdint>
#include <map>
#include <vector>

namespace dank {
namespace mesh {

struct VertexData {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 uv;
};

struct MeshData {
  std::vector<VertexData> vertices;
  std::vector<uint32_t> indices;
};

struct MeshDescriptor {
  uint32_t id{};
  uint32_t vertexOffset = 0;
  uint32_t vertexCount = 0;
  uint32_t indexOffset = 0;
  uint32_t indexCount = 0;
};

class Mesh {
public:
  static const uint32_t TRIANGLE = 1;

  virtual uint32_t id() = 0;
  virtual void getData(MeshData &output) = 0;
};

class MeshLibrary {

public:
  std::map<uint32_t, MeshDescriptor> descriptors{};
  std::vector<VertexData> vbo{};
  std::vector<uint32_t> ibo{};

  size_t lastModified = 0;
  uint32_t vertexDataSize = 0;
  uint32_t indexDataSize = 0;

  const MeshDescriptor *get(const uint32_t id) const {
    return &descriptors.at(id);
  };

  void add(Mesh &mesh) {
    MeshData data{};
    mesh.getData(data);

    MeshDescriptor descriptor{};
    descriptor.id = descriptors.size();
    descriptor.vertexOffset = vbo.size();
    descriptor.vertexCount = data.vertices.size();
    descriptor.indexOffset = ibo.size();
    descriptor.indexCount = data.indices.size();
    descriptors[mesh.id()] = descriptor;

    vbo.insert(vbo.end(), data.vertices.begin(), data.vertices.end());
    ibo.insert(ibo.end(), data.indices.begin(), data.indices.end());

    vertexDataSize = vbo.size() * sizeof(VertexData);
    indexDataSize = ibo.size() * sizeof(uint32_t);

    lastModified++;
  }
};

} // namespace mesh
} // namespace dank
