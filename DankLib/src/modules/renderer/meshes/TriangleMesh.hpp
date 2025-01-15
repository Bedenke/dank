#pragma once
#include "modules/Foundation.hpp"
#include "modules/renderer/meshes/Mesh.hpp"

namespace dank {

namespace mesh {

class Triangle : public Mesh {
public:
  uint32_t id() override { return Mesh::TRIANGLE; }
  void getData(MeshData &output) override {
    output.vertices.push_back(VertexData{// vertices
                                         {-1.0f, -1.0f, 0.0f},
                                         // normals
                                         {1.0f, 0.0f, 0.0f},
                                         // uv
                                         {0.0f, 0.0f}});
    output.vertices.push_back(VertexData{// vertices
                                         {0.0f, 1.0f, 0.0f},
                                         // normals
                                         {0.0f, 1.0f, 0.0f},
                                         // uv
                                         {0.0f, 1.0f}});
    output.vertices.push_back(VertexData{// vertices
                                         {1.0f, -1.0f, 0.0f},
                                         // normals
                                         {0.0f, 0.0f, 1.0f},
                                         // uv
                                         {0.0f, 1.0f}});
    output.indices.push_back(0);
    output.indices.push_back(1);
    output.indices.push_back(2);
  }
};

} // namespace mesh

} // namespace dank
