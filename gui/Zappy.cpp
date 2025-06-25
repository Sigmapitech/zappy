#include <glm/trigonometric.hpp>
#include <memory>

#include "Demeter/Renderer/SDL2.hpp"
#include "Entities/E_CameraControler.hpp"
#include "Entities/E_Coms.hpp"
#include "Entities/E_Light.hpp"
#include "Entities/E_Mother.hpp"
#include "Zappy.hpp"

Zappy::Zappy(Args &params)
{
  std::unique_ptr<SDL2> sdl2 = std::make_unique<SDL2>();
  demeter = std::make_unique<Dem::Demeter>(std::move(sdl2));

  // Register entities
  demeter->AddEntity(std::make_shared<E_Coms>(params));
  demeter->AddEntity(std::make_shared<E_Light>());

  demeter->AddEntity(std::make_shared<E_CameraControler>(*demeter));
  demeter->AddEntity(std::make_shared<E_Mother>(*demeter));

  // camera
  demeter->camera.position = glm::vec3(0.0, 1.0, 200.0);
}

void Zappy::Run()
{
  Log::inf << "GUI started.";
  demeter->Run();
}
