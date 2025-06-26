#pragma once

#include <memory>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_surface.h>

#include "SDL_timer.h"

/**
 * @brief SDL2 wrapper class for managing SDL2 window, OpenGL context, and
 * events.
 *
 * This struct encapsulates SDL2 initialization, window/context management,
 * event polling, and image loading functionalities. It provides utility
 * methods for error handling and OpenGL attribute configuration.
 *
 * It also handles OpenGL context initialization using GLEW.
 */
struct SDL2 {
private:
  bool _isSDLInit = false;
  bool _isIMGInit = false;
  SDL_Window *_window;
  SDL_GLContext _context = nullptr;
  SDL_Event _event;
  size_t _width;
  size_t _height;

  /**
   * @brief Sets an OpenGL attribute for the SDL context.
   *
   * This function sets the specified OpenGL attribute to the given value using
   * SDL_GL_SetAttribute. If the operation fails, it throws a
   * std::runtime_error with the SDL error message.
   *
   * @param attr The OpenGL attribute to set (of type SDL_GLattr).
   * @param value The value to assign to the attribute.
   *
   * @throws std::runtime_error if SDL_GL_SetAttribute fails.
   */
  void SetAttribute(SDL_GLattr attr, int value) const  // NOLINT
  {
    SDL_GL_SetAttribute(attr, value);
  }

public:
  /**
   * @brief Constructs the SDL2 renderer context.
   *
   * Initializes SDL2 with video support, sets OpenGL context attributes,
   * creates the SDL window and OpenGL context, and initializes GLEW.
   * Enables depth testing, face culling, and alpha blending for rendering.
   * Also initializes SDL_image with PNG and JPG support.
   *
   * @throws std::runtime_error if any initialization step fails, including:
   *         - SDL initialization
   *         - Window creation
   *         - OpenGL context creation
   *         - GLEW initialization
   *         - SDL_image initialization
   */
  SDL2() = default;
  ~SDL2();

  bool Init(size_t width = 800, size_t height = 600);

  void SetWindowSize(size_t width, size_t height)
  {
    _width = width;
    _height = height;
    glViewport(0, 0, (GLsizei)_width, (GLsizei)_height);
  }

  [[nodiscard]] size_t GetWidth() const
  {
    return _width;
  }

  [[nodiscard]] size_t GetHeight() const
  {
    return _height;
  }

  std::string GetError() const  // NOLINT
  {
    return SDL_GetError();
  }

  std::string GetIMGError() const  // NOLINT
  {
    return IMG_GetError();
  }

  /**
   * @brief Refreshes the screen by swapping the OpenGL buffers.
   *
   * This function updates the display with the rendered image by swapping
   * the front and back buffers of the SDL window.
   */
  void SwapWindow() const
  {
    SDL_GL_SwapWindow(_window);
  }

  /**
   * @brief Clears the color and depth buffers with the specified color.
   *
   * Sets the clear color using the provided RGBA values and clears both the
   * color and depth buffers.
   *
   * @param r Red component of the clear color (range: 0.0f to 1.0f).
   * @param g Green component of the clear color (range: 0.0f to 1.0f).
   * @param b Blue component of the clear color (range: 0.0f to 1.0f).
   * @param a Alpha component of the clear color (range: 0.0f to 1.0f).
   */
  void Clear(GLclampf r, GLclampf g, GLclampf b, GLclampf a) const  // NOLINT
  {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  /**
   * @brief Retrieves the number of milliseconds since the SDL library
   * initialization.
   *
   * This function returns the number of milliseconds, as a 64-bit unsigned
   * integer, that have elapsed since the SDL library was initialized. It is a
   * wrapper around SDL_GetTicks64(), providing higher precision and avoiding
   * overflow issues present in the 32-bit version.
   *
   * @return The number of milliseconds since SDL initialization as a Uint64.
   */
  [[nodiscard]] Uint64 GetTicks64() const  // NOLINT
  {
    return SDL_GetTicks64();
  }

  /**
   * @brief Polls for currently pending events.
   *
   * This function checks the event queue for pending events and updates the
   * internal SDL_Event structure. It returns true if there is an event
   * available, false otherwise.
   *
   * @note This function does not remove the event from the queue.
   *
   * @return true if there is a pending event, false otherwise.
   */
  [[nodiscard]] bool PollEvent()
  {
    return SDL_PollEvent(&_event);
  }

  /**
   * @brief Retrieves the current SDL event.
   *
   * This function returns a constant reference to the internally stored
   * SDL_Event object, allowing read-only access to the most recent event
   * polled from the SDL event queue.
   *
   * @return const SDL_Event& A constant reference to the current SDL_Event.
   */
  [[nodiscard]] const SDL_Event &GetEvent() const
  {
    return _event;
  }

  /**
   * @brief Loads an image from the specified file path into an SDL_Surface.
   *
   * This function uses SDL_image's IMG_Load to load an image file and returns
   * a unique_ptr managing the resulting SDL_Surface. The caller is responsible
   * for ensuring the file path is valid and that SDL_image is properly
   * initialized.
   *
   * @param s The file path to the image to load.
   * @return std::unique_ptr<SDL_Surface> A unique pointer to the loaded
   * SDL_Surface, or nullptr if loading fails.
   */
  [[nodiscard]] std::unique_ptr<SDL_Surface>
  IMGLoad(const std::string &s) const  // NOLINT
  {
    return std::unique_ptr<SDL_Surface>(IMG_Load(s.c_str()));
  }
};
