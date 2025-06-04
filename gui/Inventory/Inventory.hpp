#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

enum ITEM : uint8_t {
  LINEMATE,
  DERAUMERE,
  SIBUR,
  MENDIANE,
  PHIRAS,
  THYSTAME,
  FOOD
};

union Inventory {
  struct {
    size_t linemate;
    size_t deraumere;
    size_t sibur;
    size_t mendiane;
    size_t phiras;
    size_t thystame;
    size_t food;
  };

  std::array<size_t, 7> quantities;
};
