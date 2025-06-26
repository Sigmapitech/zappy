#pragma once

#include "API/API.hpp"
#include "Demeter/Demeter.hpp"
#include "Demeter/Entity.hpp"
#include "Demeter/Renderer/Object3D.hpp"
#include "Demeter/Renderer/Texture.hpp"
#include "Entities/SubWindowHandler.hpp"
#include "Utils/Utils.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>

struct TileData {
  int x;
  int y;
  int z;
  float rotation;
};

class E_Mother : public Dem::IEntity {
private:
  glm::mat4 modelMatrix;

  std::shared_ptr<Object3D> _tile = nullptr;
  std::shared_ptr<Texture> _textureTile = nullptr;

  std::shared_ptr<Object3D> _ressources = nullptr;
  std::shared_ptr<Object3D> _player = nullptr;
  std::shared_ptr<Object3D> _egg = nullptr;

  std::shared_ptr<Texture> _textureRessource = nullptr;
  std::shared_ptr<Texture> _texturePlayer = nullptr;
  std::shared_ptr<Texture> _textureEgg = nullptr;

  std::vector<std::vector<TileData>> _tilemap;
  std::shared_ptr<API> _api;

  SubWindowHandler _subWindowHandler;

public:
  bool Init(Dem::Demeter &d) override;

  bool Update(Dem::Demeter &) override;

  bool Draw(Dem::Demeter &d) override;
};
