#pragma once

#include <string>

namespace Dem {

  struct Window {
  private:
    const std::string _name;
    sfRenderWindow *_window;
    size_t _width;
    size_t _height;
    size_t _framerate;

  public:
    Window(
      const std::string &name,
      size_t _width = 1920,
      size_t _height = 1080,
      size_t framerate = 60);
    ~Window();

    [[nodiscard]] const std::string &GetName() const;
    [[nodiscard]] size_t GetWidth() const;
    [[nodiscard]] size_t GetHeight() const;
    [[nodiscard]] size_t getFramerate() const;
  };

}  // namespace Dem
