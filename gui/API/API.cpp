#include "API.hpp"
#include "Utils/Utils.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

void API::AskMapSize()
{
  std::string tmp_command = "msz\n";
  _command.emplace_back(tmp_command);
}

void API::AskAllTileContent()
{
  std::string tmp_command = "mct\n";
  _command.emplace_back(tmp_command);
}

void API::AskTileContent(int x, int y)
{
  std::string tmp_command = "bct " + std::to_string(x) + " "
    + std::to_string(y) + "\n";
  _command.emplace_back(tmp_command);
}

void API::AskAllTeamName()
{
  std::string tmp_command = "tna\n";
  _command.emplace_back(tmp_command);
}

void API::AskAllPlayerPos()
{
  std::string tmp_command;
  for (const auto &teamName: _allTeamName) {
    const auto &teamPlayers = _teams[teamName];
    for (const Trantor &trantor: teamPlayers) {
      tmp_command = "ppo " + std::to_string(trantor.GetId()) + "\n";
      _command.emplace_back(tmp_command);
    }
  }
}

void API::AskPlayerPos(int id)
{
  std::string tmp_command = "ppo " + std::to_string(id) + "\n";
  _command.emplace_back(tmp_command);
}

void API::AskAllPlayerLevel()
{
  std::string tmp_command;
  for (const auto &teamName: _allTeamName) {
    const auto &teamPlayers = _teams[teamName];
    for (const Trantor &trantor: teamPlayers) {
      tmp_command = "plv " + std::to_string(trantor.GetId()) + "\n";
      _command.emplace_back(tmp_command);
    }
  }
}

void API::AskPlayerLevel(int id)
{
  std::string tmp_command = "plv " + std::to_string(id) + "\n";
  _command.emplace_back(tmp_command);
}

void API::AskAllPlayerInventory()
{
  std::string tmp_command;
  for (const auto &teamName: _allTeamName) {
    const auto &teamPlayers = _teams[teamName];
    for (const Trantor &trantor: teamPlayers) {
      tmp_command = "pin " + std::to_string(trantor.GetId()) + "\n";
      _command.emplace_back(tmp_command);
    }
  }
}

void API::AskPlayerInventory(int id)
{
  std::string tmp_command = "pin " + std::to_string(id) + "\n";
  _command.emplace_back(tmp_command);
}

void API::ParseManageCommande(std::string &command)
{
  std::string word;

  if (command.empty())
    return;

  std::istringstream stream(command);
  std::string line;

  while (std::getline(stream, line)) {
    std::stringstream lineParsed(line);
    lineParsed >> word;
    switch (hash(word)) {
      case hash("WELCOME"):
        break;
      case hash("msz"):
        std::cout << "map size\n";
        break;
      case hash("bct"):
        std::cout << "content of a tile\n";
        break;
      case hash("mct"):
        std::cout << "content of the map (all the tiles)\n";
        break;
      case hash("tna"):
        std::cout << "name of all the teams\n";
        break;
      case hash("pnw"):
        std::cout << "connection of a new player\n";
        break;
      case hash("ppo"):
        std::cout << "player's position\n";
        break;
      case hash("plv"):
        std::cout << "player's level\n";
        break;
      case hash("pin"):
        std::cout << "player's inventory\n";
        break;
      case hash("pex"):
        std::cout << "expulsion\n";
        break;
      case hash("pbc"):
        std::cout << "broadcast\n";
        break;
      case hash("pic"):
        std::cout << "start of an incantation\n";
        break;
      case hash("pie"):
        std::cout << "end of an incantation\n";
        break;
      case hash("pfk"):
        std::cout << "egg laying by the player\n";
        break;
      case hash("pdr"):
        std::cout << "resource dropping\n";
        break;
      case hash("pgt"):
        std::cout << "resource collecting\n";
        break;
      case hash("pdi"):
        std::cout << "death of a player\n";
        break;
      case hash("enw"):
        std::cout << "an egg was laid by a player\n";
        break;
      case hash("ebo"):
        std::cout << "player connection for an egg\n";
        break;
      case hash("edi"):
        std::cout << "death of an egg\n";
        break;
      case hash("sgt"):
        std::cout << "time unit request\n";
        break;
      case hash("sst"):
        std::cout << "time unit modification\n";
        break;
      case hash("seg"):
        std::cout << "end of game\n";
        break;
      case hash("smg"):
        std::cout << "message from the server\n";
        break;
      case hash("suc"):
        std::cout << "unknown command\n";
        break;
      case hash("sbp"):
        std::cout << "command parameter\n";
        break;
      default:
        std::cout << "command =" << command << "\n";
        throw(std::runtime_error(
          "Error: unknown cmd, Function: "
          "ParseManageCommande, File: API.cpp"));
    }
  }
}

/*
 X Y L #n #n . . .                             start of an incantation (by the
first player) pie X Y R                                         end of an
incantation pfk #n                                            egg laying by the
player pdr #n i                                          resource dropping pgt
#n i                                          resource collecting pdi #n death
of a player enw #e #n X Y                                     an egg was laid
by a player ebo #e                                            player connection
for an egg edi #e                                            death of an egg
sgt T                                             time unit request
sst T                                             time unit modification
seg N                                             end of game
smg M                                             message from the server
*/
