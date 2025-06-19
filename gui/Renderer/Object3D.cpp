#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

#include "Object3D.hpp"

Object3D::Object3D(const std::string &path)
{
  std::ifstream file(path);
  if (!file.is_open())
    throw std::runtime_error("Failed to open OBJ file: " + path);

  std::vector<glm::vec3> positions;
  std::vector<glm::vec2> texCoords;
  std::vector<glm::vec3> normals;

  std::string currentName = "default";
  std::unique_ptr<std::vector<Vertex>> meshVertices = std::
    make_unique<std::vector<Vertex>>();
  std::unique_ptr<std::vector<unsigned int>> meshIndices = std::
    make_unique<std::vector<unsigned int>>();

  std::unordered_map<std::string, unsigned int> uniqueVertexMap;
  std::string line;
  unsigned int index = 0;

  while (std::getline(file, line)) {
    std::istringstream ss(line);
    std::string prefix;
    ss >> prefix;

    if (prefix == "o" || prefix == "g") {
      if (!meshIndices->empty()) {
        std::unique_ptr<Mesh> m = std::make_unique<Mesh>(
          currentName, std::move(meshVertices), std::move(meshIndices));
        _meshArr.push_back(std::move(m));
        meshVertices = std::make_unique<std::vector<Vertex>>();
        meshIndices = std::make_unique<std::vector<unsigned int>>();
        uniqueVertexMap.clear();
        index = 0;
      }
      ss >> currentName;
    } else if (prefix == "v") {
      glm::vec3 pos;
      ss >> pos.x >> pos.y >> pos.z;
      positions.push_back(pos);
    } else if (prefix == "vt") {
      glm::vec2 uv;
      ss >> uv.x >> uv.y;
      uv.y = 1.0 - uv.y;  // Flip Y for OpenGL
      texCoords.push_back(uv);
    } else if (prefix == "vn") {
      glm::vec3 norm;
      ss >> norm.x >> norm.y >> norm.z;
      normals.push_back(norm);
    } else if (prefix == "f") {
      std::string vertexStr;
      std::vector<unsigned int> faceIndices;

      while (ss >> vertexStr) {
        if (uniqueVertexMap.count(vertexStr) == 0) {
          std::istringstream vss(vertexStr);
          std::string v;
          std::string t;
          std::string n;

          std::getline(vss, v, '/');
          std::getline(vss, t, '/');
          std::getline(vss, n, '/');

          Vertex vert = {};
          vert.position = positions[std::stoi(v) - 1];

          if (!t.empty())
            vert.texCoord = texCoords[std::stoi(t) - 1];
          if (!n.empty())
            vert.normal = normals[std::stoi(n) - 1];

          meshVertices->push_back(vert);
          uniqueVertexMap[vertexStr] = index++;
        }
        faceIndices.push_back(uniqueVertexMap[vertexStr]);
      }

      // Triangulate polygon (fan method)
      for (size_t i = 1; i + 1 < faceIndices.size(); i++) {
        meshIndices->push_back(faceIndices[0]);
        meshIndices->push_back(faceIndices[i]);
        meshIndices->push_back(faceIndices[i + 1]);
      }
    }
  }

  // Final mesh (in case file doesn't end with new object)
  if (!meshIndices->empty()) {
    std::unique_ptr<Mesh> m = std::make_unique<Mesh>(
      currentName, std::move(meshVertices), std::move(meshIndices));
    _meshArr.push_back(std::move(m));
  }
}

void Object3D::
  Draw(ShaderProgram &shader, const glm::mat4 &view, const glm::mat4 &proj)
    const
{
  for (const std::unique_ptr<Mesh> &mesh: _meshArr)
    mesh->Draw(shader, modelMatrix, view, proj);
  //_meshArr[0].Draw(shader, modelMatrix, view, proj);
}
