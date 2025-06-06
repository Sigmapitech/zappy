#include <getopt.h>
#include <iostream>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args_parser.hpp"
#include "bits/getopt_core.h"

// Helper message to avoid long strings in the code
static constexpr const char INVALID_ARG[] = {
  "Invalid option or missing argument\n%s\n"};

// Structure to hold the command line parameters, to be used by getopt_long
static const struct option long_options[] = {
  {"help", no_argument, nullptr, 'H'},
  {"port", required_argument, nullptr, 'p'},
  {"host", required_argument, nullptr, 'h'},
  {nullptr, 0, nullptr, 0}};

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
static uint16_t
parse_number_arg(const char *arg, const char *name, uint16_t min, uint16_t max)
{
  char *endptr;
  long value = strtol(arg, &endptr, 10);

  if (*endptr != '\0' || value < min || value > max) {
    fprintf(
      stderr,
      "Invalid value for %s: %s (must be between %u and %u)\n",
      name,
      arg,
      min,
      max);
    return 0;
  }
  return value;
}

/**
 * @brief Dispatches the argument parsing based on the option character.
 * @param params pointer to the params_t structure to fill
 * @param argv list of arguments
 * @param opt the option char that indicates which argument is being parsed
 * @return true if the argument was parsed successfully
 * @return false if there was an error, printing an error message to stderr
 */
static bool arg_dispatcher(parameters_s &params, char *argv[], char opt)
{
  (void)argv;
  switch (opt) {
    case 'H':
      params.help = true;
      return true;
    case 'h':
      params.host = std::string(optarg);
      break;
    case 'p':
      params.port = parse_number_arg(optarg, "p", 1024, 65535);
      break;
    case '?':
    default:
      return fprintf(stderr, INVALID_ARG, GUI_USAGE.c_str()), false;
  }
  return true;
}

void print_params(const parameters_s &params)
{
  printf("====================Zappy GUI====================\n");
  printf("port = %d\n", params.port);
  printf("host = %s\n", params.host.c_str());
  printf("=================================================\n");
}

bool parse_args(parameters_s &params, int argc, char *argv[])
{
  for (int opt;;) {
    opt = getopt_long(argc, argv, "Hp:h:", long_options, nullptr);
    if (opt < 0)
      break;
    if (!arg_dispatcher(params, argv, opt))
      return false;
  }
  if (params.host.empty())
    params.host = std::string("127.0.0.1");
  if (params.help == true)
    return true;
  if (params.port == 0 || params.host.empty()) {
    std::cerr << GUI_USAGE;
    return false;
  }
  return true;
}
