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
private:
  glm::mat4 proj;
  glm::mat4 view;

public:
  glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);  // NOLINT
  float yaw = 0.0f;                                  // NOLINT
  float pitch = 0.0f;                                // NOLINT

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
  Camera(double fov, double aspectRatio)
  {
    this->proj = glm::
      perspective<float>(glm::radians(fov), aspectRatio, 0.1, 10000.0);
  }

  [[nodiscard]] glm::mat4 GetView()
  {
    // Reset view
    view = glm::mat4(1.0F);

    // Apply rotation: pitch (X axis), then yaw (Y axis)
    view = glm::rotate(view, glm::radians(-pitch), glm::vec3(1, 0, 0));
    view = glm::rotate(view, glm::radians(-yaw), glm::vec3(0, 1, 0));

    // Apply translation (move the world opposite to camera's position)
    view = glm::translate(view, -position);
    return view;
  }

  [[nodiscard]] glm::mat4 GetProj() const
  {
    return proj;
  }
};
