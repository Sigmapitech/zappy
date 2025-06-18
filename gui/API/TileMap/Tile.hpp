#include "API/Inventory/Inventory.hpp"

class Tile {
private:
  Inventory _inventory;
  std::pair<int, int> _position;  // First int = x, Second int = y

public:
  Tile() = default;
  ~Tile() = default;

  /**
   * @brief Get the Inventory
   *
   * @return Inventory
   */
  [[nodiscard]] Inventory GetInventory();

  /**
   * @brief Set the Inventory of the tile
   *
   * @param food      Contain the number of food to set
   * @param linemate  Contain the number of linemate to set
   * @param deraumere Contain the number of deraumere to set
   * @param sibur     Contain the number of sibur to set
   * @param mendiane  Contain the number of mendiane to set
   * @param phiras    Contain the number of phiras to set
   * @param thystame  Contain the number of thystame to set
   */
  void SetInventory(
    size_t food,
    size_t linemate,
    size_t deraumere,
    size_t sibur,
    size_t mendiane,
    size_t phiras,
    size_t thystame);

  /**
   * @brief Set the Position of the tile
   *
   * @param x  Contain the position x of the tile
   * @param y  Contain the position y of the tile
   */
  void SetPosition(int x, int y);

  /**
   * @brief Add item to the inventory
   *
   * @param item        Contain the id of the item
   * @param quantity    Contain the quantity of item to add
   */
  void AddToInventory(Item item, int quantity);
};
