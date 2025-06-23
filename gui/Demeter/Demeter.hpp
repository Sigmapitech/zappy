#pragma once

#include <algorithm>
#include <memory>
#include <vector>

#include "Renderer/Object3D.hpp"

#include "Entity.hpp"

/**
 * @brief The Demeter struct encapsulates the main application logic for the
 * Demeter GUI.
 *
 * This struct manages the SDL2 rendering context, timing, entity, texture, and
 * object pools, as well as shader programs and camera state. It provides
 * methods for adding and removing entities, textures, and 3D objects, and
 * controls the main application loop.
 *
 * @note This struct is non-copyable due to unique ownership of SDL2 and
 * ShaderProgram resources.
 */
namespace Dem {

  struct Demeter {
  private:
    std::unique_ptr<SDL2> sdl2;

    /**
     * @struct Time
     * @brief Manages timing information using SDL2 for frame updates.
     *
     * The Time struct encapsulates timing data, including the last and current
     * tick counts, as well as the delta time between updates. It provides
     * methods to update and retrieve timing information, which is useful for
     * frame-based logic and animations.
     */
    struct Time {
    private:
      Uint64 last;
      Uint64 current;
      double delta = 0;

    public:
      /**
       * @brief Constructs a Time object and initializes its time tracking
       * members.
       *
       * This constructor initializes the Time object by capturing the current
       * tick count from the provided SDL2 instance. Both the 'last' and
       * 'current' members are set to the value returned by SDL2::GetTicks64(),
       * ensuring that time tracking starts from the same reference point.
       *
       * @param sdl2Instance Reference to an SDL2 object used to obtain the
       * current tick count.
       */
      Time(const SDL2 &sdl2Instance);

      /**
       * @brief Updates the delta time using the provided SDL2 instance.
       *
       * @param sdl2Instance Reference to an SDL2 instance used for rendering
       * or event handling.
       */
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

    /**
     * @brief OpenGL debug callback function for handling debug messages.
     *
     * This static function is intended to be registered with OpenGL as a debug
     * message callback. It will be called automatically by OpenGL whenever a
     * debug message is generated.
     *
     * @param source    Specifies the source of the debug message (e.g.,
     * GL_DEBUG_SOURCE_API).
     * @param type      Specifies the type of the debug message (e.g.,
     * GL_DEBUG_TYPE_ERROR).
     * @param id        Specifies the ID of the debug message.
     * @param severity  Specifies the severity of the debug message (e.g.,
     * GL_DEBUG_SEVERITY_HIGH).
     * @param length    Specifies the length of the debug message string.
     * @param message   Pointer to the debug message string.
     * @param userParam Pointer to user-supplied data.
     */
    static void DebugCallback(
      GLenum source,
      GLenum type,
      GLuint id,
      GLenum severity,
      GLsizei length,
      const GLchar *message,
      const void *userParam);

    /**
     * @brief Updates the state of the Demeter instance.
     *
     * This method performs two main tasks:
     * 1. Updates the internal time state using the SDL2 context.
     * 2. Iterates through all entities in the entity pool and calls their
     * Update method, passing the current Demeter instance as a parameter.
     *
     * This ensures that both the timing logic and all managed entities are
     * kept up to date each frame or tick.
     */
    void Update();

    /**
     * @brief Renders the current frame by drawing all entities and updating
     * the display.
     *
     * This function clears the screen with a specified background color,
     * activates the shader program, iterates through all entities in the
     * entity pool and calls their Draw method, and finally swaps the window
     * buffer to present the rendered frame.
     */
    void Draw();

  public:
    Demeter(std::unique_ptr<SDL2> renderer, bool activateDebug = false);
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

    /**
     * @brief Main loop for the Demeter application.
     *
     * This function starts the main event loop, processing SDL events and
     * updating/drawing the application state while `isRunning` is true. It
     * handles the SDL_QUIT event to terminate the loop. If OpenGL debug mode
     * (`glDebug`) is enabled, it checks for and logs any OpenGL errors after
     * each frame.
     *
     * @note This function blocks until the application is instructed to quit.
     */
    void Run();
  };

}  // namespace Dem
