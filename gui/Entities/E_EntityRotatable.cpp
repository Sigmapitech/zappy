#include "E_EntityRotatable.hpp"

E_EntityRotatable::E_EntityRotatable(
  Dem::Demeter &d,
  const std::string &objPath,
  const std::string &texturePath,
  float rotation,
  int x,
  int y,
  int z)
  : _rotation(rotation)
{
  auto tmp = d.AddObject3D(objPath);
  if (!tmp)
    throw std::runtime_error("Failed to load object: " + objPath);
  obj = *tmp;
  texture = d.AddTexture(texturePath);
  modelMatrix = glm::translate(glm::mat4(1), glm::vec3(x, y, z));
}

bool E_EntityRotatable::Update(Dem::Demeter &d)
{
  float t = d.GetTime().GetCurrent() / 1000.0;
  t *= _rotation;
  modelMatrix = glm::rotate(modelMatrix, t, glm::vec3(0.0, 1.0, 0.0));
  return true;
}

bool E_EntityRotatable::Draw(Dem::Demeter &d)
{
  obj->modelMatrix = modelMatrix;
  obj->SetTexture(0, texture);
  obj->Draw(*d.GetShader(), d.camera);
  return true;
}
