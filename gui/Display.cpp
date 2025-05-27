#include "Display.hpp"
#include <SDL2/SDL.h>
#include <stdio.h>

void Display::RunDisplayer(void) {}

void Display::OpenWindow(void)
{
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stdout, "Error :(%s)\n", SDL_GetError());
    return;
  }
  {
    SDL_Window *pWindow = NULL;
    pWindow = SDL_CreateWindow(
      "Window",
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      640,
      480,
      SDL_WINDOW_SHOWN);

    if (pWindow) {
      SDL_Delay(3000);

      SDL_DestroyWindow(pWindow);
    } else {
      fprintf(
        stderr, "Erreur de création de la fenêtre: %s\n", SDL_GetError());
    }
  }
  SDL_Quit();

  return;
}
