#include <memory>
#include <stdexcept>

#include "Texture.hpp"

Texture::Texture(SDL2 &sdl, const std::string &path)
  : surface(std::move(sdl.IMGLoad(path)))
{
  if (!surface)
    throw std::runtime_error(
      "Failed to load image: " + path
      + "\nSDL_image error: " + sdl.GetIMGError());

  switch (surface->format->BytesPerPixel) {
    case 4:
      format = (surface->format->Rmask == 0x000000ff) ? GL_RGBA : GL_BGRA;
      break;
    case 3:
      format = (surface->format->Rmask == 0x000000ff) ? GL_RGB : GL_BGR;
      break;
    default:
      throw std::runtime_error("Unknown image format");
  }

  GenGLTexture();
}

Texture::~Texture()
{
  if (surface)
    SDL_FreeSurface(surface.release());
}

void Texture::GenGLTexture()
{
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GetFormat(),
    surface->w,
    surface->h,
    0,
    format,
    GL_UNSIGNED_BYTE,
    surface->pixels);

  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(
    GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
