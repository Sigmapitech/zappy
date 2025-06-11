#include <getopt.h> // NOLINT getopt must be before bits/getopt_core.h
#include <bits/getopt_core.h>

#include <array>
#include <cstdint>
#include <iostream>
#include <sstream>

#include "args_parser.hpp"

#define HELP_OPT 1000

// Structure to hold the command line parameters, to be used by getopt_long
static
const std::array<struct option, 4>  long_options = {{
  {"help", no_argument, nullptr, HELP_OPT},
  {"port", required_argument, nullptr, 'p'},
  {"host", required_argument, nullptr, 'h'},
  {nullptr, 0, nullptr, 0}
}};

uint16_t Args::ParseNumber(const std::string &arg, const char *name, uint16_t min, uint16_t max)
{
  std::stringstream ss(arg);
  long value;
  if (!(ss >> value)) {
    std::cerr << "Invalid value for " << name << ": " << arg << " (not a number)\n";
    return 0;
  }
  if (!ss.eof()) {
    std::cerr << "Invalid value for " << name << ": " << arg << " (extra characters after number)\n";
    return 0;
  }
  if (value < min || value > max) {
    std::cerr << "Invalid value for " << name << ": " << arg << " (must be between " << min << " and " << max << ")\n";
    return 0;
  }
  return value;
}

bool Args::Dispatcher(char **argv, char opt)
{
  (void)argv;
  switch (opt) {
    case HELP_OPT:
      help = true;
      break;
    case 'h':
      host = optarg;
      break;
    case 'p':
      port = ParseNumber(optarg, "p", 1024, UINT16_MAX);
      break;
    case '?':
    default:
      std::cerr << "Invalid option or missing argument\n" << GUI_USAGE << "\n";
      return false;
  }
  return true;
}

std::string Args::ToString() const
{
  return
    "====================Zappy GUI====================\n"
    "port = " + std::to_string(port) + "\n"
    "host = " + host + "\n"
    "=================================================\n";
}

bool Args::Parse(int argc, char **argv)
{
  for (int opt;;) {
    opt = getopt_long(argc, argv, "p:h:", long_options.data(), nullptr);
    if (opt < 0)
      break;
    if (!Dispatcher(argv, opt))
      return false;
  }
  if (help)
    return true;
  if (port == 0 || host.empty()) {
    std::cerr << GUI_USAGE;
    return false;
  }
  return true;
}
