#include <cstdlib>
#include <exception>
#include <iostream>
#include <unistd.h>

#include "ArgsParser.hpp"
#include "Zappy.hpp"
#include "logging/Logger.hpp"

const std::string GUI_USAGE = {
  "Usage: ./zappy_gui [OPTIONS]\n"
  "Options:\n"
  "  --help                    Show this help message and exit\n"
  "  -p, --port <port>         Set the port number\n"
  "  -h, --host <machine>      Set the host machine\n"};

static constexpr const int EXIT_TEK_FAILURE = 84;

[[gnu::weak]]
int main(int argc, char *argv[])
{
  try {
    Args params;

    if (!params.Parse(argc, argv))
      return EXIT_TEK_FAILURE;
    if (params.GetHelp()) {
      std::cout << GUI_USAGE;
      return EXIT_SUCCESS;
    }
    std::cout << params;

    Zappy zappy;
    if (!zappy.Init(params)) {
      Log::failed << "Failed to initialize Zappy!";
      return EXIT_TEK_FAILURE;
    }
    zappy.Run();
    return EXIT_SUCCESS;
  } catch (const std::exception &e) {
    Log::failed << "Runtime error: " << e.what();
    return EXIT_TEK_FAILURE;
  }
}
