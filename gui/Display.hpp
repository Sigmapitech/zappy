#pragma once

#include <SDL2/SDL.h>

class Display {
public:
  static void run_display();
  static SDL_Window *open_window();
};
