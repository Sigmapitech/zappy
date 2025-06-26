#pragma once

#include <cstddef>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>

#include "API/Inventory/Inventory.hpp"
#include "Demeter/Demeter.hpp"
#include "Demeter/Entity.hpp"
#include "Demeter/Renderer/Object3D.hpp"
#include "Demeter/Renderer/Texture.hpp"
#include "Demeter/Renderer/asset_dir.hpp"
#include "Entities/E_Coms.hpp"
#include "logging/Logger.hpp"

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
  std::shared_ptr<Texture> _textureRessource = nullptr;

  std::vector<std::vector<TileData>> _tilemap;
  std::shared_ptr<API> _api;

public:
  bool Init(Dem::Demeter &d) override
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

  bool Update(Dem::Demeter &) override
  {
    if (_api->GetTilemap().GetSize().first == 0
        || _api->GetTilemap().GetSize().second == 0)
      return false;

    if (_tilemap.empty()) {
      auto [width, height] = _api->GetTilemap().GetSize();
      _tilemap.resize(height, std::vector<TileData>(width));

      for (int i = 0; i < height; ++i)
        for (int j = 0; j < width; ++j)
          _tilemap[i][j] = {.x = i, .y = 0, .z = j, .rotation = 0.0};
    }
    return true;
  }

  bool Draw(Dem::Demeter &d) override
  {
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
              glm::vec3((i) + (0.1 * itemId), 0.5 + (0.1 * q), (j)));
            _ressources->modelMatrix = modelMatrix;
            _ressources->SetTexture(0, _textureRessource);
            _ressources->Draw(*d.GetShader(), d.camera);
          }
          itemId++;
        }
      }
    return true;
  }
};
