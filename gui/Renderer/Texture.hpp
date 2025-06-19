#include <memory>

#include "SDL2.hpp"

struct Texture {
private:
  std::unique_ptr<SDL_Surface> surface;
  GLenum format;
  GLuint texture = 0;

  void GenGLTexture();

public:
  Texture(SDL2 &sdl, const std::string &path);
  ~Texture();

  [[nodiscard]] GLuint GetGL() const
  {
    return texture;
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
