#include "Display.hpp"

#include "logging/Logger.hpp"

#include <thread>

int main()
{
  Display displayer;

  Log::info << "GUI started.";
  std::thread display([&displayer]() { displayer.RunDisplayer(); });

  display.join();
  return 0;
}
