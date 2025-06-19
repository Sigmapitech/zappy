#include <memory>
#include <unordered_map>

#include "Mesh.hpp"

class Object3D {
private:
  std::unordered_map<std::string, size_t> _idMap;

public:
  glm::mat4 modelMatrix = glm::mat4(1.0);  // NOLINT

  std::vector<std::unique_ptr<Mesh>> _meshArr;

  Object3D(const std::string &path);
  void
  Draw(ShaderProgram &shader, const glm::mat4 &view, const glm::mat4 &proj)
    const;
};
