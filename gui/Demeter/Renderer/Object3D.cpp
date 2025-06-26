#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

#include "logging/Logger.hpp"

#include "Object3D.hpp"
#include "Utils.hpp"

bool Object3D::HandleSubObject(std::istringstream &ss)
{
  if (!meshIndices->empty()) {
    std::unique_ptr<Mesh> m = std::make_unique<Mesh>();
    if (
      !m->Init(currentName, std::move(meshVertices), std::move(meshIndices))) {
      Log::failed << "Failed to initialize mesh for object: " << currentName;
      return false;
    }
    _meshArr.push_back(std::move(m));
    // Reset for next mesh
    meshVertices = std::make_unique<std::vector<Vertex>>();
    meshIndices = std::make_unique<std::vector<unsigned int>>();
    uniqueVertexMap.clear();
    index = 0;
  }
  ss >> currentName;
  return true;
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
    if (!uniqueVertexMap.contains(vertexStr)) {
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

bool Object3D::ParseLine(const std::string &prefix, std::istringstream &ss)
{
  switch (Hash(prefix.c_str())) {
    case Hash("o"):
    case Hash("g"):
      return HandleSubObject(ss);
    case Hash("v"):
      HandleVertex(ss);
      break;
    case Hash("vt"):
      HandleVertexTexture(ss);
      break;
    case Hash("vn"):
      HandleVertexNormal(ss);
      break;
    case Hash("f"):
      HandleFace(ss);
      break;
    default:
      break;  // Ignore other lines
  }
  return true;
}

bool Object3D::Init(const std::string &path)
{
  std::ifstream file(path);
  if (!file.is_open()) {
    Log::failed << "Could not open OBJ file: " << path;
    return false;
  }

  std::string line;
  while (std::getline(file, line)) {
    std::istringstream ss(line);
    std::string prefix;
    ss >> prefix;

    if (!ParseLine(prefix, ss)) {
      Log::failed << "Failed to parse line in OBJ file: " << line;
      return false;
    }
  }

  // Final mesh (in case file doesn't end with new object)
  if (!meshIndices->empty()) {
    std::unique_ptr<Mesh> m = std::make_unique<Mesh>();
    if (
      !m->Init(currentName, std::move(meshVertices), std::move(meshIndices))) {
      Log::failed << "Failed to initialize mesh for object: " << currentName;
      return false;
    }
    _meshArr.push_back(std::move(m));
  }
  return true;
}

void Object3D::Draw(ShaderProgram &shader, Camera &camera) const
{
  for (const std::unique_ptr<Mesh> &mesh: _meshArr)
    mesh->Draw(shader, modelMatrix, camera);
}

void Object3D::SetTexture(size_t meshID, std::shared_ptr<Texture> t)
{
  if (meshID < _meshArr.size())
    _meshArr[meshID]->SetTexture(std::move(t));
}
