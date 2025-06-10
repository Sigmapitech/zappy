#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <thread>

#include "ArgsParser.hpp"
#include "Network/Network.hpp"
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
  Args params;
  Log::info << "GUI started.";
  if (!params.Parse(argc, argv))
    return EXIT_TEK_FAILURE;
  if (params.GetHelp()) {
    std::cerr << GUI_USAGE;
    return EXIT_SUCCESS;
  }
  std::cout << params;

  Network networkClass(params.GetPort(), params.GetHost());
  Log::info << "Network started.";
  try {
    std::jthread network_thread(&Network::RunNetwork, &networkClass);
  } catch (const std::runtime_error &e) {
    Log::warn << "Runtime warn: " << e.what();
    return EXIT_FAILURE;
  }
  // std::jthread ui_thread(Display::run_display);
  return EXIT_SUCCESS;
}
