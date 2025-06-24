#pragma once

#include <memory>

#include "API/API.hpp"
#include "ArgsParser.hpp"
#include "Demeter/Entity.hpp"
#include "Network/Network.hpp"
#include "logging/Logger.hpp"

class E_Coms : public Dem::IEntity {
public:
  std::shared_ptr<API> api = nullptr;  // NOLINT

private:
  Network network;

public:
  E_Coms(Args &params)
    : api(std::make_shared<API>()),
      network(params.GetPort(), params.GetHost(), api)
  {
    Log::info << "Network started.";
  }

  ~E_Coms() override
  {
    network.RequestStop();
  }

  bool Update(Dem::Demeter &d) override
  {
    (void)d;
    network.Run();
    return true;
  }

  bool Draw(Dem::Demeter &d) override
  {
    (void)d;
    return true;
  }
};
