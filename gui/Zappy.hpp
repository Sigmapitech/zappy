#pragma once

#include <memory>

#include "Demeter/Demeter.hpp"

struct Zappy {
private:
  std::unique_ptr<Dem::Demeter> demeter;

public:
  Zappy();
  ~Zappy() = default;

  void Run();
};
