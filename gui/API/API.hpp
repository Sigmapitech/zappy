#pragma once

#include "API/TileMap/Tilemap.hpp"
#include "API/Trantor/Trantor.hpp"

#include <map>
#include <string>
#include <vector>

class API {
private:
  std::vector<std::string> _command;
  Tilemap _map;
  std::vector<std::string> _allTeamName;
  std::map<std::string, std::vector<Trantor>> _teams;

public:
  API() = default;
  ~API() = default;

  // FROM GIU
  void AskMapSize();

  void AskAllTileContent();
  void AskTileContent(int x, int y);

  void AskAllTeamName();

  void AskAllPlayerPos();
  void AskPlayerPos(int id);

  void AskAllPlayerLevel();
  void AskPlayerLevel(int id);

  void AskAllPlayerInventory();
  void AskPlayerInventory(int id);

  // FROM SERVER
  void ParseManageCommande(std::string &command);
  void AddPlayer(
    int id,
    int x,
    int y,
    int orientation,
    int level,
    std::string teamsName);
};
