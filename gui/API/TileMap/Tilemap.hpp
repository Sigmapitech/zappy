#pragma once

#include "API/Inventory/Inventory.hpp"

#include <vector>

class Tile {
private:
  Inventory _inventory;
  std::pair<int, int> _position;  // First int = x, Second int = y

public:
  Tile() = default;
  ~Tile() = default;

  [[nodiscard]] Inventory GetInventory();
  void SetInventory(
    size_t food,
    size_t linemate,
    size_t deraumere,
    size_t sibur,
    size_t mendiane,
    size_t phiras,
    size_t thystame);
  void SetPosition(int x, int y);
  void AddToInventory(Item item, int quantity);
};

class Tilemap {
private:
  int _sizeX;
  int _sizeY;
  std::vector<std::vector<Tile>> _tilemap;

public:
  Tilemap() = default;
  ~Tilemap() = default;

  [[nodiscard]] std::vector<std::vector<Tile>> GetTilemap();
  void SetSize(int x, int y);
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
  void AddToInventory(int x, int y, Item item, int quantity);
};
