#include "Display.hpp"
#include "Network/Network.hpp"
#include "args_parser.hpp"
#include "logging/Logger.hpp"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <thread>

const std::string GUI_USAGE = {
  "Usage: ./zappy_gui [OPTIONS]\n"
  "Options:\n"
  "  --help                Show this help message and exit\n"
  "  -p, --port <port>         Set the port number\n"
  "  -h, --host <machine>      Set the host machine\n"};

static constexpr const int EXIT_TEK_FAILURE = 84;

int main(int argc, char *argv[])
{
  parameters_s params;

  if (!parse_args(params, argc, argv))
    return EXIT_TEK_FAILURE;
  if (params.help) {
    std::cerr << GUI_USAGE;
    return EXIT_SUCCESS;
  }
  print_params(params);

  Network networkClass(params.port, params.host);

  Log::info << "Network started.\n";
  std::jthread network_thread(&Network::runNetwork, &networkClass);

  Log::info << "GUI started.\n";
  std::jthread ui_thread(Display::run_display);
  return EXIT_SUCCESS;
}
