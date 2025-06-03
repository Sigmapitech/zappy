#include "Display.hpp"
#include "Synthesizer.hpp"

#include "logging/Logger.hpp"

#include <iostream>
#include <thread>

int main()
{
  // Log::info << "GUI started.";
  // std::jthread ui_thread(Display::run_display);

  // return 0;

  Synthesizer synth;
  if (!synth.init("gui/animalese_samples"))
    return 1;

  std::cout << "> Hello? This is Tom Nook from Nook Inc., yes..." << std::endl;
  synth.speak("Hello? This is Tom Nook from Nook Inc., yes...");
  std::this_thread::sleep_for(std::chrono::seconds(2));
  // xor utf8 'ZAPPY'
  std::cout
    << "> "
       "\x12\x24\x3C\x3C\x36\x65\x61\x04\x38\x30\x29\x61\x39\x23\x79\x0E\x2E"
       "\x3D\x70\x17\x35\x2E\x3B\x70\x3F\x28\x2E\x3D\x70\x17\x35\x2E\x3B\x70"
       "\x10\x34\x22\x7E\x7C\x79\x23\x24\x23\x7E\x77\x74"
    << std::endl;
  synth.speak(
    "\x12\x24\x3C\x3C\x36\x65\x61\x04\x38\x30\x29\x61\x39\x23\x79\x0E\x2E\x3D"
    "\x70\x17\x35\x2E\x3B\x70\x3F\x28\x2E\x3D\x70\x17\x35\x2E\x3B\x70\x10\x34"
    "\x22\x7E\x7C\x79\x23\x24\x23\x7E\x77\x74");
  std::this_thread::sleep_for(std::chrono::seconds(6));

  synth.shutdown();
  return 0;
}
