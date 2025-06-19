#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Camera {
public:
  glm::mat4 view;  // NOLINT

private:
  glm::mat4 proj;

public:
  Camera() = default;

  Camera(const glm::mat4 &viewMtx, double fov, double aspectRatio)
    : view(viewMtx)
  {
    this->proj = glm::
      perspective<float>(glm::radians(fov), aspectRatio, 0.1, 10000.0);
  }

  [[nodiscard]] glm::mat4 GetProj() const
  {
    return proj;
  }
};
