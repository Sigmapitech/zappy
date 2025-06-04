#pragma once

#include "../Inventory/Inventory.hpp"

class Trantor {
private:
  Inventory inventory;
  int level;
  int food;

public:
  /**
   * @brief Construct a new Trantor object
   */
  Trantor();
  /**
   * @brief Construct a new Trantor object
   *
   * @param inventory Contain a union with all item the Trantor can have.
   * @param level_arg Contain the level of the Trantor.
   * @param food Contain the number of food.
   */
  Trantor(Inventory inventory, int level_arg, int food);
  ~Trantor() = default;
};
