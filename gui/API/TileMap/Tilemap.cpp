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

Inventory Tilemap::GetTileInventory(int x, int y)
{
  return _tilemap[y][x].GetInventory();
}

int Tilemap::GetItemQuantity(Item id)
{
  int quantity = 0;

  for (auto &tile: _tilemap) {
    for (auto t: tile) {
      switch (id) {
        case Item::FOOD:
          quantity += t.GetInventory().GetInventory()["food"];
          break;
        case Item::LINEMATE:
          quantity += t.GetInventory().GetInventory()["linemate"];
          break;
        case Item::DERAUMERE:
          quantity += t.GetInventory().GetInventory()["deraumere"];
          break;
        case Item::SIBUR:
          quantity += t.GetInventory().GetInventory()["sibur"];
          break;
        case Item::MENDIANE:
          quantity += t.GetInventory().GetInventory()["mendiane"];
          break;
        case Item::PHIRAS:
          quantity += t.GetInventory().GetInventory()["phiras"];
          break;
        case Item::THYSTAME:
          quantity += t.GetInventory().GetInventory()["thystame"];
          break;
        default:
          continue;
      }
    }
  }
  return quantity;
}
