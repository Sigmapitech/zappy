#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include "Demeter/Demeter.hpp"
#include "Demeter/Entity.hpp"
#include "Demeter/Renderer/Camera.hpp"
#include "Demeter/Renderer/Object3D.hpp"
#include "Demeter/Renderer/Texture.hpp"
#include "SDL_mouse.h"
#include "SDL_scancode.h"

#include <iostream>

class E_CameraControler : public Dem::IEntity {
private:
  glm::mat4 modelMatrix;
  std::shared_ptr<Object3D> obj = nullptr;
  std::shared_ptr<Texture> texture = nullptr;

public:
  E_CameraControler(Dem::Demeter &d)
  {
    obj = d.AddObject3D("assets/cube.obj3D");
    texture = d.AddTexture("assets/texture.png");
  }

  bool Update(Dem::Demeter &d) override
  {
    float deltaTime = d.GetTime().GetDelta();
    float t = d.GetTime().GetCurrent() / 1000.0;
    modelMatrix = glm::rotate(glm::mat4(1.0), t, glm::vec3(0.0, 1.0, 0.0));

    // Input handling
    if (d.GetInput().keys[SDL_SCANCODE_RIGHT])
      d.camera.position += glm::vec3(1.0, 0.0, 0.0) * deltaTime;
    if (d.GetInput().keys[SDL_SCANCODE_LEFT])
      d.camera.position -= glm::vec3(1.0, 0.0, 0.0) * deltaTime;
    if (d.GetInput().keys[SDL_SCANCODE_DOWN])
      d.camera.position += glm::vec3(0.0, 0.0, 1.0) * deltaTime;
    if (d.GetInput().keys[SDL_SCANCODE_UP])
      d.camera.position -= glm::vec3(0.0, 0.0, 1.0) * deltaTime;

    if (d.GetInput().keys[SDL_SCANCODE_SPACE])
      d.camera.position += glm::vec3(0.0, 10.0, 0.0) * deltaTime;
    if (d.GetInput().keys[SDL_SCANCODE_LCTRL])
      d.camera.position -= glm::vec3(0.0, 10.0, 0.0) * deltaTime;

    if (d.GetInput().mouseButtons[SDL_BUTTON_RIGHT]) {
      if (d.GetInput().mouseX > 0)
        d.camera.yaw += 5.0F * d.GetInput().mouseDeltaX * deltaTime;
      if (d.GetInput().mouseX < 0)
        d.camera.yaw -= 5.0F * d.GetInput().mouseDeltaX * deltaTime;
      if (d.GetInput().mouseY > 0)
        d.camera.pitch += 5.0F * d.GetInput().mouseDeltaY * deltaTime;
      if (d.GetInput().mouseY < 0)
        d.camera.pitch -= 5.0F * d.GetInput().mouseDeltaY * deltaTime;
    }
    std::cout
      << "Camera Position: " << d.camera.position.x << ", "
      << d.camera.position.y << ", " << d.camera.position.z << "\n"
      << "Camera Yaw: " << d.camera.yaw << "\n"
      << "Camera Pitch: " << d.camera.pitch << "\n"
      << "deltaTime: " << deltaTime << "\n";
    return true;
  }

  bool Draw(Dem::Demeter &d) override
  {
    (void)d;
    return true;
  }
};
