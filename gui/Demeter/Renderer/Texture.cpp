#include "logging/Logger.hpp"

#include "Texture.hpp"

bool Texture::Init(SDL2 &sdl, const std::string &path)
{
  surface = std::unique_ptr<SDL_Surface>(sdl.IMGLoad(path));
  if (!surface) {
    Log::failed
      << "Failed to load image: " << path
      << "\nSDL_image error: " << sdl.GetIMGError();
    return false;
  }

  switch (surface->format->BytesPerPixel) {
    case 4:
      format = (surface->format->Rmask == 0x000000ff) ? GL_RGBA : GL_BGRA;
      break;
    case 3:
      format = (surface->format->Rmask == 0x000000ff) ? GL_RGB : GL_BGR;
      break;
    default:
      Log::failed
        << "Unsupported image format: " << surface->format->BytesPerPixel
        << " bytes per pixel";
      return false;
  }

  GenGLTexture();
  return true;
}

Texture::~Texture()
{
  if (surface)
    SDL_FreeSurface(surface.release());
  if (texture)
    glDeleteTextures(1, &texture);
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
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
  // GL_LINEAR_MIPMAP_LINEAR); glTexParameteri(GL_TEXTURE_2D,
  // GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}
