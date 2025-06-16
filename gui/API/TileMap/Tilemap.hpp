#pragma once

#include "API/Inventory/Inventory.hpp"

class Tilemap {
private:
  Inventory inventory;
  std::pair<int, int> position;  // First int = x, Second int = y

public:
  Tilemap() = default;
  ~Tilemap() = default;
};
