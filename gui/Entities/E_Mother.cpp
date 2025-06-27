#include "Entities/E_Mother.hpp"
#include <sstream>

#include "API/Inventory/Inventory.hpp"
#include "Demeter/Renderer/asset_dir.hpp"
#include "Entities/E_Coms.hpp"
#include "logging/Logger.hpp"

bool E_Mother::Init(Dem::Demeter &d)
{
  std::shared_ptr<E_Coms> eComsPtr = std::
    dynamic_pointer_cast<E_Coms>(d.GetEntity(0));
  _api = eComsPtr->GetApi();

  fd = _api->GetEventsInFd();

  _pollOutFd[0].fd = fd;
  _pollOutFd[0].events = POLLIN;
  _pollOutFd[0].revents = 0;

  auto tmp = d.AddObject3D(ASSET_DIR "/cube.obj3D");
  if (!tmp) {
    Log::failed << "Failed to load object: " ASSET_DIR "/cube.obj3D";
    return false;
  }
  _tile = *tmp;
  _textureTile = d.AddTexture(ASSET_DIR "/textures/grass.png");

  tmp = d.AddObject3D(ASSET_DIR "/ressources.obj3D");
  if (!tmp) {
    Log::failed << "Failed to load object: " ASSET_DIR "/ressources.obj3D";
    return false;
  }
  _ressources = *tmp;
  _textureRessource = d.AddTexture(ASSET_DIR "/textures/green.png");
  return true;
}

bool E_Mother::Update(Dem::Demeter &)
{
  _api->AskAllPlayerInventory();
  _api->AskAllPlayerLevel();
  _api->AskAllPlayerPos();
  _api->AskAllTeamName();
  _api->AskAllTileContent();

  if (_api->GetTilemap().GetSize().first == 0
      || _api->GetTilemap().GetSize().second == 0) {
    return false;
  }

  if (_tilemap.empty()) {
    auto [width, height] = _api->GetTilemap().GetSize();
    _tilemap.resize(height, std::vector<TileData>(width));

    for (int i = 0; i < height; ++i)
      for (int j = 0; j < width; ++j)
        _tilemap[i][j] = {.x = i, .y = 0, .z = j, .rotation = 0.0};
  }

  std::array<char, 1024> buffer;

  std::string fullData;
  while (true) {
    if (poll(_pollOutFd.data(), _pollOutFd.size(), 0) == -1) {
      Log::failed << "Poll failed in E_Mother::Update";
      return false;
    }
    if (!(_pollOutFd[0].revents & POLLIN))
      break;  // No data to read
    ssize_t bytesRead = read(fd, buffer.data(), buffer.size());
    if (bytesRead == -1) {
      Log::failed << "Read failed in E_Mother::Update";
      return false;
    }
    if (bytesRead == 0)
      break;  // No more data
    fullData.append(buffer.data(), bytesRead);
  }

  std::stringstream ss(fullData);
  std::string message;
  while (std::getline(ss, message)) {
    _eventIndex = (_eventIndex + 1) % _events.size();
    _events[_eventIndex] = std::move(message);
    if (_eventCount < _events.size())
      _eventCount++;
  }
  return true;
}

static constexpr float tileHeight = 1.0;

bool E_Mother::Draw(Dem::Demeter &d)
{
  // Draw Tiles
  for (int i = 0; i < _api->GetTilemap().GetSize().second; i++)
    for (int j = 0; j < _api->GetTilemap().GetSize().first; j++) {
      modelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(i, 0.0, j));
      _tile->modelMatrix = modelMatrix;
      _tile->SetTexture(0, _textureTile);
      _tile->Draw(*d.GetShader(), d.camera);

      std::map<std::string, size_t> tileInventory =
        _api->GetTilemap().GetTileInventory(j, i).GetInventory();

      int itemId = 0;
      for (auto &item: tileInventory) {
        for (size_t q = 0; q < item.second; ++q) {
          modelMatrix = glm::translate(
            glm::mat4(1.0),
            glm::vec3((i) + (0.1 * itemId), tileHeight + (0.1 * q), (j)));
          _ressources->modelMatrix = modelMatrix;
          _ressources->SetTexture(0, _textureRessource);
          _ressources->Draw(*d.GetShader(), d.camera);
        }
        itemId++;
      }
    }
  // Draw Players
  for (auto &teams: _api->GetTeams())
    for (Trantor &trantor: teams.second) {
      modelMatrix = glm::translate(
        glm::mat4(1.0),
        glm::vec3(
          trantor.GetPosition().first + randomFloat(0.1, 1.0),
          tileHeight,
          trantor.GetPosition().second + randomFloat(0.1, 1.0)));
      _player->modelMatrix = modelMatrix;
      _player->SetTexture(0, _texturePlayer);
      _player->Draw(*d.GetShader(), d.camera);
    }
  // Draw Eggs
  for (auto &egg: _api->GetEggList()) {
    modelMatrix = glm::translate(
      glm::mat4(1.0),
      glm::vec3(
        egg.second.first + randomFloat(0.1, 1.0),
        tileHeight,
        egg.second.second + randomFloat(0.1, 1.0)));
    _egg->modelMatrix = modelMatrix;
    _egg->SetTexture(0, _textureEgg);
    _egg->Draw(*d.GetShader(), d.camera);
  }
  // Draw ImGui
  _subWindowHandler.Run(d, _api, _events, _eventIndex, _eventCount);
  return true;
}
