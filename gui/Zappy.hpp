#pragma once

#include <memory>

#include "ArgsParser.hpp"
#include "Demeter/Demeter.hpp"

struct Zappy {
private:
  std::unique_ptr<Dem::Demeter> demeter;

public:
  Zappy(Args &params);
  ~Zappy() = default;

  void Run();
};
