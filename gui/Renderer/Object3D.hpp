#include <memory>
#include <unordered_map>

#include "Mesh.hpp"

struct Object3D {
private:
  std::unordered_map<std::string, size_t> _idMap;

  // parsing data
  std::vector<glm::vec3> positions;
  std::vector<glm::vec2> texCoords;
  std::vector<glm::vec3> normals;

  std::string currentName = "default";
  std::unique_ptr<std::vector<Vertex>> meshVertices = std::
    make_unique<std::vector<Vertex>>();
  std::unique_ptr<std::vector<unsigned int>> meshIndices = std::
    make_unique<std::vector<unsigned int>>();

  std::unordered_map<std::string, unsigned int> uniqueVertexMap;
  unsigned int index = 0;

  void HandleSubObject(std::istringstream &ss);
  void HandleVertex(std::istringstream &ss);
  void HandleVertexTexture(std::istringstream &ss);
  void HandleVertexNormal(std::istringstream &ss);
  void HandleFace(std::istringstream &ss);

  void ParseLine(const std::string &prefix, std::istringstream &ss);

public:
  glm::mat4 modelMatrix = glm::mat4(1.0);       // NOLINT
  std::vector<std::unique_ptr<Mesh>> _meshArr;  // NOLINT

  Object3D(const std::string &path);
  void
  Draw(ShaderProgram &shader, const glm::mat4 &view, const glm::mat4 &proj)
    const;
};
