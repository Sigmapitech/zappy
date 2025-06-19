#include <memory>
#include <vector>

#include "SDL2.hpp"
#include "Shader.hpp"
#include "Vertex.hpp"

struct Mesh {
private:
  GLuint VAO, VBO, EBO = 0;
  std::string _name;
  std::unique_ptr<std::vector<Vertex>> _vertices;
  std::unique_ptr<std::vector<unsigned int>> _indices;
  GLuint _texture = 0;

  void GenMesh();

public:
  Mesh(
    std::string name,
    std::unique_ptr<std::vector<Vertex>> vv,
    std::unique_ptr<std::vector<unsigned int>> vi);
  Mesh(const Mesh &) = delete;
  Mesh &operator=(const Mesh &) = delete;
  ~Mesh();

  [[nodiscard]] const std::string &GetName() const
  {
    return _name;
  }

  void LoadTexture(SDL2::Texture &t);
  void Draw(
    ShaderProgram &shader,
    const glm::mat4 &modelMatrix,
    const glm::mat4 &view,
    const glm::mat4 &projection) const;

  friend std::ostream &operator<<(std::ostream &os, const Mesh &mesh)
  {
    for (const Vertex &v: *mesh._vertices)
      os << v << '\n';
    for (size_t i = 0; i < mesh._indices->size(); i += 3)
      os << mesh._indices->at(i) << ' ' << mesh._indices->at(i + 1) << ' '
         << mesh._indices->at(i + 2) << '\n';
    return os;
  }
};
