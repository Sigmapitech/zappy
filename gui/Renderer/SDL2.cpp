#include <iostream>

#include "SDL2.hpp"

SDL2::SDL2()
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    throw std::
      runtime_error("SDL initialization failed! SDL_Error: " + GetError());
  SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  _window = SDL_CreateWindow(
    "SDL2 OpenGL Cube",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    800,
    600,
    SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
  if (_window == nullptr)
    throw std::
      runtime_error("SDL_CreateWindow failed! SDL_Error: " + GetError());

  _context = SDL_GL_CreateContext(_window);
  if (_context == nullptr)
    throw std::
      runtime_error("SDL_GL_CreateContext failed! SDL_Error: " + GetError());
  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK)
    throw std::runtime_error("GLEW initialization failed!");
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  // enable face culling, eg: triangles facing away from the camera
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);  // cull back-facing polygons
  glFrontFace(GL_CCW);  // winding order, eg: determines front-facing polygons

  // Enable alpha blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) == 0)
    throw std::
      runtime_error("SDL_image initialization failed: " + GetIMGError());
}

SDL2::~SDL2()
{
  IMG_Quit();
  SDL_GL_DeleteContext(_context);
  SDL_DestroyWindow(_window);
  SDL_Quit();
}

[[nodiscard]] std::shared_ptr<SDL2::Texture>
SDL2::LoadTexture(const std::string &path) const  // NOLINT musn't be static
{
  std::unique_ptr<SDL_Surface> surface(IMG_Load(path.c_str()));
  if (!surface) {
    std::cerr
      << "Failed to load image: " << path
      << "\nSDL_image error: " << IMG_GetError() << '\n';
    return nullptr;
  }

  GLenum format;
  if (surface->format->BytesPerPixel == 4) {
    format = (surface->format->Rmask == 0x000000ff) ? GL_RGBA : GL_BGRA;
  } else if (surface->format->BytesPerPixel == 3) {
    format = (surface->format->Rmask == 0x000000ff) ? GL_RGB : GL_BGR;
  } else {
    std::cerr << "Unknown image format\n";
    SDL_FreeSurface(surface.get());
    return nullptr;
  }
  return std::make_shared<Texture>(std::move(surface), format);
}
