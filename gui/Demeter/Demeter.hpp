#pragma once

#include <vector>

#include "Entity.hpp"
#include "Window.hpp"

namespace Dem {

  struct Demeter {
  private:
    std::vector<Entity> entity_arr;
    event_t event;
    sfTexture *tex_arr[TEX_ARR_SIZE];
    sfSoundBuffer *snd_arr[SND_ARR_SIZE];
    de_arr_t *scene_arr;
    sfClock *main_clock;
    sfClock *game_clock;
    sfSprite *font_sprite;
    Window window;
    sfInt64 _prevFrameClock;
    double _deltaTime;
    bool is_running;

  public:
    Demeter(const Window &win);
    ~Demeter();

    void unset();
    int update();
    void draw();
  };

}  // namespace Dem
