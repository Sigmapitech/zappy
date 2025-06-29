#pragma once

#include <memory>

#include "SDL2.hpp"

/**
 * @brief Represents an OpenGL texture loaded from an SDL_Surface.
 *
 * The Texture class encapsulates the loading, management, and cleanup of an
 * OpenGL texture created from an image file using SDL2. It manages the
 * underlying SDL_Surface and OpenGL texture object, providing access to
 * texture properties and pixel data.
 *
 * @note Copying is disabled to prevent multiple ownership of the same texture
 * resource.
 */
struct Texture {
private:
  std::unique_ptr<SDL_Surface> surface;
  GLenum format;
  GLuint texture = 0;

  /**
   * @brief Generates and configures an OpenGL texture from the associated
   * surface.
   *
   * This method creates a new OpenGL texture, binds it, uploads the pixel data
   * from the surface, generates mipmaps, and sets texture parameters for
   * wrapping and filtering. The texture format and data are determined by the
   * surface and the class's format settings.
   *
   * @note Used only once at construction, assumes that a valid OpenGL context
   * is active.
   */
  void GenGLTexture();

public:
  /**
   * @brief Constructs a Texture object by loading an image from the specified
   * path.
   *
   * This constructor loads an image using the provided SDL2 wrapper and
   * initializes the OpenGL texture format based on the image's pixel format.
   * If the image fails to load, or if the image format is unsupported, it
   * throws a std::runtime_error.
   *
   * @param sdl Reference to an SDL2 wrapper object used for image loading and
   * error handling.
   * @param path Path to the image file to be loaded as a texture.
   *
   * @throws std::runtime_error If the image cannot be loaded or if the image
   * format is unknown.
   */
  Texture(SDL2 &sdl, const std::string &path);
  Texture(const Texture &) = delete;
  Texture &operator=(const Texture &) = delete;
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
