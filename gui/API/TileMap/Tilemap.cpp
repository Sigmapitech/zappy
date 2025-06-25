#include "Tilemap.hpp"
#include "API/Inventory/Inventory.hpp"

std::vector<std::vector<Tile>> Tilemap::GetTilemap()
{
  return _tilemap;
}

std::pair<int, int> Tilemap::GetSize() const
{
  std::pair<int, int> position;
  position.first = _sizeX;
  position.second = _sizeY;
  return position;
}

void Tilemap::SetSize(int x, int y)
{
  _sizeX = x;
  _sizeY = y;

  Tile tmp;
  _tilemap.clear();
  _tilemap.resize(y);

  for (int i = 0; i < y; i++) {
    for (int j = 0; j < x; j++) {
      tmp.SetPosition(j, i);
      _tilemap[i].push_back(tmp);
    }
  }
}

void Tilemap::SetTileInventory(
  int x,
  int y,
  size_t food,
  size_t linemate,
  size_t deraumere,
  size_t sibur,
  size_t mendiane,
  size_t phiras,
  size_t thystame)
{
  _tilemap[y][x].SetInventory(
    food, linemate, deraumere, sibur, mendiane, phiras, thystame);
}

void Tilemap::AddToInventory(int x, int y, Item item, int quantity)
{
  _tilemap[y][x].AddToInventory(item, quantity);
}
