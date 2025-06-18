#include "Tile.hpp"

Inventory Tile::GetInventory()
{
  return _inventory;
}

void Tile::SetInventory(
  size_t food,
  size_t linemate,
  size_t deraumere,
  size_t sibur,
  size_t mendiane,
  size_t phiras,
  size_t thystame)
{
  _inventory.SetInventory(
    food, linemate, deraumere, sibur, mendiane, phiras, thystame);
}

void Tile::SetPosition(int x, int y)
{
  _position.first = x;
  _position.second = y;
}

void Tile::AddToInventory(Item item, int quantity)
{
  _inventory.AddToInventory(item, quantity);
}
