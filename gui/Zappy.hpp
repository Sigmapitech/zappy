#pragma once

#include <memory>

#include "API/API.hpp"
#include "Demeter/Demeter.hpp"

struct Zappy {
private:
  std::unique_ptr<Dem::Demeter> demeter;
  std::shared_ptr<API> _api;

public:
  Zappy(std::shared_ptr<API> api);
  ~Zappy() = default;

  void Run();
};
