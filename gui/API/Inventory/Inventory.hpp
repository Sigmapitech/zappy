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
  size_t _food = 0;
  size_t _linemate = 0;
  size_t _deraumere = 0;
  size_t _sibur = 0;
  size_t _mendiane = 0;
  size_t _phiras = 0;
  size_t _thystame = 0;

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

  /**
   * @brief Add the quantity to one item
   *
   * @param item
   * @param quantity
   */
  void AddToInventory(Item item, size_t quantity);
};
