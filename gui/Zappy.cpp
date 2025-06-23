#include <memory>

#include "Demeter/Renderer/SDL2.hpp"
#include "Entities/E_Coms.hpp"
#include "Zappy.hpp"

Zappy::Zappy(Args &params)
{
  std::unique_ptr<SDL2> sdl2 = std::make_unique<SDL2>();
  demeter = std::make_unique<Dem::Demeter>(std::move(sdl2));

  // Register entities
  demeter->AddEntity(std::make_shared<E_Coms>(params));
}

void Zappy::Run()
{
  Log::info << "GUI started.";
  demeter->Run();
}
