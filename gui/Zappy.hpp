#pragma once

#include <memory>

#include "ArgsParser.hpp"
#include "Demeter/Demeter.hpp"

struct Zappy {
private:
  std::unique_ptr<Dem::Demeter> demeter;

public:
  Zappy() = default;
  ~Zappy() = default;
  [[nodiscard]] bool Init(Args &params);

  void Run();
};
