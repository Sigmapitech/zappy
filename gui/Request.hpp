#pragma once

#include <cstdint>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <vector>

enum cmd : uint8_t {
  MAP_SIZE,
  TILE_CONTENT,
  MAP_CONTENT,
  TEAM_NAMES,
  PLAYER_POSITION,
  PLAYER_LEVEL,
  PLAYER_INVENTORY,
  TIME_UNIT,
  TIME_UNIT_MODIFICATION
};

static const std::vector<std::pair<cmd, std::string>> REQUEST = {
  {MAP_SIZE, "msz"},
  {TILE_CONTENT, "bct %lu %lu"},
  {MAP_CONTENT, "mct"},  // content of the map (all the tiles)
  {TEAM_NAMES, "tna"},
  {PLAYER_POSITION, "ppo #%i"},
  {PLAYER_LEVEL, "plv #%i"},
  {PLAYER_INVENTORY, "pin #%i"},
  {TIME_UNIT, "sgt"},
  {TIME_UNIT_MODIFICATION, "sst %d"}};
