#pragma once

#include "Demeter/Demeter.hpp"
#include "Demeter/Entity.hpp"
#include "Demeter/Renderer/Object3D.hpp"
#include "Demeter/Renderer/Texture.hpp"

class E_EntityRotatable : public Dem::IEntity {
private:
  glm::mat4 modelMatrix;
  std::shared_ptr<Object3D> obj = nullptr;
  std::shared_ptr<Texture> texture = nullptr;
  float _rotation;

public:
  E_EntityRotatable(
    Dem::Demeter &d,
    const std::string &objPath,
    const std::string &texturePath,
    float rotation,
    int x,
    int y,
    int z);
  bool Update(Dem::Demeter &d) override;
  bool Draw(Dem::Demeter &d) override;
};
