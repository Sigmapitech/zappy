#pragma once

#include "API/TileMap/Tilemap.hpp"
#include "API/Trantor/Trantor.hpp"

#include <map>
#include <mutex>
#include <string>
#include <vector>

class Incantation {
private:
  std::vector<int> _member;
  int _level;
  std::pair<int, int> _position;
  std::string _state;

public:
  Incantation(int level, int x, int y);
  ~Incantation() = default;

  void AddMember(int id);
  std::pair<int, int> GetPosition();
  void SetState(std::string &state);
};

class API {
private:
  std::vector<std::string> _command;
  std::mutex _commandListLocker;

  Tilemap _tilemap;
  std::vector<std::string> _allTeamName;
  std::map<std::string, std::vector<Trantor>> _teams;
  std::vector<Incantation> _incantationList;
  std::map<int, std::pair<int, int>> _eggList;
  short _timeUnit = 0;
  bool _isGameRunning = false;
  std::string _winner;
  std::vector<std::string> _serverMessage;

public:
  API() = default;
  ~API() = default;

  void AddEgg(int id, int x, int y);
  void DeleteEgg(int id);

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
  void HandleMSZ(std::stringstream &ss);
  void HandleBCT(std::stringstream &ss);
  static void HandleMCT();
  void HandleTNA(std::stringstream &ss);
  void HandlePNW(std::stringstream &ss);
  void HandlePPO(std::stringstream &ss);
  void HandlePLV(std::stringstream &ss);
  void HandlePIN(std::stringstream &ss);
  void HandlePEX(std::stringstream &ss);
  void HandlePBC(std::stringstream &ss);
  void HandlePIC(std::stringstream &ss);
  void HandlePIE(std::stringstream &ss);
  void HandlePFK(std::stringstream &ss);
  void HandlePDR(std::stringstream &ss);
  void HandlePGT(std::stringstream &ss);
  void HandlePDI(std::stringstream &ss);
  void HandleENW(std::stringstream &ss);
  void HandleEBO(std::stringstream &ss);
  void HandleEDI(std::stringstream &ss);
  void HandleSGT(std::stringstream &ss);
  void HandleSST(std::stringstream &ss);
  void HandleSEG(std::stringstream &ss);
  void HandleSMG(std::stringstream &ss);
  static void HandleSUC();
  static void HandleSBP();
};
