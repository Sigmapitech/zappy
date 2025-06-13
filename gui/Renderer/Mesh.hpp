#include <ostream>
#include <vector>

#include <glm/glm.hpp>

#include "SDL2.hpp"

struct Vertex {
  glm::vec3 position;
  glm::vec2 texCoord;
  glm::vec3 normal;

  friend std::ostream &operator<<(std::ostream &os, const Vertex &v)
  {
    os << v.position.x << ' ' << v.position.y << ' ' << v.position.z << '\t'
       << v.texCoord.x << ' ' << v.texCoord.y << '\t' << v.normal.x << ' '
       << v.normal.y << ' ' << v.normal.z << '\n';
    return os;
  }
};

struct Mesh {
private:
  GLuint VAO, VBO, EBO;
  std::vector<Vertex> _vertices;
  std::vector<unsigned int> _indices;
  GLuint _texture = 0;

  void GenMesh();

public:
  Mesh(std::vector<Vertex> &vv, std::vector<unsigned int> &vi);
  Mesh(const std::string &path);
  ~Mesh();

  void LoadTexture(SDL2::Texture &t);
  void Draw(GLuint shader) const;

  friend std::ostream &operator<<(std::ostream &os, const Mesh &mesh)
  {
    for (const Vertex &v: mesh._vertices)
      os << v << '\n';
    for (size_t i = 0; i < mesh._indices.size(); i += 3)
      os << mesh._indices[i] << ' ' << mesh._indices[i + 1] << ' '
         << mesh._indices[i + 2] << '\n';
    return os;
  }
};
