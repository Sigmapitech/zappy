#pragma once

#include <memory>
#include <vector>

#include "Renderer/Object3D.hpp"

#include "Entity.hpp"

namespace Dem {

  struct Demeter {
  private:
    std::vector<std::shared_ptr<IEntity>> entityPool;
    std::vector<std::shared_ptr<Texture>> texturePool;
    std::vector<std::shared_ptr<Object3D>> objectPool;
    std::unique_ptr<ShaderProgram> shader;
    Camera camera;
    std::unique_ptr<SDL2> sdl2;
    Uint64 lastTime;
    Uint64 currentTime;
    double deltaTime;
    bool isRunning;
    bool glDebug;
    static void DebugCallback(
      GLenum source,
      GLenum type,
      GLuint id,
      GLenum severity,
      GLsizei length,
      const GLchar *message,
      const void *userParam);

    void Update();
    void Draw();

  public:
    Demeter(std::unique_ptr<SDL2> renderer, bool debug = false);
    ~Demeter();

    void Run();
  };

}  // namespace Dem
