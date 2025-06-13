#include <fstream>
#include <sstream>
#include <unordered_map>

#include "Mesh.hpp"

Mesh::Mesh(std::vector<Vertex> &vv, std::vector<unsigned int> &vi)
{
  if (vv.empty() || vi.empty())
    throw std::runtime_error("Mesh cannot be empty");
  _vertices.reserve(vv.size());
  _indices.reserve(vi.size());
  for (const Vertex &v: vv)
    _vertices.push_back(v);
  for (unsigned int i: vi)
    _indices.push_back(i);
  GenMesh();
}

void Mesh::GenMesh()
{
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(
    GL_ARRAY_BUFFER,
    _vertices.size() * sizeof(Vertex),
    _vertices.data(),
    GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER,
    _indices.size() * sizeof(unsigned int),
    _indices.data(),
    GL_STATIC_DRAW);

  // Position
  glVertexAttribPointer(
    0,
    3,
    GL_FLOAT,
    GL_FALSE,
    sizeof(Vertex),
    (void *)offsetof(Vertex, position));  // NOLINT
  glEnableVertexAttribArray(0);

  // TexCoord
  glVertexAttribPointer(
    1,
    2,
    GL_FLOAT,
    GL_FALSE,
    sizeof(Vertex),
    (void *)offsetof(Vertex, texCoord));  // NOLINT
  glEnableVertexAttribArray(1);

  // Normal
  // layout(location = 2) = normal
  glVertexAttribPointer(
    2,
    3,
    GL_FLOAT,
    GL_FALSE,
    sizeof(Vertex),
    (void *)offsetof(Vertex, normal));  // NOLINT
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);
}

Mesh::Mesh(const std::string &path)
{
  std::ifstream file(path);
  if (!file.is_open())
    throw std::runtime_error("Failed to open OBJ file: " + path);
  std::vector<glm::vec3> positions;
  std::vector<glm::vec2> texCoords;
  std::vector<glm::vec3> normals;

  std::unordered_map<std::string, unsigned int> uniqueVertexMap;
  std::string line;
  unsigned int index = 0;

  while (std::getline(file, line)) {
    std::istringstream ss(line);
    std::string prefix;
    ss >> prefix;

    if (prefix == "v") {
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

          _vertices.push_back(vert);
          uniqueVertexMap[vertexStr] = index++;
        }
        faceIndices.push_back(uniqueVertexMap[vertexStr]);
      }

      // Triangulate the polygon (fan method)
      for (size_t i = 1; i + 1 < faceIndices.size(); i++) {
        _indices.push_back(faceIndices[0]);
        _indices.push_back(faceIndices[i]);
        _indices.push_back(faceIndices[i + 1]);
      }
    }
  }

  GenMesh();
}

Mesh::~Mesh()
{
  glDeleteBuffers(1, &VBO);
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &EBO);
}

void Mesh::LoadTexture(SDL2::Texture &t)
{
  glGenTextures(1, &_texture);
  glBindTexture(GL_TEXTURE_2D, _texture);

  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    t.GetFormat(),
    t.GetWidth(),
    t.GetHeight(),
    0,
    t.GetFormat(),
    GL_UNSIGNED_BYTE,
    t.GetPixels());

  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(
    GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Mesh::Draw(GLuint shader) const
{
  glBindVertexArray(VAO);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _texture);
  glUniform1i(glGetUniformLocation(shader, "tex"), 0);
  glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, nullptr);
}
