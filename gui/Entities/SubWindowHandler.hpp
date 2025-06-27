#pragma once

#include <memory>
#include "API/API.hpp"
#include "Demeter/Demeter.hpp"
#include "imgui.h"

#define STATE_MENU 0
#define STATE_TEAM 1
#define STATE_OPTIONS 2

struct SubWindowHandler {
private:
  ImVec2 _windowSize = ImVec2(500, 400);
  ImVec2 _windowPos = ImVec2(10, 10);

  bool _firstFrame = true;

  ImGuiWindowFlags _window_flags = ImGuiWindowFlags_NoCollapse;
  std::string _windowName = "Game Interface";

  ImVec4 _textColor = ImVec4(1.0, 1.0, 1.0, 1.0);
  bool _showTableBg = true;
  bool _debugMode = false;

  int _ImGuiState = STATE_MENU;

  std::shared_ptr<API> _api;

public:
  SubWindowHandler() = default;

  void Run(
    Dem::Demeter &d,
    const std::shared_ptr<API> &api,
    std::array<std::string, 256> &eventArray,
    size_t eventIndex,
    size_t eventCount);
  void RunMenu(
    Dem::Demeter &d,
    std::array<std::string, 256> &eventArray,
    size_t eventIndex,
    size_t eventCount);
  void RunTeam(Dem::Demeter &d);
  void RunOption();
  void RunInventory(std::string team, int id);
};
