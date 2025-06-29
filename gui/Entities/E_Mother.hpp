#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <sys/poll.h>
#include <vector>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <poll.h>

#include "API/API.hpp"
#include "Demeter/Demeter.hpp"
#include "Demeter/Entity.hpp"
#include "Demeter/Renderer/Object3D.hpp"
#include "Demeter/Renderer/Texture.hpp"
#include "Entities/SubWindowHandler.hpp"
#include "Utils/Utils.hpp"

struct TileData {
  int x;
  int y;
  int z;
  float rotation;
};

class E_Mother : public Dem::IEntity {
private:
  int fd = 0;
  std::array<pollfd, 2> _pollOutFd;
  std::array<std::pair<std::string, std::string>, 256> _events;
  size_t _eventCount = 0;
  size_t _eventIndex = 0;
  glm::mat4 modelMatrix;

  std::shared_ptr<Object3D> _tile = nullptr;
  std::shared_ptr<Texture> _textureTile = nullptr;

  std::shared_ptr<Object3D> _ressources = nullptr;
  std::shared_ptr<Object3D> _player = nullptr;
  std::shared_ptr<Object3D> _egg = nullptr;

  std::shared_ptr<Texture> _textureFood = nullptr;
  std::shared_ptr<Texture> _textureLinemate = nullptr;
  std::shared_ptr<Texture> _textureDeraumere = nullptr;
  std::shared_ptr<Texture> _textureSibur = nullptr;
  std::shared_ptr<Texture> _textureMendiane = nullptr;
  std::shared_ptr<Texture> _texturePhiras = nullptr;
  std::shared_ptr<Texture> _textureThystame = nullptr;

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
