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

class Mesh {
public:
  virtual void getData(MeshData &output) = 0;
  virtual ~Mesh() = default;
};

struct MeshDescriptor {
  Mesh *mesh = nullptr;
  uint32_t vertexOffset = 0;
  uint32_t vertexCount = 0;
  uint32_t indexOffset = 0;
  uint32_t indexCount = 0;
};

struct MeshLibraryData {
  std::vector<VertexData> vbo{};
  std::vector<uint32_t> ibo{};

  uint32_t vertexDataSize = 0;
  uint32_t indexDataSize = 0;
};

class MeshLibrary {
private:
  std::map<uint32_t, MeshDescriptor> descriptors{};
  
public:
  size_t lastModified = 0;

  ~MeshLibrary() {
    for (auto &entry : descriptors) {
      delete entry.second.mesh;
    }
  }

  const MeshDescriptor *get(const uint32_t id) const {
    return &descriptors.at(id);
  };

  void getData(MeshLibraryData &output) {
    for (auto &entry : descriptors) {
      MeshDescriptor* descriptor = &entry.second;

      MeshData md{};
      descriptor->mesh->getData(md);

      descriptor->vertexOffset = output.vbo.size();
      descriptor->vertexCount = md.vertices.size();
      descriptor->indexOffset = output.ibo.size();
      descriptor->indexCount = md.indices.size();

      // offset the indices based on the previous number of vertices
      // for example:
      // add triangle mesh (3 vertices), indices 0, 1, 2
      // add rectangle mesh, indices offset by 3 (vertices) 3,4,5,5,6,3
      size_t vertexOffset = output.vbo.size();
      for (const auto index : md.indices) {
        output.ibo.push_back(vertexOffset + index);
      }
      output.vbo.insert(output.vbo.end(), md.vertices.begin(), md.vertices.end());
    }

    output.vertexDataSize = output.vbo.size() * sizeof(VertexData);
    output.indexDataSize = output.ibo.size() * sizeof(uint32_t);
  }

  void add(uint32_t id, Mesh *mesh) {
    MeshDescriptor descriptor{};
    descriptor.mesh = mesh;
    descriptors[id] = descriptor;
    lastModified++;
  }
};

} // namespace mesh
} // namespace dank
