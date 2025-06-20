#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <unistd.h>

#include "ArgsParser.hpp"
#include "Network/Network.hpp"
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
  std::shared_ptr<API> data = std::make_shared<API>();
  Args params;

  Log::info << "GUI started.";
  if (!params.Parse(argc, argv))
    return EXIT_TEK_FAILURE;
  if (params.GetHelp()) {
    std::cout << GUI_USAGE;
    return EXIT_SUCCESS;
  }
  std::cout << params;

  try {
    Network networkClass(params.GetPort(), params.GetHost(), data);
    Log::info << "Network started.";
    networkClass.RunNetwork();
    Zappy zappy(data);
    zappy.Run();
    networkClass.RequestStop();
  } catch (const std::exception &e) {
    Log::warn << "Runtime warn: " << e.what();
    return EXIT_TEK_FAILURE;
  }

  return EXIT_SUCCESS;
}
