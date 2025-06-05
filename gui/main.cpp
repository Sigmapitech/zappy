#include <cstdio>
#include <cstdlib>
#include <thread>

#include "Display.hpp"
#include "args_parser.hpp"
#include "logging/Logger.hpp"

const char GUI_USAGE[] = {
  "Usage: ./zappy_ai [OPTIONS]\n"
  "Options:\n"
  "  -H, --help                Show this help message and exit\n"
  "  -p, --port <port>         Set the port number\n"
  "  -h, --host <machine>      Set the host machine\n"};

static constexpr const int EXIT_TEK_FAILURE = 84;

int main(int argc, char *argv[])
{
  parameters_s params;
  Log::info << "GUI started.";

  if (!parse_args(params, argc, argv))
    return EXIT_TEK_FAILURE;
  if (params.help)
    return printf("%s", GUI_USAGE), EXIT_SUCCESS;
  print_params(params);
  // std::jthread ui_thread(Display::run_display);
  return EXIT_SUCCESS;
}
