#include "Tilemap.hpp"
#include <cstddef>
#include "gui/Inventory/Inventory.hpp"

Tilemap::Tilemap(size_t height, size_t width)
  : _tilemap(height, std::vector<Inventory>(width))
{
}
