#pragma once

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

constexpr glm::vec3 up = glm::vec3(0.0F, 1.0F, 0.0F);

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
  glm::mat4 _proj;
  glm::mat4 _view;

  float _fov = 90.0F;
  float _aspectRatio = 800.0F / 600.0F;

  glm::vec3 _position = glm::vec3(0.0F, 0.0F, 0.0F);
  float _yaw = -90.0F;
  float _pitch = 0.0F;

  glm::vec3 cameraFront = glm::vec3(0.0F, 0.0F, -1.0F);
  glm::vec3 cameraUp = glm::vec3(0.0F, 1.0F, 0.0F);

  void UpdateProjection()
  {
    _proj = glm::
      perspective<float>(glm::radians(_fov), _aspectRatio, 0.1F, 10000.0F);
  }

  void UpdateView()
  {
    _view = glm::lookAt(_position, _position + cameraFront, cameraUp);
  }

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
  Camera(double fov, double aspectRatio) : _fov(fov), _aspectRatio(aspectRatio)
  {
    UpdateProjection();
  }

  void SetPosition(const glm::vec3 &position)
  {
    _position = position;
    UpdateView();
  }

  [[nodiscard]] const glm::vec3 &GetPosition() const
  {
    return _position;
  }

  [[nodiscard]] const glm::vec3 &GetFront() const
  {
    return cameraFront;
  }

  [[nodiscard]] const glm::vec3 &GetUp() const
  {
    return cameraUp;
  }

  void SetRotation(float yaw, float pitch)
  {
    _yaw = yaw;
    _pitch = std::min(pitch, 89.0F);
    _pitch = std::max(pitch, -89.0F);
    glm::vec3 direction;
    direction.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    direction.y = sin(glm::radians(_pitch));
    direction.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    cameraFront = glm::normalize(direction);

    UpdateView();
  }

  [[nodiscard]] float GetYaw() const
  {
    return _yaw;
  }

  [[nodiscard]] float GetPitch() const
  {
    return _pitch;
  }

  [[nodiscard]] const glm::mat4 &GetView() const
  {
    return _view;
  }

  void SetAspectRatio(float aspectRatio)
  {
    _aspectRatio = aspectRatio;
    UpdateProjection();
  }

  [[nodiscard]] float GetAspectRatio() const
  {
    return _aspectRatio;
  }

  void SetFov(float fov)
  {
    _fov = fov;
    UpdateProjection();
  }

  [[nodiscard]] float GetFov() const
  {
    return _fov;
  }

  [[nodiscard]] const glm::mat4 &GetProj() const
  {
    return _proj;
  }
};
