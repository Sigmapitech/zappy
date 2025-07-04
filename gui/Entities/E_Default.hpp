#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include "Demeter/Demeter.hpp"
#include "Demeter/Entity.hpp"
#include "Demeter/Renderer/Object3D.hpp"
#include "Demeter/Renderer/Texture.hpp"
#include "logging/Logger.hpp"

class E_Default : public Dem::IEntity {
private:
  glm::mat4 modelMatrix;
  std::shared_ptr<Object3D> obj = nullptr;
  std::shared_ptr<Texture> texture = nullptr;

public:
  bool Init(Dem::Demeter &d) override
  {
    auto tmp = d.AddObject3D("assets/cube.obj3D");
    if (!tmp) {
      Log::failed << "Failed to load object: assets/cube.obj3D";
      return false;
    }
    obj = *tmp;
    texture = d.AddTexture("assets/texture.png");
    return true;
  }

  bool Update(Dem::Demeter &d) override
  {
    float t = d.GetTime().GetCurrent() / 1000.0;
    modelMatrix = glm::rotate(glm::mat4(1.0), t, glm::vec3(0.0, 1.0, 0.0));
    return true;
  }

  bool Draw(Dem::Demeter &d) override
  {
    obj->modelMatrix = modelMatrix;
    obj->SetTexture(0, texture);
    obj->Draw(*d.GetShader(), d.camera);
    return true;
  }
};
