#include "Display.hpp"

#include "logging/Logger.hpp"

#include <thread>

int main()
{
  Log::info << "GUI started.";
  std::jthread ui_thread(Display::run_display);

  return 0;
}
