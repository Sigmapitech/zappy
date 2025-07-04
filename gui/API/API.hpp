#pragma once

#include <array>
#include <mutex>

#include "API/Incantation/Incantation.hpp"
#include "API/TileMap/Tilemap.hpp"
#include "API/Trantor/Trantor.hpp"

#include <sys/poll.h>

class API {
private:
  std::array<int, 2> _pipeFdNetwork;
  std::array<int, 2> _pipeFdEvents;
  std::array<struct pollfd, 1> _pollOutFd;
  std::array<struct pollfd, 1> _pollEventOutFd;
  std::mutex _commandListLocker;

  Tilemap _tilemap;
  std::mutex _tilemapLocker;

  std::vector<std::string> _allTeamName;
  std::mutex _allTeamNameLocker;

  std::map<std::string, std::vector<Trantor>> _teams;
  std::mutex _teamsLocker;

  std::vector<Incantation> _incantationList;
  std::mutex _incantationListLocker;

  std::map<int, std::pair<int, int>> _eggList;
  std::mutex _eggListLocker;

  short _timeUnit = 0;
  std::mutex _timeUnitLocker;

  bool _isGameRunning = false;
  std::mutex _isGameRunningLocker;

  std::string _winner;
  std::mutex _winnerLocker;

  std::vector<std::string> _serverMessage;
  std::mutex _serverMessageLocker;

public:
  API();
  ~API() = default;

  /**
   * @brief Write a message to the network pipe
   *
   * @param msg Contain the message to send
   */
  void WriteMessage(const std::string &msg);

  /**
   * @brief Retrieves the input file descriptor for the network pipe
   *
   * @return The file descriptor associated with the input end of the network
   * pipe
   */
  [[nodiscard]] int GetInFd() const
  {
    return _pipeFdNetwork[0];
  }

  /**
   * @brief Retrieves the file descriptor used for relaying events from the
   * network.
   *
   * This method returns the read end of the pipe that is used to receive
   * events from the network layer. The returned file descriptor can be
   * monitored for incoming events.
   *
   * @return The file descriptor associated with network event relaying.
   */
  [[nodiscard]] int GetEventsInFd() const
  {
    return _pipeFdEvents[0];
  }

  /**
   * @brief Get the Teams
   *
   * @return std::map<std::string, std::vector<Trantor>>
   */
  [[nodiscard]] std::map<std::string, std::vector<Trantor>> GetTeams()
  {
    std::lock_guard<std::mutex> lock(_teamsLocker);
    return _teams;
  }

  /**
   * @brief Get the tilemap
   */
  Tilemap GetTilemap();

  /**
   * @brief Get the Egg List
   *
   * @return std::map<int, std::pair<int, int>>
   */
  std::map<int, std::pair<int, int>> GetEggList()
  {
    std::lock_guard<std::mutex> lock(_eggListLocker);
    return _eggList;
  }

  /**
   * @brief Get the teams name
   *
   * @return std::string
   */
  std::vector<std::string> GetTeamsName();

  /**
   * @brief Create an egg to store it into _eggList
   *
   * @param id Contain the id of the new egg
   * @param x  Contain the position in x of the new egg
   * @param y  Contain the position in y of the new egg
   */
  void AddEgg(int id, int x, int y);

  /**
   * @brief Delete an egg from _eggList
   *
   * @param id Contain the id of the deleted egg
   */
  void DeleteEgg(int id);

  /**
   * @brief Clear the command list
   */
  void ClearCommand();

  // FROM GIU

  /**
   * @brief Send a message to ask the map size
   */
  void AskMapSize();

  /**
   * @brief Send a message to ask the content of all tile
   */
  void AskAllTileContent();
  /**
   * @brief    Send a message to ask the content of one tile
   *
   * @param x  Contain the position in x of tile
   * @param y  Contain the position in y of tile
   */
  void AskTileContent(int x, int y);

  /**
   * @brief Ask all team name
   */
  void AskAllTeamName();

  /**
   * @brief Send a message to ask the position of all player
   */
  void AskAllPlayerPos();
  /**
   * @brief    Send a message to ask the position of one player
   *
   * @param id Contain the id of the player
   */
  void AskPlayerPos(int id);

  /**
   * @brief Send a message to ask the level of all player
   */
  void AskAllPlayerLevel();
  /**
   * @brief    Send a message to ask the lever of one player
   *
   * @param id Contain the id of the player
   */
  void AskPlayerLevel(int id);

  /**
   * @brief Send a message to ask the inventory of all player
   */
  void AskAllPlayerInventory();
  /**
   * @brief    Send a message to ask the inventory of one player
   *
   * @param id Contain the id of the player
   */
  void AskPlayerInventory(int id);

  // FROM SERVER

  /**
   * @brief    Parse and manage the command received from the server
   *
   * @param command Contain the full string command from the server
   */
  void ParseManageCommand(std::string &command);

  /**
   * @brief    Add a new player to a team
   *
   * @param id         Contain the id of the new player
   * @param x          Contain the position in x of the player
   * @param y          Contain the position in y of the player
   * @param orientation Contain the orientation of the player
   * @param level      Contain the level of the player
   * @param teamsName  Contain the name of the team of the player
   */
  void AddPlayer(
    int id,
    int x,
    int y,
    int orientation,
    int level,
    std::string teamsName);

  /**
   * @brief    Handle the map size from the server
   *
   * @param ss Contain the stringstream with map width (X) and height (Y)
   */
  void HandleMSZ(std::stringstream &ss);

  /**
   * @brief    Handle the tile content from the server
   *
   * @param ss Contain the stringstream with tile position and resources (X Y
   * q0-q6)
   */
  void HandleBCT(std::stringstream &ss);

  /**
   * @brief Handle all tile contents from the server
   */
  static void HandleMCT();

  /**
   * @brief    Handle a team name from the server
   *
   * @param ss Contain the stringstream with the team name (N)
   */
  void HandleTNA(std::stringstream &ss);

  /**
   * @brief    Handle a new player connection from the server
   *
   * @param ss Contain the stringstream with the player's info (#n X Y O L N)
   */
  void HandlePNW(std::stringstream &ss);

  /**
   * @brief    Handle the position of a player
   *
   * @param ss Contain the stringstream with the player's position (#n X Y O)
   */
  void HandlePPO(std::stringstream &ss);

  /**
   * @brief    Handle the level of a player
   *
   * @param ss Contain the stringstream with the player's level (#n L)
   */
  void HandlePLV(std::stringstream &ss);

  /**
   * @brief    Handle the inventory of a player
   *
   * @param ss Contain the stringstream with the inventory data (#n X Y q0-q6)
   */
  void HandlePIN(std::stringstream &ss);

  /**
   * @brief    Handle the expulsion of a player
   *
   * @param ss Contain the stringstream with the id of the player (#n)
   */
  void HandlePEX(std::stringstream &ss);

  /**
   * @brief    Handle a broadcast message sent by a player
   *
   * @param ss Contain the stringstream with the message (#n M)
   */
  void HandlePBC(std::stringstream &ss);

  /**
   * @brief    Handle the start of an incantation
   *
   * @param ss Contain the stringstream with the incantation data (X Y L #n
   * ...)
   */
  void HandlePIC(std::stringstream &ss);

  /**
   * @brief    Handle the result of an incantation
   *
   * @param ss Contain the stringstream with the result (X Y R)
   */
  void HandlePIE(std::stringstream &ss);

  /**
   * @brief    Handle the egg laying action of a player
   *
   * @param ss Contain the stringstream with the id of the player (#n)
   */
  void HandlePFK(std::stringstream &ss);

  /**
   * @brief    Handle the drop of a resource by a player
   *
   * @param ss Contain the stringstream with the player id and resource number
   * (#n i)
   */
  void HandlePDR(std::stringstream &ss);

  /**
   * @brief    Handle the pickup of a resource by a player
   *
   * @param ss Contain the stringstream with the player id and resource number
   * (#n i)
   */
  void HandlePGT(std::stringstream &ss);

  /**
   * @brief    Handle the death of a player
   *
   * @param ss Contain the stringstream with the id of the player (#n)
   */
  void HandlePDI(std::stringstream &ss);

  /**
   * @brief    Handle the creation of a new egg
   *
   * @param ss Contain the stringstream with egg and player data (#e #n X Y)
   */
  void HandleENW(std::stringstream &ss);

  /**
   * @brief    Handle the connection of a player from an egg
   *
   * @param ss Contain the stringstream with the egg id (#e)
   */
  void HandleEBO(std::stringstream &ss);

  /**
   * @brief    Handle the death of an egg
   *
   * @param ss Contain the stringstream with the egg id (#e)
   */
  void HandleEDI(std::stringstream &ss);

  /**
   * @brief    Handle the current time unit from the server
   *
   * @param ss Contain the stringstream with the time unit (T)
   */
  void HandleSGT(std::stringstream &ss);

  /**
   * @brief    Handle the modification of the time unit
   *
   * @param ss Contain the stringstream with the new time unit value (T)
   */
  void HandleSST(std::stringstream &ss);

  /**
   * @brief    Handle the end of the game and store the winner's name
   *
   * @param ss Contain the stringstream with the team name (N)
   */
  void HandleSEG(std::stringstream &ss);

  /**
   * @brief    Handle a general message from the server
   *
   * @param ss Contain the stringstream with the message (M)
   */
  void HandleSMG(std::stringstream &ss);

  /**
   * @brief Handle an unknown command from the server
   */
  static void HandleSUC();

  /**
   * @brief Handle a command with bad parameters from the server
   */
  static void HandleSBP();
};
