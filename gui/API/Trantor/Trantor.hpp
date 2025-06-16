#pragma once

#include "API/Inventory/Inventory.hpp"

class Trantor {
private:
  Inventory _inventory;
  int _food;
  int _id;
  std::pair<int, int> _position;  // First int = x, Second int = y

public:
  Trantor() = default;
  ~Trantor() = default;

  [[nodiscard]] int GetId() const;
};
