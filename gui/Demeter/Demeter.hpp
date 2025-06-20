#pragma once

#include <algorithm>
#include <memory>
#include <vector>

#include "Renderer/Object3D.hpp"

#include "Entity.hpp"

namespace Dem {

  struct Demeter {
  private:
    std::unique_ptr<SDL2> sdl2;

    struct Time {
    private:
      Uint64 last;
      Uint64 current;
      double delta = 0;

    public:
      Time(const SDL2 &sdl2Instance);

      void Update(const SDL2 &sdl2Instance);

      [[nodiscard]] Uint64 GetLast() const
      {
        return last;
      }

      [[nodiscard]] Uint64 GetCurrent() const
      {
        return current;
      }

      [[nodiscard]] double GetDelta() const
      {
        return delta;
      }
    } time;

    std::vector<std::shared_ptr<IEntity>> entityPool;
    std::vector<std::shared_ptr<Texture>> texturePool;
    std::vector<std::shared_ptr<Object3D>> objectPool;
    std::unique_ptr<ShaderProgram> shader;
    Camera camera;
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
    ~Demeter() = default;

    void AddEntity(std::shared_ptr<IEntity> entity)
    {
      entityPool.push_back(std::move(entity));
    }

    void DeleteEntity(const std::shared_ptr<IEntity> &entity)
    {
      std::ranges::remove(entityPool, entity);
    }

    void AddTexture(std::shared_ptr<Texture> texture)
    {
      texturePool.push_back(std::move(texture));
    }

    void DeleteTexture(const std::shared_ptr<Texture> &texture)
    {
      std::ranges::remove(texturePool, texture);
    }

    void AddObject3D(std::shared_ptr<Object3D> object)
    {
      objectPool.push_back(std::move(object));
    }

    void DeleteObject3D(const std::shared_ptr<Object3D> &object)
    {
      std::ranges::remove(objectPool, object);
    }

    void Run();
  };

}  // namespace Dem
