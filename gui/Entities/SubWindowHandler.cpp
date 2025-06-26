#include "Entities/SubWindowHandler.hpp"
#include "API/API.hpp"
#include "Demeter/Demeter.hpp"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"

#include <array>
#include <iostream>
#include <memory>
#include <string>

void SubWindowHandler::Run(
  Dem::Demeter &d,
  const std::shared_ptr<API> &api,
  std::array<std::string, 256> &eventArray,
  size_t eventIndex,
  size_t eventCount)
{
  _api = api;

  d.SetIsImGuiWindowCreated(true);
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  if (_firstFrame) {
    ImGui::SetNextWindowSize(_windowSize);
    ImGui::SetNextWindowPos(_windowPos);
    _firstFrame = false;
  }
  if (_ImGuiState == STATE_MENU)
    RunMenu(d, eventArray, eventIndex, eventCount);
  if (_ImGuiState == STATE_TEAM)
    RunTeam(d);
  if (_ImGuiState == STATE_OPTIONS)
    RunOption();

  _windowSize = ImGui::GetWindowSize();
  _windowPos = ImGui::GetWindowPos();
}

void SubWindowHandler::RunMenu(
  Dem::Demeter &d,
  std::array<std::string, 256> &eventArray,
  size_t eventIndex,
  size_t eventCount)
{
  ImGui::Begin(_windowName.c_str(), nullptr, _window_flags);

  ImGui::TextColored(
    _textColor, "Time: %lu seconds", d.GetTime().GetCurrent() / 1000);
  ImGui::Separator();

  ImVec2 startPos = ImGui::GetCursorScreenPos();

  // Draw map Content
  ImGui::BeginGroup();

  ImGui::TextColored(_textColor, " MAP CONTENT :");

  ImGui::TextColored(
    _textColor,
    "\ttilemap size: width = %d, height = %d",
    _api->GetTilemap().GetSize().first,
    _api->GetTilemap().GetSize().second);
  ImGui::TextColored(
    _textColor,
    "\tquantity of food : %d",
    _api->GetTilemap().GetItemQuantity(Item::FOOD));
  ImGui::TextColored(
    _textColor,
    "\tquantity of linemate : %d",
    _api->GetTilemap().GetItemQuantity(Item::LINEMATE));
  ImGui::TextColored(
    _textColor,
    "\tquantity of deraumere : %d",
    _api->GetTilemap().GetItemQuantity(Item::DERAUMERE));
  ImGui::TextColored(
    _textColor,
    "\tquantity of mendiane : %d",
    _api->GetTilemap().GetItemQuantity(Item::MENDIANE));
  ImGui::TextColored(
    _textColor,
    "\tquantity of sibur : %d",
    _api->GetTilemap().GetItemQuantity(Item::SIBUR));
  ImGui::TextColored(
    _textColor,
    "\tquantity of phyras : %d",
    _api->GetTilemap().GetItemQuantity(Item::PHIRAS));
  ImGui::TextColored(
    _textColor,
    "\tquantity of thystame : %d",
    _api->GetTilemap().GetItemQuantity(Item::THYSTAME));
  ImGui::Unindent();

  ImGui::EndGroup();

  ImVec2 endPos = ImGui::GetCursorScreenPos();

  ImGui::GetWindowDrawList()->AddRect(
    startPos, endPos, IM_COL32(200, 200, 200, 255), 5.0);

  // Display the events
  ImGui::TextColored(_textColor, "EVENTS :");
  ImVec2 startPosEvent = ImGui::GetCursorScreenPos();
  ImGui::BeginGroup();
  float scrollableHeight = ImGui::GetContentRegionAvail().y - 60.0;
  ImGui::BeginChild(
    "Scrollable",
    ImVec2(ImGui::GetContentRegionAvail().x, scrollableHeight),
    true,
    ImGuiWindowFlags_AlwaysVerticalScrollbar
      | ImGuiWindowFlags_HorizontalScrollbar);
  for (size_t i = eventIndex; i < (eventCount + eventIndex); i++)
    ImGui::TextColored(_textColor, "%s", eventArray[i % eventCount].c_str());

  ImGui::EndChild();
  ImGui::EndGroup();
  ImVec2 endPosEvent = ImGui::GetCursorScreenPos();

  ImGui::GetWindowDrawList()->AddRect(
    startPosEvent, endPosEvent, IM_COL32(200, 200, 200, 255), 5.0);

  // Draw buttons at the bottom
  float footerHeight = ImGui::GetFrameHeight();
  float spacing = ImGui::GetStyle().ItemSpacing.y;
  float bottomPadding = 10.0;

  ImGui::SetCursorPosY(
    ImGui::GetWindowHeight() - footerHeight - spacing - bottomPadding);

  float totalWidth = ImGui::GetContentRegionAvail().x;
  float buttonWidth = (totalWidth - 10.0) / 2.0;

  if (ImGui::Button("Team Data", ImVec2(buttonWidth, 0)))
    _ImGuiState = STATE_TEAM;

  ImGui::SameLine();

  if (ImGui::Button("Options", ImVec2(buttonWidth, 0)))
    _ImGuiState = STATE_OPTIONS;

  ImGui::End();
}

void SubWindowHandler::RunTeam(Dem::Demeter &d)
{
  ImGui::Begin(_windowName.c_str(), nullptr, _window_flags);

  ImGui::TextColored(
    _textColor, "Time: %lu seconds", d.GetTime().GetCurrent() / 1000);
  ImGui::Separator();

  ImVec2 scrollSize = ImVec2(
    ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 30);
  ImGui::BeginChild(
    "ZoneScrollable",
    scrollSize,
    true,
    ImGuiWindowFlags_AlwaysVerticalScrollbar
      | ImGuiWindowFlags_HorizontalScrollbar);

  if (
    ImGui::BeginTable(
      "PlayerTable",
      4,
      ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders
        | ImGuiTableFlags_ScrollX)) {

    if (_showTableBg) {
      ImGui::TableSetBgColor(
        ImGuiTableBgTarget_RowBg0,
        ImGui::GetColorU32(ImVec4(0.86, 0.86, 0.86, 1.0)));
    } else {
      ImGui::TableSetBgColor(
        ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(1, 1, 1, 0)));
    }

    ImGui::TableSetupColumn("Player");
    ImGui::TableSetupColumn("Position");
    ImGui::TableSetupColumn("Action");
    ImGui::TableSetupColumn("Level");
    ImGui::TableHeadersRow();

    for (int i = 0; i < 100; i++) {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextColored(_textColor, "Player: %d", i);
      ImGui::TableSetColumnIndex(1);
      ImGui::TextColored(_textColor, "x = %d, y = %d", i, i * 2);
      ImGui::TableSetColumnIndex(2);
      std::string label = "Bouton##" + std::to_string(i);
      if (ImGui::Button(label.c_str()))
        std::cout << "\a";
      ImGui::TableSetColumnIndex(3);
      ImGui::TextColored(_textColor, "Lvl: %d", i);
    }
    ImGui::EndTable();
  }
  ImGui::EndChild();

  if (ImGui::Button("Back to Menu", ImVec2(-1, 0)))
    _ImGuiState = STATE_MENU;

  ImGui::End();
}

void SubWindowHandler::RunOption()
{
  ImGui::Begin(_windowName.c_str(), nullptr, _window_flags);

  ImGui::Text("Options");
  ImGui::Separator();

  ImGui::ColorEdit3("Text Color", (float *)&_textColor);
  ImGui::Checkbox("Show Table Background", &_showTableBg);
  ImGui::Checkbox("Debug Mode", &_debugMode);

  ImGui::Dummy(ImVec2(0.0, ImGui::GetContentRegionAvail().y - 30));

  if (ImGui::Button("Back to Menu", ImVec2(-1, 0)))
    _ImGuiState = STATE_MENU;

  ImGui::End();
}

