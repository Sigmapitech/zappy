#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>

enum class Item : uint8_t {
  FOOD,
  LINEMATE,
  DERAUMERE,
  SIBUR,
  MENDIANE,
  PHIRAS,
  THYSTAME
};

class Inventory {
private:
  size_t _food;
  size_t _linemate;
  size_t _deraumere;
  size_t _sibur;
  size_t _mendiane;
  size_t _phiras;
  size_t _thystame;

public:
  /**
   * @brief Construct a new Inventory
   */
  Inventory() = default;

  /**
   * @brief Construct a new Inventory object
   *
   * @param food
   * @param linemate
   * @param deraumere
   * @param sibur
   * @param mendiane
   * @param phiras
   * @param thystame
   */
  Inventory(
    size_t food,
    size_t linemate,
    size_t deraumere,
    size_t sibur,
    size_t mendiane,
    size_t phiras,
    size_t thystame);

  /**
   * @brief Destroy the Inventory object
   */
  ~Inventory() = default;

  /**
   * @brief Get the Inventory object
   *
   * @return std::map<std::string, size_t>
   */
  [[nodiscard]] std::map<std::string, size_t> GetInventory() const;

  /**
   * @brief Set the quantity of all object
   *
   * @param food
   * @param linemate
   * @param deraumere
   * @param sibur
   * @param mendiane
   * @param phiras
   * @param thystame
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
   * @brief Set the quantity of one item
   *
   * @param item
   * @param quantity
   */
  void SetInventory(Item item, size_t quantity);
};
