#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include "Entities/E_Mother.hpp"

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

  tmp = d.AddObject3D(ASSET_DIR "/rock.obj3D");
  if (!tmp) {
    Log::failed << "Failed to load object: " ASSET_DIR "/rock.obj3D";
    return false;
  }
  _ressources = *tmp;

  tmp = d.AddObject3D(ASSET_DIR "/17855_Alien_v1.obj3D");
  if (!tmp) {
    Log::failed << "Failed to load object: " ASSET_DIR "/17855_Alien_v1.obj3D";
    return false;
  }
  _player = *tmp;

  tmp = d.AddObject3D(ASSET_DIR "/Egg.obj3D");
  if (!tmp) {
    Log::failed << "Failed to load object: " ASSET_DIR "/Egg.obj3D";
    return false;
  }
  _egg = *tmp;

  _textureTile = d.AddTexture(ASSET_DIR "/textures/grass.png");

  _textureFood = d.AddTexture(ASSET_DIR "/textures/brown.png");
  _textureLinemate = d.AddTexture(ASSET_DIR "/textures/yellow.png");
  _textureDeraumere = d.AddTexture(ASSET_DIR "/textures/green.png");
  _textureSibur = d.AddTexture(ASSET_DIR "/textures/blue.png");
  _textureMendiane = d.AddTexture(ASSET_DIR "/textures/red.png");
  _texturePhiras = d.AddTexture(ASSET_DIR "/textures/white.png");
  _textureThystame = d.AddTexture(ASSET_DIR "/textures/pink.png");

  _texturePlayer = d.AddTexture(ASSET_DIR "/textures/purple.png");
  _textureEgg = d.AddTexture(ASSET_DIR "/textures/white.png");
  return true;
}

namespace {
  std::string getTeamNameFromMessage(
    const std::string &message,
    const std::map<std::string, std::vector<Trantor>> &teams)
  {
    std::istringstream iss(message);
    std::string command;
    iss >> command;

    int id = -1;
    std::string teamName;

    if (command == "pnw") {
      std::string idStr;
      int x;
      int y;
      int o;
      int l;

      iss >> idStr >> x >> y >> o >> l >> teamName;
      if (teams.find(teamName) != teams.end())
        return teamName;

      if (idStr[0] == '#')
        id = std::stoi(idStr.substr(1));
    }

    else if (
      command == "ppo" || command == "plv" || command == "pin"
      || command == "pex" || command == "pbc" || command == "pfk"
      || command == "pdr" || command == "pgt" || command == "pdi") {
      std::string idStr;
      iss >> idStr;
      if (!idStr.empty() && idStr[0] == '#')
        id = std::stoi(idStr.substr(1));
    }

    else if (command == "enw") {
      std::string eggStr;
      std::string idStr;
      int x;
      int y;
      iss >> eggStr >> idStr >> x >> y;
      if (!idStr.empty() && idStr[0] == '#')
        id = std::stoi(idStr.substr(1));
    }

    if (id != -1) {
      for (const auto &[team, trantors]: teams) {
        for (const auto &trantor: trantors)
          if (trantor.GetId() == id)
            return team;
      }
    }

    return {};
  }
}  // namespace

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
    // Set _events[_eventIndex].first to the team name
    if (!getTeamNameFromMessage(message, _api->GetTeams()).empty()) {
      _events[_eventIndex].first = getTeamNameFromMessage(
        message, _api->GetTeams());
      _events[_eventIndex].second = std::move(message);
      if (_eventCount < _events.size())
        _eventCount++;
    }
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
        for (size_t q = 0; q < item.second; q++) {
          modelMatrix = glm::translate(
            glm::mat4(1.0),
            glm::vec3(
              (i) + (0.1F * itemId) - 0.25F, tileHeight + (0.05F * q), (j)));
          _ressources->modelMatrix = modelMatrix;
          if (item.first == "food")
            _ressources->SetTexture(0, _textureFood);
          else if (item.first == "linemate")
            _ressources->SetTexture(0, _textureLinemate);
          else if (item.first == "deraumere")
            _ressources->SetTexture(0, _textureDeraumere);
          else if (item.first == "sibur")
            _ressources->SetTexture(0, _textureSibur);
          else if (item.first == "mendiane")
            _ressources->SetTexture(0, _textureMendiane);
          else if (item.first == "phiras")
            _ressources->SetTexture(0, _texturePhiras);
          else if (item.first == "thystame")
            _ressources->SetTexture(0, _textureThystame);
          else {
            Log::warn
              << "Unknown item type: " << item.first << ", skipping drawing.";
            continue;  // Skip unknown items
          }
          _ressources->Draw(*d.GetShader(), d.camera);
        }
        itemId++;
      }
    }
  // Draw Players
  for (auto &teams: _api->GetTeams()) {
    for (Trantor &trantor: teams.second) {
      modelMatrix = glm::translate(
        glm::mat4(1.0),
        glm::vec3(
          trantor.GetPosition().first
            + hashToRange(trantor.GetId() + trantor.GetPosition().first, 0.5),
          tileHeight,
          trantor.GetPosition().second
            + hashToRange(
              trantor.GetId() + trantor.GetPosition().second, 0.5)));
      modelMatrix = glm::rotate(
        modelMatrix,
        glm::radians(trantor.GetRotation() * 45.0F),
        glm::vec3(0.0, 1.0, 0.0));
      _player->modelMatrix = modelMatrix;
      _player->SetTexture(0, _texturePlayer);
      _player->Draw(*d.GetShader(), d.camera);
    }
  }
  // Draw Eggs
  for (auto &egg: _api->GetEggList()) {
    modelMatrix = glm::translate(
      glm::mat4(1.0),
      glm::vec3(
        egg.second.first + hashToRange(egg.first + egg.second.first, 0.5),
        tileHeight,
        egg.second.second + hashToRange(egg.first + egg.second.second, 0.5)));
    _egg->modelMatrix = modelMatrix;
    _egg->SetTexture(0, _textureEgg);
    _egg->Draw(*d.GetShader(), d.camera);
  }
  // Draw ImGui
  _subWindowHandler.Run(d, _api, _events, _eventIndex, _eventCount);
  return true;
}
