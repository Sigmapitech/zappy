#include "ArgParsing.hpp"
#include <ostream>
#include <string>

#include <iostream>

ArgParsing::ArgParsing(int port_arg, std::string &hostname_arg)
  : _port(port_arg), _hostname(hostname_arg)
{
}

namespace {
  constexpr unsigned int hash(const char *str)
  {
    unsigned int h = 5381;
    for (; *str != '\0'; str++)
      h = (h * 33) ^ (unsigned char)*str;
    return h;
  }
}  // namespace

bool ArgParsing::ParseArg(int ac, char **av)
{
  if (ac <= 1 || ac > 6)
    return false;

  for (int i = 1; i < ac; i++) {
    switch (hash(av[i])) {

      case hash("-h"):
      case hash("--hostname"):
        if (i == ac - 1)
          return false;
        _hostname = av[i + 1];
        std::cout << "hostname = " << _hostname << "\n";
        i++;
        break;

      case hash("-p"):
      case hash("--port"):
        if (i == ac - 1)
          return false;
        _port = std::stoi(av[i + 1]);
        std::cout << "port = " << _port << "\n";
        i++;
        break;

      case hash("-help"):
      case hash("--help"):
        _help = true;
        return true;

      default:
        return false;
    }
  }
  return true;
}
