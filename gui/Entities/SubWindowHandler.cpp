#include "Entities/SubWindowHandler.hpp"
#include "API/API.hpp"
#include "Demeter/Demeter.hpp"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"

#include <iostream>
#include <memory>

void SubWindowHandler::Run(Dem::Demeter &d, const std::shared_ptr<API> &api)
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
    RunMenu(d);
  if (_ImGuiState == STATE_TEAM)
    RunTeam(d);
  if (_ImGuiState == STATE_OPTIONS)
    RunOption();

  _windowSize = ImGui::GetWindowSize();
  _windowPos = ImGui::GetWindowPos();
}

void SubWindowHandler::RunMenu(Dem::Demeter &d)
{
  ImGui::Begin(_windowName.c_str(), nullptr, _window_flags);

  ImGui::TextColored(
    _textColor, "Time: %lu seconds", d.GetTime().GetCurrent() / 1000);
  ImGui::Separator();

  ImGui::Dummy(ImVec2(0.0, ImGui::GetContentRegionAvail().y - 60));

  if (ImGui::Button("Team Data", ImVec2(-1, 0)))
    _ImGuiState = STATE_TEAM;

  if (ImGui::Button("Options", ImVec2(-1, 0)))
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

