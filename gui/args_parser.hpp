#pragma once

#include <cstdint>
#include <string>

/**
 * @brief Structure to hold command line parameters for the GUI.
 */
struct parameters_s {
  std::string host;  // Server hostname, NULL-terminated
  uint16_t port;     // Range between 1024 and 65535
  bool help;         // Display help message
};

extern const char GUI_USAGE[];  // Usage message for the GUI

/**
 * @brief Parses command line arguments and fills the params structure.
 *
 * @param params pointer to the params_t structure to fill
 * @param argc size of argv
 * @param argv command line arguments
 * @return true if all arguments are valid and parsed successfully
 * @return false if there are invalid arguments or if malloc fails
 */
bool parse_args(parameters_s &params, int argc, char *argv[]);

/**
 * @brief Debugging function to print the parsed parameters.
 *
 * @param params Params structure containing the parsed command line arguments.
 */
void print_params(const parameters_s &params);
