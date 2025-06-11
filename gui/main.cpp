#include "Display.hpp"

#include "./Network/Network.hpp"

#include <thread>

int main()
{
  std::string host = "127.0.0.1";

  Network networkClass(4242, host);
  std::cout << "Network started.\n";
  std::jthread network_thread(&Network::runNetwork, &networkClass);

  std::cout << "GUI started.\n";
  std::jthread ui_thread(Display::run_display);
  return 0;
}
