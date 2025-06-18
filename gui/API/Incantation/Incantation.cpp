#include "Incantation.hpp"

Incantation::Incantation(int level, int x, int y) : _level(level)
{
  _position.first = x;
  _position.second = y;
}

void Incantation::AddMember(int id)
{
  _member.push_back(id);
}

std::pair<int, int> Incantation::GetPosition()
{
  return _position;
}

void Incantation::SetState(std::string &state)
{
  _state = state;
}
