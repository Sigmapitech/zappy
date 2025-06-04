#pragma once

#include <vector>
#include "../Inventory/Inventory.hpp"

class Tilemap {
private:
  std::vector<std::vector<Inventory>> _tilemap;

public:
  /**
   * @brief Construct a new Tilemap object
   *
   * @param height
   * @param width
   */
  Tilemap(size_t height, size_t width);
  ~Tilemap() = default;
};
