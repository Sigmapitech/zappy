#include <memory>

#include "Demeter/Renderer/SDL2.hpp"
#include "Zappy.hpp"

Zappy::Zappy(std::shared_ptr<API> api) : _api(api)
{
  std::unique_ptr<SDL2> sdl2 = std::make_unique<SDL2>();
  demeter = std::make_unique<Dem::Demeter>(std::move(sdl2));
}

void Zappy::Run()
{
  demeter->Run();
}
