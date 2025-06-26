#pragma once

#include "API/Inventory/Inventory.hpp"
#include "Tile.hpp"

#include <vector>

class Tilemap {
private:
  int _sizeX = 0;
  int _sizeY = 0;
  std::vector<std::vector<Tile>> _tilemap;

public:
  Tilemap() = default;
  ~Tilemap() = default;

  /**
   * @brief Get the Size object
   *
   * @return std::pair<x, y>
   */
  std::pair<int, int> GetSize() const;

  /**
   * @brief Get the Tilemap
   *
   * @return std::vector<std::vector<Tile>>
   */
  [[nodiscard]] std::vector<std::vector<Tile>> GetTilemap();

  /**
   * @brief Set the Size of the tilemap
   *
   * @param x  Contain the width of the tilemap
   * @param y  Contain the height of the tilemap
   */
  void SetSize(int x, int y);

  /**
   * @brief Set the inventory of one tile
   *
   * @param x           Contain position x of the tile
   * @param y           Contain position y of the tile
   * @param food        Contain number of food of the tile
   * @param linemate    Contain number of linemate of the tile
   * @param deraumere   Contain number of deraumere of the tile
   * @param sibur       Contain number of sibur of the tile
   * @param mendiane    Contain number of mendiane of the tile
   * @param phiras      Contain number of phiras of the tile
   * @param thystame    Contain number of thystame of the tile
   */
  void SetTileInventory(
    int x,
    int y,
    size_t food,
    size_t linemate,
    size_t deraumere,
    size_t sibur,
    size_t mendiane,
    size_t phiras,
    size_t thystame);

  /**
   * @brief Add a quantity of item to an element
   *
   * @param x         Contain position x of the tile
   * @param y         Contain position y of the tile
   * @param item      Contain the id of the item
   * @param quantity  Contain the quantity of the item to add
   */
  void AddToInventory(int x, int y, Item item, int quantity);

  [[nodiscard]] Inventory GetTileInventory(int x, int y);
};
