#pragma once

#include <cstdint>
#include <string>

extern const std::string GUI_USAGE;  // Usage message for the GUI

/**
 * @brief Structure to hold command line parameters for the GUI.
 */
struct Args {
private:
  std::string host;  // Server hostname, NULL-terminated
  uint16_t port;     // Range between 1024 and 65535
  bool help;         // Display help message

  /**
   * @brief Helper function to parse a numeric argument.
   *
   * @param arg The string containing the number to parse.
   * @param name The field name for error messages.
   * @param min The minimum valid value for the number.
   * @param max The maximum valid value for the number.
   * @return uint16_t The parsed number if valid, or 0 if invalid.
   * @note If the argument is invalid, an error message is printed to stderr.
   * @note If the min is 0, you will need to adjust
   * your error handling because 0 is used as a sentinel here
   */
  static uint16_t ParseNumber(
    const std::string &arg,
    const char *name,
    uint16_t min,
    uint16_t max);

  /**
   * @brief Dispatches the argument parsing based on the option character.
   * @param argv list of arguments
   * @param opt the option char that indicates which argument is being parsed
   * @return true if the argument was parsed successfully
   * @return false if there was an error, printing an error message to stderr
   */
  bool Dispatcher(char *argv[], int opt);  // NOLINT

public:
  [[nodiscard]] bool GetHelp() const
  {
    return help;
  }

  [[nodiscard]] const std::string &GetHost() const
  {
    return host;
  }

  [[nodiscard]] uint16_t GetPort() const
  {
    return port;
  }

  /**
   * @brief Parses command line arguments and fills the params structure.
   *
   * @param argc size of argv
   * @param argv command line arguments
   * @return true if all arguments are valid and parsed successfully
   * @return false if there are invalid arguments or if malloc fails
   */
  bool Parse(int argc, char *argv[]);  // NOLINT

  /**
   * @brief Debugging function to print the parsed parameters.
   *
   * @param params Params structure containing the parsed command line
   * arguments.
   * @return std::string A string representation of the parsed parameters.
   */
  [[nodiscard]] std::string ToString() const;

  friend std::ostream &operator<<(std::ostream &os, const Args &args)
  {
    os << args.ToString();
    return os;
  }
};

