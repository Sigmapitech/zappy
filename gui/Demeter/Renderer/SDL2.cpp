#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"

#include "logging/Logger.hpp"

#include "SDL2.hpp"

bool SDL2::Init(size_t width, size_t height)
{
  _width = width;
  _height = height;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    Log::failed << "SDL initialization failed! SDL_Error: " << GetError();
    return false;
  }
  _isSDLInit = true;
  SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  _window = SDL_CreateWindow(
    "SDL2 OpenGL Cube",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    _width,
    _height,
    SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  if (_window == nullptr) {
    Log::failed << "SDL_CreateWindow failed! SDL_Error: " << GetError();
    return false;
  }

  _context = SDL_GL_CreateContext(_window);
  if (_context == nullptr) {
    Log::failed << "SDL_GL_CreateContext failed! SDL_Error: " << GetError();
    return false;
  }
  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    Log::failed << "GLEW initialization failed!";
    return false;
  }

  // Initial viewport setup
  SetWindowSize(_width, _height);

  // Enable depth testing
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  // enable face culling, eg: triangles facing away from the camera
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);  // cull back-facing polygons
  glFrontFace(GL_CCW);  // winding order, eg: determines front-facing polygons

  // Enable alpha blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) == 0) {
    Log::failed << "SDL_image initialization failed: " << GetIMGError();
    return false;
  }
  _isIMGInit = true;

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

  ImGui_ImplSDL2_InitForOpenGL(_window, _context);
  ImGui_ImplOpenGL3_Init();
  return true;
}

SDL2::~SDL2()
{
  if (ImGui::GetCurrentContext()) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
  }

  if (_isIMGInit)
    IMG_Quit();
  if (_context)
    SDL_GL_DeleteContext(_context);
  if (_window)
    SDL_DestroyWindow(_window);
  if (_isSDLInit)
    SDL_Quit();
}
