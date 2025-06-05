#include "ArgParsing/ArgParsing.hpp"
#include "Display.hpp"
#include "logging/Logger.hpp"

#include <iostream>
#include <thread>

int main(int ac, char **av)
{
  ArgParsing args;

  if (!args.ParseArg(ac, av)) {
    std::cout << "Invalid argument\n";
    return 84;
  }
  Log::info << "GUI started.";
  std::jthread ui_thread(Display::run_display);

  return 0;
}
