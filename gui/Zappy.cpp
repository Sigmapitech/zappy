#include <glm/trigonometric.hpp>
#include <memory>

#include "Demeter/Renderer/SDL2.hpp"
#include "Entities/E_CameraControler.hpp"
#include "Entities/E_Coms.hpp"
#include "Entities/E_Light.hpp"
#include "Entities/E_Mother.hpp"
#include "Zappy.hpp"

bool Zappy::Init(Args &params)
{
  std::unique_ptr<SDL2> sdl2 = std::make_unique<SDL2>();
  if (!sdl2->Init()) {
    Log::failed << "Failed to initialize SDL2!";
    return false;
  }
  demeter = std::make_unique<Dem::Demeter>();
  if (!demeter->Init(std::move(sdl2))) {
    Log::failed << "Failed to initialize Demeter!";
    return false;
  }

  // Register entities
  if (!demeter->AddEntity(std::make_shared<E_Coms>(params))->Init(*demeter)) {
    Log::failed << "Failed to initialize E_Coms!";
    return false;
  }
  if (!demeter->AddEntity(std::make_shared<E_Light>())->Init(*demeter))
    Log::failed << "Failed to initialize E_Light!";
  if (
    !demeter->AddEntity(std::make_shared<E_CameraControler>())->Init(*demeter))
    Log::failed << "Failed to initialize E_CameraControler!";
  if (!demeter->AddEntity(std::make_shared<E_Mother>())->Init(*demeter))
    Log::failed << "Failed to initialize E_Mother!";

  // camera
  demeter->camera.SetPosition({0.0, 1.0, 200.0});
  return true;
}

void Zappy::Run()
{
  Log::inf << "GUI started.";
  demeter->Run();
}
