#include "Entities/E_Mother.hpp"

#include "API/Inventory/Inventory.hpp"
#include "Demeter/Renderer/asset_dir.hpp"
#include "Entities/E_Coms.hpp"

bool E_Mother::Init(Dem::Demeter &d)
{
  std::shared_ptr<E_Coms> eComsPtr = std::
    dynamic_pointer_cast<E_Coms>(d.GetEntity(0));
  _api = eComsPtr->GetApi();

  auto tmp = d.AddObject3D(ASSET_DIR "/cube.obj3D");
  if (!tmp) {
    Log::failed << "Failed to load object: " ASSET_DIR "/cube.obj3D";
    return false;
  }
  _tile = *tmp;
  _textureTile = d.AddTexture(ASSET_DIR "/textures/ground.jpg");

  tmp = d.AddObject3D(ASSET_DIR "/ressources.obj3D");
  if (!tmp) {
    Log::failed << "Failed to load object: " ASSET_DIR "/ressources.obj3D";
    return false;
  }
  _ressources = *tmp;
  _textureRessource = d.AddTexture(ASSET_DIR "/textures/green.jpg");
  return true;
}

bool E_Mother::Update(Dem::Demeter &d)
{
  (void)d;

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
  _subWindowHandler.Run(d, _api);
  return true;
}
