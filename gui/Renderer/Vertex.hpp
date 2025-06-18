#include <ostream>

#include <glm/glm.hpp>

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
