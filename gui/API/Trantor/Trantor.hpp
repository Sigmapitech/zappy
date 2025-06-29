#pragma once

#include "API/Inventory/Inventory.hpp"

#include <vector>

class Trantor {
private:
  Inventory _inventory;
  int _food;
  int _id;
  int _orientation;
  int _level;
  std::pair<int, int> _position;  // First int = x, Second int = y
  std::vector<std::string> _broadcast;
  bool _isLaying;

public:
  Trantor() = default;
  /**
   * @brief Construct a new Trantor object
   *
   * @param id            Contain the id of the trantor
   * @param x             Contain the x position of the trantor
   * @param y             Contain the y position of the trantor
   * @param orientation   Contain the orientation of the trantor
   * @param level         Contain the level of the trantor
   */
  Trantor(int id, int x, int y, int orientation, int level);
  ~Trantor() = default;

  /**
   * @brief Get the Id of the trantor
   *
   * @return int
   */
  [[nodiscard]] int GetId() const;

  /**
   * @brief Get the Rotation of the trantor
   *
   * @return int [0-7]
   */
  [[nodiscard]] int GetRotation() const;

  /**
   * @brief Get the Level
   *
   * @return int
   */
  [[nodiscard]] int GetLevel() const;

  /**
   * @brief Get the Inventory
   *
   * @return std::map<std::string, size_t>
   */
  [[nodiscard]] std::map<std::string, size_t> GetInventory();

  /**
   * @brief Set the Position object
   *
   * @param x             Contain the x position of the trantor
   * @param y             Contain the y position of the trantor
   * @param orientation   Contain the orientation of the trantor
   */
  void SetPosition(int x, int y, int orientation);

  /**
   * @brief Set the Position object
   *
   * @param x             Contain the x position of the trantor
   * @param y             Contain the y position of the trantor
   */
  void SetPosition(int x, int y);

  /**
   * @brief Add x and y to the x and y position
   *
   * @param x             Contain the x that will be add to x position
   * @param y             Contain the y that will be add to y position
   */
  void AddPosition(int x, int y);

  /**
   * @brief Get the Position of the trantor
   *
   * @return std::pair<int, int>
   */
  [[nodiscard]] std::pair<int, int> GetPosition();

  /**
   * @brief Set the Level of the trantor
   *
   * @param level
   */
  void SetLevel(int level);

  /**
   * @brief Set the Inventory of the trantor
   *
   * @param q0            Contain the food quantity to set the inventory
   * @param q1            Contain the linemate quantity to set the inventory
   * @param q2            Contain the deraumere quantity to set the inventory
   * @param q3            Contain the sibur quantity to set the inventory
   * @param q4            Contain the mendiane quantity to set the inventory
   * @param q5            Contain the phiras quantity to set the inventory
   * @param q6            Contain the thystame quantity to set the inventory
   */
  void SetInventory(
    size_t q0,
    size_t q1,
    size_t q2,
    size_t q3,
    size_t q4,
    size_t q5,
    size_t q6);

  /**
   * @brief Add items to the the Inventory of the trantor
   *
   * @param q0            Contain the food quantity to add to the inventory
   * @param q1            Contain the linemate quantity to add to the inventory
   * @param q2            Contain the deraumere quantity to add to the
   * inventory
   * @param q3            Contain the sibur quantity to add to the inventory
   * @param q4            Contain the mendiane quantity to add to the inventory
   * @param q5            Contain the phiras quantity to add to the inventory
   * @param q6            Contain the thystame quantity to add to the inventory
   */
  void AddToInventory(Item item, int quantity);

  /**
   * @brief Set the Broadcast object
   *
   * @param message       Contain the message the trantor is screaming
   */
  void SetBroadcast(std::string &message);

  /**
   * @brief Set the laying state of the trantor
   *
   * @param state         Contain the state is the trantor is laying or not
   */
  void IsTrantorLaying(bool state);
};
