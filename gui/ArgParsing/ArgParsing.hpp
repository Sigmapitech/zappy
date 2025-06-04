#pragma once

#include "string"

class ArgParsing {
private:
  int _port = -1;
  std::string _hostname;
  bool _help = false;

public:
  /**
   * @brief Construct a new Arg Parsing object without parametter
   */
  ArgParsing() = default;

  /**
   * @brief Construct a new Arg Parsing object with parsed arg
   *
   * @param port_arg     Contain the port of the GUI
   * @param hostname_arg Contain the hostname of the server
   */
  ArgParsing(int port_arg, std::string &hostname_arg);
  ~ArgParsing() = default;

  /**
   * @brief
   *
   * @param ac      Contain the number of parametter given to "./zappy_gui"
   * @param av      Contain all parametter givent to "./zappy_gui"
   * @return true   Means that ParseArg didn't fail
   * @return false  Means that ParseArg fail
   */
  [[nodiscard]] bool ParseArg(int ac, char **av);
};
