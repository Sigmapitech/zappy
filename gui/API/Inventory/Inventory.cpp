#include "Inventory.hpp"
#include <map>

Inventory::Inventory(
  size_t food,
  size_t linemate,
  size_t deraumere,
  size_t sibur,
  size_t mendiane,
  size_t phiras,
  size_t thystame)
  : _food(food),
    _linemate(linemate),
    _deraumere(deraumere),
    _sibur(sibur),
    _mendiane(mendiane),
    _phiras(phiras),
    _thystame(thystame)
{
}

std::map<std::string, size_t> Inventory::GetInventory() const
{
  std::map<std::string, size_t> tmp;

  tmp["food"] = _food;
  tmp["linemate"] = _linemate;
  tmp["deraumere"] = _deraumere;
  tmp["mendiane"] = _mendiane;
  tmp["phiras"] = _phiras;
  tmp["sibur"] = _sibur;
  tmp["thystame"] = _thystame;
  return tmp;
}

void Inventory::SetInventory(
  size_t food,
  size_t linemate,
  size_t deraumere,
  size_t sibur,
  size_t mendiane,
  size_t phiras,
  size_t thystame)
{
  _food = food;
  _linemate = linemate;
  _deraumere = deraumere;
  _sibur = sibur;
  _mendiane = mendiane;
  _phiras = phiras;
  _thystame = thystame;
}

void Inventory::SetInventory(Item item, size_t quantity)
{
  switch (item) {
    case Item::FOOD:
      _food = quantity;
      break;
    case Item::LINEMATE:
      _linemate = quantity;
      break;
    case Item::DERAUMERE:
      _deraumere = quantity;
      break;
    case Item::SIBUR:
      _sibur = quantity;
      break;
    case Item::MENDIANE:
      _mendiane = quantity;
      break;
    case Item::PHIRAS:
      _phiras = quantity;
      break;
    case Item::THYSTAME:
      _thystame = quantity;
      break;
  }
}

void Inventory::AddToInventory(Item item, size_t quantity)
{
  switch (item) {
    case Item::FOOD:
      _food += quantity;
      break;
    case Item::LINEMATE:
      _linemate += quantity;
      break;
    case Item::DERAUMERE:
      _deraumere += quantity;
      break;
    case Item::SIBUR:
      _sibur += quantity;
      break;
    case Item::MENDIANE:
      _mendiane += quantity;
      break;
    case Item::PHIRAS:
      _phiras += quantity;
      break;
    case Item::THYSTAME:
      _thystame += quantity;
      break;
  }
}
