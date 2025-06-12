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

  static std::string GetError()
  {
    return SDL_GetError();
  }

  static std::string GetIMGError()
  {
    return IMG_GetError();
  }

  void SetAttribute(SDL_GLattr attr, int value) const  // NOLINT musn't be
                                                       // static
  {
    if (SDL_GL_SetAttribute(attr, value) < 0)
      throw std::
        runtime_error("SDL_GL_SetAttribute failed! SDL_Error: " + GetError());
  }

public:
  struct Texture {
  private:
    std::unique_ptr<SDL_Surface> surface;
    GLenum format;

  public:
    Texture(std::unique_ptr<SDL_Surface> surf, GLenum fmt)
      : surface(std::move(surf)), format(fmt)
    {
    }

    ~Texture()
    {
      if (surface)
        SDL_FreeSurface(surface.release());
    }

    [[nodiscard]] GLsizei GetWidth() const
    {
      return surface->w;
    }

    [[nodiscard]] GLsizei GetHeight() const
    {
      return surface->h;
    }

    [[nodiscard]] const void *GetPixels() const
    {
      return surface->pixels;
    }

    [[nodiscard]] GLenum GetFormat() const
    {
      return format;
    }
  };

  SDL2();
  ~SDL2();

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

  [[nodiscard]] std::shared_ptr<SDL2::Texture>
  LoadTexture(const std::string &path) const;
};
