#include <memory>
#include <stdexcept>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_surface.h>

#include <GL/glew.h>

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
  bool _isInit = false;
  SDL_Window *_window;
  SDL_GLContext _context = nullptr;
  SDL_Event _event;

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
    if (SDL_GL_SetAttribute(attr, value) < 0)
      throw std::
        runtime_error("SDL_GL_SetAttribute failed! SDL_Error: " + GetError());
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
  SDL2();
  ~SDL2();

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
