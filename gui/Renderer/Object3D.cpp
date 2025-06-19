#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

#include "Object3D.hpp"
#include "Utils.hpp"

void Object3D::HandleSubObject(std::istringstream &ss)
{
  if (!meshIndices->empty()) {
    std::unique_ptr<Mesh> m = std::make_unique<Mesh>(
      currentName, std::move(meshVertices), std::move(meshIndices));
    _meshArr.push_back(std::move(m));
    // Reset for next mesh
    meshVertices = std::make_unique<std::vector<Vertex>>();
    meshIndices = std::make_unique<std::vector<unsigned int>>();
    uniqueVertexMap.clear();
    index = 0;
  }
  ss >> currentName;
}

void Object3D::HandleVertex(std::istringstream &ss)
{
  glm::vec3 pos;
  ss >> pos.x >> pos.y >> pos.z;
  positions.push_back(pos);
}

void Object3D::HandleVertexTexture(std::istringstream &ss)
{
  glm::vec2 uv;
  ss >> uv.x >> uv.y;
  uv.y = 1.0 - uv.y;  // Flip Y for OpenGL
  texCoords.push_back(uv);
}

void Object3D::HandleVertexNormal(std::istringstream &ss)
{
  glm::vec3 norm;
  ss >> norm.x >> norm.y >> norm.z;
  normals.push_back(norm);
}

void Object3D::HandleFace(std::istringstream &ss)
{
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

void Object3D::ParseLine(const std::string &prefix, std::istringstream &ss)
{
  switch (hash(prefix.c_str())) {
    case hash("o"):
    case hash("g"):
      HandleSubObject(ss);
      break;
    case hash("v"):
      HandleVertex(ss);
      break;
    case hash("vt"):
      HandleVertexTexture(ss);
      break;
    case hash("vn"):
      HandleVertexNormal(ss);
      break;
    case hash("f"):
      HandleFace(ss);
      break;
    default:
      break;  // Ignore other lines
  }
}

Object3D::Object3D(const std::string &path)
{
  std::ifstream file(path);
  if (!file.is_open())
    throw std::runtime_error("Failed to open OBJ file: " + path);

  std::string line;
  while (std::getline(file, line)) {
    std::istringstream ss(line);
    std::string prefix;
    ss >> prefix;

    ParseLine(prefix, ss);
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
