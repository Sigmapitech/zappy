#include <SDL2/SDL_surface.h>
#include <memory>
#include <stdexcept>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <GL/glew.h>

struct SDL2 {
private:
  bool _isInit = false;
  SDL_Window *_window;
  SDL_GLContext _context = nullptr;
  SDL_Event _event;

  void SetAttribute(SDL_GLattr attr, int value) const  // NOLINT
  {
    if (SDL_GL_SetAttribute(attr, value) < 0)
      throw std::
        runtime_error("SDL_GL_SetAttribute failed! SDL_Error: " + GetError());
  }

public:
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

  void SwapWindow() const
  {
    SDL_GL_SwapWindow(_window);
  }

  [[nodiscard]] bool PollEvent()
  {
    return SDL_PollEvent(&_event);
  }

  [[nodiscard]] const SDL_Event &GetEvent() const
  {
    return _event;
  }

  [[nodiscard]] std::unique_ptr<SDL_Surface>
  IMGLoad(const std::string &s) const  // NOLINT
  {
    return std::unique_ptr<SDL_Surface>(IMG_Load(s.c_str()));
  }
};
