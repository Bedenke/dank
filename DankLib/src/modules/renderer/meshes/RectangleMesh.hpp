#pragma once
#include "modules/Foundation.hpp"
#include "modules/renderer/meshes/Mesh.hpp"

namespace dank {

namespace mesh {

class Rectangle : public Mesh {
public:
  uint32_t id() override { return Mesh::RECTANGLE; }
  void getData(MeshData &output) override {
    output.vertices.push_back(VertexData{
        {-1.0f, -1.0f, 0.0f}, // vertices
        {0.0f, 0.0f, 1.0f},   // normals
        {0.0f, 0.0f}          // uv
    });
    output.vertices.push_back(VertexData{
        {1.0f, -1.0f, 0.0f}, // vertices
        {0.0f, 0.0f, 1.0f}, // normals
        {1.0f, 0.0f} // uv
    });
    output.vertices.push_back(VertexData{
        {1.0f, 1.0f, 0.0f}, // vertices
        {0.0f, 0.0f, 1.0f}, // normals
        {1.0f, 1.0f} // uv
    });
    output.vertices.push_back(VertexData{
        {-1.0f, 1.0f, 0.0f}, // vertices
        {0.0f, 0.0f, 1.0f}, // normals
        {0.0f, 1.0f} // uv
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
