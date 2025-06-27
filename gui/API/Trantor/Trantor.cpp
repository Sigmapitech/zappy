#include "Trantor.hpp"

Trantor::Trantor(int id, int x, int y, int orientation, int level)
  : _id(id), _orientation(orientation), _level(level)
{
  _position.first = x;
  _position.second = y;
}

int Trantor::GetId() const
{
  return _id;
}

int Trantor::GetLevel() const
{
  return _level;
}

void Trantor::SetPosition(int x, int y, int orientation)
{
  _position.first = x;
  _position.second = y;
  _orientation = orientation;
}

void Trantor::SetPosition(int x, int y)
{
  _position.first = x;
  _position.second = y;
}

std::pair<int, int> Trantor::GetPosition()
{
  return _position;
}

void Trantor::SetLevel(int level)
{
  _level = level;
}

void Trantor::SetInventory(
  size_t q0,
  size_t q1,
  size_t q2,
  size_t q3,
  size_t q4,
  size_t q5,
  size_t q6)
{
  _inventory.SetInventory(q0, q1, q2, q3, q4, q5, q6);
}

void Trantor::AddToInventory(Item item, int quantity)
{
  _inventory.AddToInventory(item, quantity);
}

void Trantor::SetBroadcast(std::string &message)
{
  _broadcast.push_back(message);
}

void Trantor::IsTrantorLaying(bool state)
{
  _isLaying = state;
}

int Trantor::GetRotation() const
{
  return _orientation;
}

void Trantor::AddPosition(int x, int y)
{
  _position.first += x;
  _position.second += y;
}
