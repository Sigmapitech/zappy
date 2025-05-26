#include "Display.hpp"
#include <iostream>
#include <thread>

int main(int argc, char **argv) {
  Display displayer;

  // std::thread network;
  std::thread display([&displayer]() { displayer.RunDisplayer(); });

  // network.join();
  display.join();
  return 0;
}
