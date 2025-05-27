#include <cstdio>

#include <SDL2/SDL.h>

#include "Display.hpp"
#include "logging/Logger.hpp"

void Display::run_display()
{
  // TODO: make the window "safe"
  auto *window = Display::open_window();

  SDL_Delay(3000);

  SDL_DestroyWindow(window);
  SDL_Quit();
}

SDL_Window *Display::open_window()
{
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stdout, "Error :(%s)\n", SDL_GetError());
    return nullptr;
  }
  {
    SDL_Window *window = SDL_CreateWindow(
      "Window",
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      640,
      480,
      SDL_WINDOW_SHOWN);

    if (window == nullptr)
      Log::failed << "Cannot open window: " << SDL_GetError();
    return window;
  }
}
