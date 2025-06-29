#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include "Demeter/Demeter.hpp"
#include "Demeter/Entity.hpp"
#include "Demeter/Renderer/Camera.hpp"
#include "SDL_mouse.h"
#include "SDL_scancode.h"

class E_CameraControler : public Dem::IEntity {
private:
  static constexpr float vel = 5;
  static constexpr float sensitivity = 0.5F;
  static constexpr float zoomSpeed = 10.0F;

public:
  bool Init(Dem::Demeter &) override
  {
    return true;
  }

  bool Update(Dem::Demeter &d) override
  {
    float dt = d.GetTime().GetDelta();

    // Input handling
    glm::vec3 position = d.camera.GetPosition();
    if (d.GetInput().keys[SDL_SCANCODE_UP])
      position += d.camera.GetFront() * dt * vel * zoomSpeed;
    if (d.GetInput().keys[SDL_SCANCODE_DOWN])
      position -= d.camera.GetFront() * dt * vel * zoomSpeed;
    if (d.GetInput().keys[SDL_SCANCODE_LEFT])
      position -=
        glm::normalize(glm::cross(d.camera.GetFront(), d.camera.GetUp())) * dt
        * vel;
    if (d.GetInput().keys[SDL_SCANCODE_RIGHT])
      position +=
        glm::normalize(glm::cross(d.camera.GetFront(), d.camera.GetUp())) * dt
        * vel;

    if (d.GetInput().keys[SDL_SCANCODE_SPACE])
      position += d.camera.GetUp() * dt * vel;
    if (d.GetInput().keys[SDL_SCANCODE_LCTRL])
      position -= d.camera.GetUp() * dt * vel;
    d.camera.SetPosition(position);

    float yaw = d.camera.GetYaw();
    float pitch = d.camera.GetPitch();
    if (d.GetInput().mouseButtons[SDL_BUTTON_MIDDLE]) {
      if (d.GetInput().mouseX > 0)
        yaw += 5.0F * d.GetInput().mouseDeltaX * dt * sensitivity;
      if (d.GetInput().mouseX < 0)
        yaw -= 5.0F * d.GetInput().mouseDeltaX * dt * sensitivity;
      if (d.GetInput().mouseY > 0)
        pitch -= 5.0F * d.GetInput().mouseDeltaY * dt * sensitivity;
      if (d.GetInput().mouseY < 0)
        pitch += 5.0F * d.GetInput().mouseDeltaY * dt * sensitivity;
      d.camera.SetRotation(yaw, pitch);
    }
    // std::cout
    //   << std::fixed << std::setprecision(2) << "Camera: pos "
    //   << d.camera.GetPosition().x << ", " << d.camera.GetPosition().y << ",
    //   "
    //   << d.camera.GetPosition().z << "\t"
    //   << "| yaw " << yaw << "\t"
    //   << "| pitch " << pitch << "\t"
    //   << "| front " << d.camera.GetFront().x << ", " <<
    //   d.camera.GetFront().y
    //   << ", " << d.camera.GetFront().z << "\t"
    //   << "| dt " << dt << "\n";
    return true;
  }

  bool Draw(Dem::Demeter &) override
  {
    return true;
  }
};
