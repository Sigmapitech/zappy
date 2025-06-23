#pragma once

#include <vector>
#include "API/Inventory/Inventory.hpp"

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
  Trantor(int id, int x, int y, int orientation, int level);
  ~Trantor() = default;

  [[nodiscard]] int GetId() const;
  [[nodiscard]] int GetRotation() const;

  void SetPosition(int x, int y, int orientation);
  void SetPosition(int x, int y);
  void AddPosition(int x, int y);
  [[nodiscard]] std::pair<int, int> GetPosition();

  void SetLevel(int level);
  void SetInventory(
    size_t q0,
    size_t q1,
    size_t q2,
    size_t q3,
    size_t q4,
    size_t q5,
    size_t q6);
  void AddToInventory(Item item, int quantity);
  void SetBroadcast(std::string &message);

  void IsTrantorLaying(bool state);
};
