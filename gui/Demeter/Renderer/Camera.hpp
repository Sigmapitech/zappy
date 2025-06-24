#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/**
 * @struct Camera
 * @brief Represents a camera with view and projection matrices for 3D
 * rendering.
 *
 * The Camera struct encapsulates the view and projection matrices used in 3D
 * graphics. It provides constructors for initializing the camera with a view
 * matrix, field of view, and aspect ratio, and offers access to the projection
 * matrix.
 */
struct Camera {
public:
  glm::mat4 view;  // NOLINT

private:
  glm::mat4 proj;

public:
  Camera() = default;

  /**
   * @brief Constructs a Camera object with the specified view matrix, field of
   * view, and aspect ratio.
   *
   * Initializes the camera's view matrix and computes the projection matrix
   * using a perspective projection.
   *
   * @param viewMtx The view matrix representing the camera's orientation and
   * position in the world.
   * @param fov The field of view angle (in degrees) for the perspective
   * projection.
   * @param aspectRatio The aspect ratio (width divided by height) of the
   * viewport.
   */
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
