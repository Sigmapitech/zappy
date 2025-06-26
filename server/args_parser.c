#include <getopt.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args_parser.h"
#include "bits/getopt_core.h"
#include "client.h"
#include "debug.h"

// Helper message to avoid long strings in the code
static constexpr const char INVALID_ARG[] = {
    "Invalid option or missing argument\n%s\n"
};

// Structure to hold the command line parameters, to be used by getopt_long
static const struct option long_options[] = {
    {"help", no_argument, nullptr, 'h'},
    {"port", required_argument, nullptr, 'p'},
    {"width", required_argument, nullptr, 'x'},
    {"height", required_argument, nullptr, 'y'},
    {"names", required_argument, nullptr, 'n'},
    {"client-number", required_argument, nullptr, 'c'},
    {"freq", required_argument, nullptr, 'f'},
    {nullptr, 0, nullptr, 0}
};

static
size_t get_team_slot(char **teams, const char *team_name, size_t count)
{
    size_t i = 0;

    for (; i < count; i++)
        if (strcmp(teams[i], team_name) == 0)
            return i;
    return count;
}

/**
 * @brief Parses team names from command line arguments.
 *
 * This function extracts team names from the command line arguments
 *
 * @param argv list of arguments
 * @param idx index in argv to start parsing teams
 * (should be the index of the first team name)
 * @return char** list of team names, nullptr-terminated
 * @note
 * The function will allocate memory for the team names.
 * The caller is responsible for freeing the memory.
 */
static
bool parse_teams(params_t *params, char *argv[], int *idx)
{
    size_t i = 0;
    size_t slot;

    if (argv[*idx] == nullptr)
        return false;
    for (; argv[*idx + i] != nullptr && *argv[*idx + i] != '-'; i++);
    for (size_t j = 0; j < i; j++) {
        slot = get_team_slot(
            params->teams, argv[*idx + j], params->registered_team_count);
        if (slot != params->registered_team_count)
            return fprintf(stderr, (slot > TEAM_ID_GRAPHIC)
                ? "Team '%s' is already registered.\n"
                : "Cannot create reserved team name '%s'\n"
                , argv[*idx + j]), false;
        params->teams[params->registered_team_count] = argv[*idx + j];
        params->registered_team_count++;
    }
    *idx += i;
    params->teams[params->registered_team_count] = nullptr;
    return true;
}

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
static
uint16_t parse_number_arg(
    const char *arg, const char *name, uint16_t min, uint16_t max)
{
    char *endptr;
    long value = strtol(arg, &endptr, 10);

    if (*endptr != '\0' || value < min || value > max) {
        fprintf(stderr,
            "Invalid value for %s: %s (must be between %u and %u)\n",
            name, arg, min, max
        );
        return 0;
    }
    return value;
}

/**
 * @brief Dispatches the argument parsing based on the option character.
 * @param params pointer to the params_t structure to fill
 * @param arg the argument string to parse
 * @param opt the option char that indicates which argument is being parsed
 * @return true if it's a valid numerical argument and parsed successfully
 * @return false otherwise, printing an error message to stderr
 */
static
bool number_arg_dispatcher(params_t *params, const char *arg, char opt)
{
    switch (opt) {
        case 'f':
            params->frequency = parse_number_arg(arg, "f", 1, 10000);
            break;
        case 'x':
            params->map_width = parse_number_arg(arg, "x", 10, 42);
            break;
        case 'y':
            params->map_height = parse_number_arg(arg, "y", 10, 42);
            break;
        case 'p':
            params->port = parse_number_arg(arg, "p", 1024, 65535);
            break;
        case 'c':
            params->team_capacity = parse_number_arg(arg, "c", 1, 200);
            break;
        default:
            return fprintf(stderr, INVALID_ARG, SERVER_USAGE), false;
    }
    return true;
}

/**
 * @brief Dispatches the argument parsing based on the option character.
 * @param params pointer to the params_t structure to fill
 * @param argv list of arguments
 * @param opt the option char that indicates which argument is being parsed
 * @return true if the argument was parsed successfully
 * @return false if there was an error, printing an error message to stderr
 */
static
bool arg_dispatcher(params_t *params, char *argv[], char opt)
{
    switch (opt) {
        case 'h':
            params->help = true;
            return true;
        case 'n':
            optind--;
            if (!parse_teams(params, argv, &optind)) {
                fprintf(stderr, "Failed to parse team names.\n");
                return false;
            }
            return true;
        case '?':
        default:
            return number_arg_dispatcher(params, optarg, opt);
    }
}

/**
 * @brief Debugging function to print the parsed parameters.
 *
 * @param params Params structure containing the parsed command line arguments.
 */
DEBUG_USED static
void print_params(const params_t *params)
{
    DEBUG_MSG("===================Zappy Server===================");
    DEBUG("port = %d", params->port);
    DEBUG("width = %d", params->map_width);
    DEBUG("heigth = %d", params->map_height);
    DEBUG("clients_nb = %d", params->team_capacity);
    DEBUG("freq = %d", params->frequency);
    DEBUG_MSG("Teams:");
    for (size_t i = 0; params->teams[i] != nullptr; i++)
        DEBUG("  - %s: id = [%03zu]", params->teams[i], i);
    DEBUG_MSG("==================================================");
}

bool parse_args(params_t *params, int argc, char *argv[])
{
    for (int opt;;) {
        opt = getopt_long(argc, argv, "hp:x:y:n:c:f:", long_options, nullptr);
        if (opt < 0)
            break;
        if (!arg_dispatcher(params, argv, opt))
            return false;
    }
    if (params->frequency == 0)
        params->frequency = 100;
    if (params->port == 0
        || params->map_width == 0
        || params->map_height == 0
        || params->team_capacity == 0
        || params->teams == nullptr
    )
        return fprintf(stderr, "%s", SERVER_USAGE), false;
    DEBUG_CALL(print_params, params);
    return true;
}
