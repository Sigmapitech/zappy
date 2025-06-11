#include <cstdlib>
#include <iostream>
#include <thread>
const std::string GUI_USAGE = {
  "Usage: ./zappy_gui [OPTIONS]\n"
  "Options:\n"
  "  --help                Show this help message and exit\n"
  "  -p, --port <port>         Set the port number\n"
  "  -h, --host <machine>      Set the host machine\n"};

#include "ArgsParser.hpp"
#include "Display.hpp"
#include "logging/Logger.hpp"

const std::string GUI_USAGE = {
  "Usage: ./zappy_gui [OPTIONS]\n"
  "Options:\n"
  "  --help                    Show this help message and exit\n"
  "  -p, --port <port>         Set the port number\n"
  "  -h, --host <machine>      Set the host machine\n"};

static constexpr const int EXIT_TEK_FAILURE = 84;

int main(int argc, char *argv[])
{
  Args params;
  Log::info << "GUI started.";

  if (!params.Parse(argc, argv))
    return EXIT_TEK_FAILURE;
  if (params.GetHelp()) {
    std::cerr << GUI_USAGE;
    return EXIT_SUCCESS;
  }
  std::cout << params;
  // std::jthread ui_thread(Display::run_display);
  return EXIT_SUCCESS;
}
