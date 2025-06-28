#include <functional>
#include <iostream>
#include <sstream>

#include <sys/poll.h>
#include <unistd.h>

#include "API.hpp"
#include "API/TileMap/Tilemap.hpp"
#include "API/Trantor/Trantor.hpp"
#include "Utils/Utils.hpp"

API::API()
{
  // Initialize the pipe for network communication
  if (pipe(_pipeFdNetwork.data()) == -1)
    throw std::runtime_error(
      "Error: pipe failed, Function: API constructor, File: API.cpp");

  // Create pipe for events
  if (pipe(_pipeFdEvents.data()) == -1)
    throw std::runtime_error(
      "Error: pipe failed. Function: API constructor, File: API.cpp");

  // Initialize the poll structures
  _pollOutFd[0].fd = _pipeFdNetwork[1];
  _pollOutFd[0].events = POLLOUT;
  _pollOutFd[0].revents = 0;

  _pollEventOutFd[0].fd = _pipeFdEvents[1];
  _pollEventOutFd[0].events = POLLOUT;
  _pollEventOutFd[0].revents = 0;
}

void API::WriteMessage(const std::string &msg)
{
  if (poll(_pollOutFd.data(), _pollOutFd.size(), -1) == -1)
    throw std::runtime_error(
      "Error: poll failed, Function: WriteMessage, File: API.cpp");
  // No need to check for POLLOUT here, there is only one fd in _pollOutFd
  if (write(_pipeFdNetwork[1], msg.c_str(), msg.size()) == -1)
    throw std::runtime_error(
      "Error: write failed, Function: WriteMessage, File: API.cpp");
}

Tilemap API::GetTilemap()
{
  std::lock_guard<std::mutex> lock(_tilemapLocker);
  return _tilemap;
}

void API::AddEgg(int id, int x, int y)
{
  std::lock_guard<std::mutex> locker(_eggListLocker);
  _eggList[id].first = x;
  _eggList[id].second = y;
}

void API::DeleteEgg(int id)
{
  std::lock_guard<std::mutex> locker(_eggListLocker);
  _eggList.erase(id);
}

void API::AskMapSize()
{
  std::string tmp_command = "msz\n";

  std::lock_guard<std::mutex> lock(_commandListLocker);
  WriteMessage(tmp_command);
}

void API::AskAllTileContent()
{
  std::string tmp_command = "mct\n";

  std::lock_guard<std::mutex> lock(_commandListLocker);
  WriteMessage(tmp_command);
}

void API::AskTileContent(int x, int y)
{
  std::string tmp_command = "bct " + std::to_string(x) + " "
    + std::to_string(y) + "\n";

  std::lock_guard<std::mutex> lock(_commandListLocker);
  WriteMessage(tmp_command);
}

void API::AskAllTeamName()
{
  std::string tmp_command = "tna\n";

  std::lock_guard<std::mutex> lock(_commandListLocker);
  WriteMessage(tmp_command);
}

void API::AskAllPlayerPos()
{
  std::string tmp_command;

  std::lock_guard<std::mutex> lockerName(_allTeamNameLocker);
  for (const auto &teamName: _allTeamName) {
    std::lock_guard<std::mutex> lockerTeam(_teamsLocker);
    const auto &teamPlayers = _teams[teamName];
    for (const Trantor &trantor: teamPlayers) {
      std::lock_guard<std::mutex> lock(_commandListLocker);
      tmp_command = "ppo " + std::to_string(trantor.GetId()) + "\n";
      WriteMessage(tmp_command);
    }
  }
}

void API::AskPlayerPos(int id)
{
  std::string tmp_command = "ppo " + std::to_string(id) + "\n";

  std::lock_guard<std::mutex> lock(_commandListLocker);
  WriteMessage(tmp_command);
}

void API::AskAllPlayerLevel()
{
  std::string tmp_command;

  std::lock_guard<std::mutex> lock(_commandListLocker);
  std::lock_guard<std::mutex> lockerName(_allTeamNameLocker);
  for (const auto &teamName: _allTeamName) {
    std::lock_guard<std::mutex> lockerTeam(_teamsLocker);
    const auto &teamPlayers = _teams[teamName];
    for (const Trantor &trantor: teamPlayers) {
      tmp_command = "plv " + std::to_string(trantor.GetId()) + "\n";
      WriteMessage(tmp_command);
    }
  }
}

void API::AskPlayerLevel(int id)
{
  std::string tmp_command = "plv " + std::to_string(id) + "\n";

  std::lock_guard<std::mutex> lock(_commandListLocker);
  WriteMessage(tmp_command);
}

void API::AskAllPlayerInventory()
{
  std::string tmp_command;

  std::lock_guard<std::mutex> lockerName(_allTeamNameLocker);
  for (const auto &teamName: _allTeamName) {
    std::lock_guard<std::mutex> lock(_commandListLocker);
    std::lock_guard<std::mutex> lockerTeam(_teamsLocker);
    const auto &teamPlayers = _teams[teamName];
    for (const Trantor &trantor: teamPlayers) {
      tmp_command = "pin " + std::to_string(trantor.GetId()) + "\n";
      WriteMessage(tmp_command);
    }
  }
}

void API::AskPlayerInventory(int id)
{
  std::string tmp_command = "pin " + std::to_string(id) + "\n";

  std::lock_guard<std::mutex> lock(_commandListLocker);
  WriteMessage(tmp_command);
}

void API::ParseManageCommand(std::string &command)
{
  if (command.empty())
    return;

  static const std::map<std::string, std::function<void(std::stringstream &)>>
    commandHandlers = {
      {"msz", [this](std::stringstream &ss) { HandleMSZ(ss); }},
      {"bct", [this](std::stringstream &ss) { HandleBCT(ss); }},
      {"mct",
       [](std::stringstream &ss) {
         (void)ss;
         API::HandleMCT();
       }},
      {"tna", [this](std::stringstream &ss) { HandleTNA(ss); }},
      {"pnw", [this](std::stringstream &ss) { HandlePNW(ss); }},
      {"ppo", [this](std::stringstream &ss) { HandlePPO(ss); }},
      {"plv", [this](std::stringstream &ss) { HandlePLV(ss); }},
      {"pin", [this](std::stringstream &ss) { HandlePIN(ss); }},
      {"pex", [this](std::stringstream &ss) { HandlePEX(ss); }},
      {"pbc", [this](std::stringstream &ss) { HandlePBC(ss); }},
      {"pic", [this](std::stringstream &ss) { HandlePIC(ss); }},
      {"pie", [this](std::stringstream &ss) { HandlePIE(ss); }},
      {"pfk", [this](std::stringstream &ss) { HandlePFK(ss); }},
      {"pdr", [this](std::stringstream &ss) { HandlePDR(ss); }},
      {"pgt", [this](std::stringstream &ss) { HandlePGT(ss); }},
      {"pdi", [this](std::stringstream &ss) { HandlePDI(ss); }},
      {"enw", [this](std::stringstream &ss) { HandleENW(ss); }},
      {"ebo", [this](std::stringstream &ss) { HandleEBO(ss); }},
      {"edi", [this](std::stringstream &ss) { HandleEDI(ss); }},
      {"sgt", [this](std::stringstream &ss) { HandleSGT(ss); }},
      {"sst", [this](std::stringstream &ss) { HandleSST(ss); }},
      {"seg", [this](std::stringstream &ss) { HandleSEG(ss); }},
      {"smg", [this](std::stringstream &ss) { HandleSMG(ss); }},
      {"suc",
       [](std::stringstream &ss) {
         (void)ss;
         API::HandleSUC();
       }},
      {"sbp", [](std::stringstream &ss) {
         (void)ss;
         API::HandleSBP();
       }}};

  std::istringstream stream(command);
  std::string line;

  while (std::getline(stream, line)) {
    line += "\n";
    if (poll(_pollEventOutFd.data(), _pollEventOutFd.size(), 0) == -1)
      throw std::runtime_error(
        "Error: poll failed, Function: ParseManageCommand, File: API.cpp");
    if (_pollEventOutFd[0].revents & POLLOUT)
      if (write(_pipeFdEvents[1], line.c_str(), line.size()) == -1)
        throw std::runtime_error(
          "Error: write failed, Function: ParseManageCommand, File: API.cpp");

    std::stringstream lineParsed(line);
    std::string word;
    lineParsed >> word;

    if (word == "WELCOME")
      continue;

    auto it = commandHandlers.find(word);
    if (it != commandHandlers.end()) {
      it->second(lineParsed);
    } else {
      throw std::runtime_error(
        "Error: unknown cmd, Function: ParseManageCommande, File: API.cpp");
    }
  }
}

void API::HandleMSZ(std::stringstream &ss)
{
  int x;
  int y;

  if (!(ss >> x >> y))
    throw std::runtime_error(
      "Error: invalid msz params, Function: HandleMSZ, File: API.cpp");
  // std::cout << "map size: x=" << x << " y=" << y << "\n";
  std::lock_guard<std::mutex> locker(_tilemapLocker);
  _tilemap.SetSize(x, y);
}

void API::HandleBCT(std::stringstream &ss)
{
  int x;
  int y;
  int q0;
  int q1;
  int q2;
  int q3;
  int q4;
  int q5;
  int q6;

  if (!(ss >> x >> y >> q0 >> q1 >> q2 >> q3 >> q4 >> q5 >> q6))
    throw std::runtime_error(
      "Error: invalid bct params, Function: HandleBCT, File: API.cpp");
  // //std::cout
  //   << "tile (" << x << "," << y << ") resources: " << q0 << "," << q1 <<
  //   ","
  //   << q2 << "," << q3 << "," << q4 << "," << q5 << "," << q6 << "\n";
  std::lock_guard<std::mutex> locker(_tilemapLocker);
  _tilemap.SetTileInventory(x, y, q0, q1, q2, q3, q4, q5, q6);
}

void API::HandleMCT()
{
  // std::cout << "map content (bct *): Handled line-by-line\n";
}

void API::HandleTNA(std::stringstream &ss)
{
  std::string N;
  std::vector<Trantor> teamTmp;

  if (!(ss >> N))
    throw std::runtime_error(
      "Error: invalid tna params, Function: HandleTNA, File: API.cpp");
  // std::cout << "team name: " << N << "\n";
  std::lock_guard<std::mutex> lockerTeam(_teamsLocker);
  _teams[N] = teamTmp;
  std::lock_guard<std::mutex> lockerName(_allTeamNameLocker);
  for (std::string &teamName: _allTeamName)
    if (teamName == N)
      return;
  _allTeamName.push_back(N);
}

void API::HandlePNW(std::stringstream &ss)
{
  std::string nTmp;
  int X;
  int Y;
  int O;
  int L;
  std::string N;

  if (!(ss >> nTmp >> X >> Y >> O >> L >> N))
    throw std::runtime_error(
      "Error: invalid pnw params, Function: HandlePNW, File: API.cpp");

  if (nTmp[0] == '#')
    nTmp.erase(0, 1);
  std::cout
    << "new player: #" << std::stoi(nTmp) << " (" << X << "," << Y
    << ") facing " << O << " level " << L << " team " << N << "\n";
  Trantor trantorTmp(std::stoi(nTmp), X, Y, O, L);
  std::lock_guard<std::mutex> lockerTeam(_teamsLocker);
  _teams[N].push_back(trantorTmp);
}

void API::HandlePPO(std::stringstream &ss)
{
  std::string nTmp;
  int X;
  int Y;
  int O;

  if (!(ss >> nTmp >> X >> Y >> O))
    throw std::runtime_error(
      "Error: invalid ppo params, Function: HandlePPO, File: API.cpp");

  if (nTmp[0] == '#')
    nTmp.erase(0, 1);
  // std::cout
  //   << "player #" << std::stoi(nTmp) << " position: (" << X << "," << Y
  //   << ") facing " << O << "\n";
  std::lock_guard<std::mutex> lockerName(_allTeamNameLocker);
  std::lock_guard<std::mutex> lockerTeam(_teamsLocker);
  for (std::string &teamNameTmp: _allTeamName)
    for (Trantor &trantorTmp: _teams[teamNameTmp])
      if (trantorTmp.GetId() == std::stoi(nTmp))
        trantorTmp.SetPosition(X, Y, O);
}

void API::HandlePLV(std::stringstream &ss)
{
  std::string nTmp;
  int L;

  if (!(ss >> nTmp >> L))
    throw std::runtime_error(
      "Error: invalid plv params, Function: HandlePLV, File: API.cpp");

  if (nTmp[0] == '#')
    nTmp.erase(0, 1);
  // std::cout << "player #" << std::stoi(nTmp) << " level: " << L << "\n";
  std::lock_guard<std::mutex> lockerName(_allTeamNameLocker);
  std::lock_guard<std::mutex> lockerTeam(_teamsLocker);
  for (std::string &teamNameTmp: _allTeamName)
    for (Trantor &trantorTmp: _teams[teamNameTmp])
      if (trantorTmp.GetId() == std::stoi(nTmp))
        trantorTmp.SetLevel(L);
}

void API::HandlePIN(std::stringstream &ss)
{
  std::string nTmp;
  int x;
  int y;
  int q0;
  int q1;
  int q2;
  int q3;
  int q4;
  int q5;
  int q6;

  if (!(ss >> nTmp >> x >> y >> q0 >> q1 >> q2 >> q3 >> q4 >> q5 >> q6))
    throw std::runtime_error(
      "Error: invalid pin params, Function: HandlePIN, File: API.cpp");

  if (nTmp[0] == '#')
    nTmp.erase(0, 1);
  std::cout
    << "player #" << std::stoi(nTmp) << " inventory at (" << x << "," << y
    << "): " << q0 << "," << q1 << "," << q2 << "," << q3 << "," << q4 << ","
    << q5 << "," << q6 << "\n";
  std::lock_guard<std::mutex> lockerName(_allTeamNameLocker);
  std::lock_guard<std::mutex> lockerTeam(_teamsLocker);
  for (std::string &teamNameTmp: _allTeamName)
    for (Trantor &trantorTmp: _teams[teamNameTmp])
      if (trantorTmp.GetId() == std::stoi(nTmp)) {
        trantorTmp.SetPosition(x, y);
        trantorTmp.SetInventory(q0, q1, q2, q3, q4, q5, q6);
      }
}

void API::HandlePEX(std::stringstream &ss)
{
  std::string nTmp;
  int posX = -1;
  int posY = -1;
  int rotation = -1;
  int id = -1;

  if (!(ss >> nTmp))
    throw std::runtime_error(
      "Error: invalid pex params, Function: HandlePEX, File: API.cpp");

  if (nTmp[0] == '#')
    nTmp.erase(0, 1);
  // std::cout << "expulsion by player #" << std::stoi(nTmp) << "\n";
  std::lock_guard<std::mutex> lockerName(_allTeamNameLocker);
  std::lock_guard<std::mutex> lockerTeam(_teamsLocker);
  for (std::string &teamNameTmp: _allTeamName) {
    auto &team = _teams[teamNameTmp];
    for (auto it = team.begin(); it != team.end(); ++it)
      if (it->GetId() == std::stoi(nTmp)) {
        id = it->GetId();
        posX = it->GetPosition().first;
        posY = it->GetPosition().second;
        rotation = it->GetRotation();
        team.erase(it);
        break;
      }
  }
  for (std::string &teamNameTmp: _allTeamName) {
    auto &team = _teams[teamNameTmp];
    for (auto &it: team)
      if (it.GetPosition().first == posX && it.GetPosition().second == posY
          && it.GetId() != id)
        switch (rotation) {
          case 0:
            it.AddPosition(0, -1);
            break;
          case 1:
            it.AddPosition(1, -1);
            break;
          case 2:
            it.AddPosition(1, 0);
            break;
          case 3:
            it.AddPosition(1, 1);
            break;
          case 4:
            it.AddPosition(0, 1);
            break;
          case 5:
            it.AddPosition(-1, 1);
            break;
          case 6:
            it.AddPosition(-1, 0);
            break;
          case 7:
            it.AddPosition(-1, -1);
            break;
          default:
            return;
        }
  }
}

void API::HandlePBC(std::stringstream &ss)
{
  std::string nTmp;
  std::string msg;

  if (!(ss >> nTmp))
    throw std::runtime_error(
      "Error: invalid pbc params (missing id), Function: HandlePBC, File: "
      "API.cpp");

  std::getline(ss >> std::ws, msg);
  if (msg.empty())
    throw std::runtime_error(
      "Error: missing message in pbc, Function: HandlePBC, File: API.cpp");

  if (nTmp[0] == '#')
    nTmp.erase(0, 1);
  // std::cout << "broadcast from #" << std::stoi(nTmp) << ": " << msg << "\n";
  std::lock_guard<std::mutex> lockerName(_allTeamNameLocker);
  std::lock_guard<std::mutex> lockerTeam(_teamsLocker);
  for (std::string &teamNameTmp: _allTeamName)
    for (Trantor &trantorTmp: _teams[teamNameTmp])
      if (trantorTmp.GetId() == std::stoi(nTmp))
        trantorTmp.SetBroadcast(msg);
}

void API::HandlePIC(std::stringstream &ss)
{
  int X;
  int Y;
  int L;

  if (!(ss >> X >> Y >> L))
    throw std::runtime_error(
      "Error: invalid pic params, Function: HandlePIC, File: API.cpp");
  // std::cout
  //   << "start incantation at (" << X << "," << Y << ") level " << L
  //   << " by players:";

  Incantation tmp(L, X, Y);
  std::vector<std::string> nList;
  std::string tmpStr;
  while (ss >> tmpStr) {
    if (tmpStr[0] == '#')
      tmpStr.erase(0, 1);
    tmp.AddMember(std::stoi(tmpStr));
    nList.push_back(tmpStr);
  }
  {
    std::lock_guard<std::mutex> locker(_incantationListLocker);
    _incantationList.push_back(tmp);
  }
  // std::cout << "\n";
}

void API::HandlePIE(std::stringstream &ss)
{
  int X;
  int Y;
  std::string R;

  if (!(ss >> X >> Y >> R))
    throw std::runtime_error(
      "Error: invalid pie params, Function: HandlePIE, File: API.cpp");
  // std::cout
  //   << "end incantation at (" << X << "," << Y << "): result = " << R <<
  //   "\n";
  std::lock_guard<std::mutex> locker(_incantationListLocker);
  for (auto &incant: _incantationList)
    if (incant.GetPosition().first == X && incant.GetPosition().second == Y)
      incant.SetState(R);
}

void API::HandlePFK(std::stringstream &ss)
{
  std::string nTmp;

  if (!(ss >> nTmp))
    throw std::runtime_error(
      "Error: invalid pfk params, Function: HandlePFK, File: API.cpp");

  if (nTmp[0] == '#')
    nTmp.erase(0, 1);
  // std::cout << "egg laiying by player #" << std::stoi(nTmp) << "\n";
  std::lock_guard<std::mutex> lockerName(_allTeamNameLocker);
  std::lock_guard<std::mutex> lockerTeam(_teamsLocker);
  for (std::string &teamNameTmp: _allTeamName)
    for (Trantor &trantorTmp: _teams[teamNameTmp])
      if (trantorTmp.GetId() == std::stoi(nTmp))
        trantorTmp.IsTrantorLaying(true);
}

void API::HandlePDR(std::stringstream &ss)
{
  std::string nTmp;
  int i;

  if (!(ss >> nTmp >> i))
    throw std::runtime_error(
      "Error: invalid pdr params, Function: HandlePDR, File: API.cpp");

  if (nTmp[0] == '#')
    nTmp.erase(0, 1);
  // std::cout
  //   << "player #" << std::stoi(nTmp) << " dropped resource " << i << "\n";
  std::lock_guard<std::mutex> lockerName(_allTeamNameLocker);
  std::lock_guard<std::mutex> lockerTeam(_teamsLocker);
  for (std::string &teamNameTmp: _allTeamName)
    for (Trantor &trantorTmp: _teams[teamNameTmp])
      if (trantorTmp.GetId() == std::stoi(nTmp)) {
        trantorTmp.AddToInventory(static_cast<Item>(i), -1);
        std::lock_guard<std::mutex> locker(_tilemapLocker);
        _tilemap.AddToInventory(
          trantorTmp.GetPosition().first,
          trantorTmp.GetPosition().second,
          static_cast<Item>(i),
          1);
      }
}

void API::HandlePGT(std::stringstream &ss)
{
  std::string nTmp;
  int i;

  if (!(ss >> nTmp >> i))
    throw std::runtime_error(
      "Error: invalid pgt params, Function: HandlePGT, File: API.cpp");

  if (nTmp[0] == '#')
    nTmp.erase(0, 1);
  // std::cout
  //   << "player #" << std::stoi(nTmp) << " collected resource " << i << "\n";
  std::lock_guard<std::mutex> lockerName(_allTeamNameLocker);
  std::lock_guard<std::mutex> lockerTeam(_teamsLocker);
  for (std::string &teamNameTmp: _allTeamName)
    for (Trantor &trantorTmp: _teams[teamNameTmp])
      if (trantorTmp.GetId() == std::stoi(nTmp)) {
        trantorTmp.AddToInventory(static_cast<Item>(i), 1);
        std::lock_guard<std::mutex> locker(_tilemapLocker);
        _tilemap.AddToInventory(
          trantorTmp.GetPosition().first,
          trantorTmp.GetPosition().second,
          static_cast<Item>(i),
          -1);
      }
}

void API::HandlePDI(std::stringstream &ss)
{
  std::string nTmp;

  if (!(ss >> nTmp))
    throw std::runtime_error(
      "Error: invalid pdi params, Function: HandlePDI, File: API.cpp");

  if (nTmp[0] == '#')
    nTmp.erase(0, 1);
  // std::cout << "player #" << std::stoi(nTmp) << " died\n";
  std::lock_guard<std::mutex> lockerName(_allTeamNameLocker);
  std::lock_guard<std::mutex> lockerTeam(_teamsLocker);
  for (std::string &teamNameTmp: _allTeamName)
    for (size_t i = 0; i < _teams[teamNameTmp].size(); i++)
      if (_teams[teamNameTmp][i].GetId() == std::stoi(nTmp))
        _teams[teamNameTmp].erase(_teams[teamNameTmp].begin() + i);
}

void API::HandleENW(std::stringstream &ss)
{
  std::string eTmp;
  std::string nTmp;
  int X;
  int Y;

  if (!(ss >> eTmp >> nTmp >> X >> Y))
    throw std::runtime_error(
      "Error: invalid enw params, Function: HandleENW, File: API.cpp");

  if (eTmp[0] == '#')
    eTmp.erase(0, 1);
  if (nTmp[0] == '#')
    nTmp.erase(0, 1);
  // std::cout
  //   << "egg #" << std::stoi(eTmp) << " laid by player #" << std::stoi(nTmp)
  //   << " at (" << X << "," << Y << ")\n";
  std::lock_guard<std::mutex> lockerName(_allTeamNameLocker);
  std::lock_guard<std::mutex> lockerTeam(_teamsLocker);
  for (std::string &teamNameTmp: _allTeamName)
    for (Trantor &trantorTmp: _teams[teamNameTmp])
      if (trantorTmp.GetId() == std::stoi(nTmp))
        AddEgg(std::stoi(eTmp), X, Y);
}

void API::HandleEBO(std::stringstream &ss)
{
  std::string eTmp;

  if (!(ss >> eTmp))
    throw std::runtime_error(
      "Error: invalid ebo params, Function: HandleEBO, File: API.cpp");

  if (eTmp[0] == '#')
    eTmp.erase(0, 1);
  std::cout << "egg #" << eTmp << " connection\n";
  DeleteEgg(std::stoi(eTmp));
}

void API::HandleEDI(std::stringstream &ss)
{
  std::string eTmp;

  if (!(ss >> eTmp))
    throw std::runtime_error(
      "Error: invalid edi params, Function: HandleEDI, File: API.cpp");

  if (eTmp[0] == '#')
    eTmp.erase(0, 1);
  // std::cout << "egg #" << eTmp << " died\n";
  DeleteEgg(std::stoi(eTmp));
}

void API::HandleSGT(std::stringstream &ss)
{
  int T;

  if (!(ss >> T))
    throw std::runtime_error(
      "Error: invalid sgt params, Function: HandleSGT, File: API.cpp");
  // std::cout << "server time unit: " << T << "\n";
  std::lock_guard<std::mutex> locker(_timeUnitLocker);
  _timeUnit = T;
}

void API::HandleSST(std::stringstream &ss)
{
  short T;

  if (!(ss >> T))
    throw std::runtime_error(
      "Error: invalid sst params, Function: HandleSST, File: API.cpp");
  // std::cout << "server time unit changed to: " << T << "\n";
  std::lock_guard<std::mutex> locker(_timeUnitLocker);
  _timeUnit = T;
}

void API::HandleSEG(std::stringstream &ss)
{
  std::string N;

  if (!(ss >> N))
    throw std::runtime_error(
      "Error: invalid seg params, Function: HandleSEG, File: API.cpp");
  // std::cout << "end of game, winning team: " << N << "\n";
  std::lock_guard<std::mutex> locker(_winnerLocker);
  _winner = N;
}

void API::HandleSMG(std::stringstream &ss)
{
  std::string msg;

  std::getline(ss >> std::ws, msg);
  if (msg.empty())
    throw std::runtime_error(
      "Error: missing message in smg, Function: HandleSMG, File: API.cpp");
  // std::cout << "server message: " << msg << "\n";
  std::lock_guard<std::mutex> locker(_serverMessageLocker);
  _serverMessage.push_back(msg);
}

void API::HandleSUC()
{
  // std::cout << "unknown command\n";
}

void API::HandleSBP()
{
  // std::cout << "bad command parameter\n";
}
