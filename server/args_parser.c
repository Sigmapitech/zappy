#include <getopt.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args_parser.h"
#include "debug.h"

const char *USAGE = (
    "Usage: ./server [OPTIONS]\n"
    "Options:\n"
    "  -h, --help                Show this help message and exit\n"
    "  -p, --port <port>         Set the port number\n"
    "  -x, --width <width>       Set the width of the map\n"
    "  -y, --height <height>     Set the height of the map\n"
    "  -n, --names <team1> ...   Set team names\n"
    "  -c, --client-number <num> Set the number of clients per team\n"
    "  -f, --freq <frequency>    reciprocal of time unit (default: 100)\n"
);

// Helper message to avoid long strings in the code
const char INVALID_ARG[] = "Invalid option or missing argument\n%s\n";

// Structure to hold the command line parameters, to be used by getopt_long
static const struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"port", required_argument, NULL, 'p'},
    {"width", required_argument, NULL, 'x'},
    {"height", required_argument, NULL, 'y'},
    {"names", required_argument, NULL, 'n'},
    {"client-number", required_argument, NULL, 'c'},
    {"freq", required_argument, NULL, 'f'},
    {NULL, 0, NULL, 0}
};

/**
 * @brief Parses team names from command line arguments.
 *
 * This function extracts team names from the command line arguments,
 * ensuring that the name "GRAPHIC" is not used, as it is reserved.
 *
 * @param argv list of arguments
 * @param idx index in argv to start parsing teams
 * (should be the index of the first team name)
 * @return char** list of team names, NULL-terminated
 * @note
 * The function will allocate memory for the team names.
 * The caller is responsible for freeing the memory.
 */
static char **parse_teams(char *argv[], int *idx)
{
    size_t i = 0;
    char **teams = NULL;

    if (argv[*idx] == NULL)
        return NULL;
    for (; argv[*idx + i] && strcspn(argv[*idx + i], "-") != 0; i++);
    teams = (char **)malloc((i + 1) * sizeof(*teams));
    if (teams == (char **)NULL)
        return NULL;
    for (size_t j = 0; j < i; j++) {
        if (!strcmp(argv[*idx + j], "GRAPHIC")) {
            fprintf(stderr, "Invalid team name: 'GRAPHIC' is reserved.\n");
            free((void *)teams);
            return NULL;
        }
        teams[j] = argv[*idx + j];
    }
    *idx += i;
    teams[i] = NULL;
    return teams;
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
    long value = strtoul(arg, &endptr, 10);

    if (*endptr != '\0' || value < min || value > max) {
        fprintf(stderr,
            "Invalid value for %s: %s (must be between %u and %u)\n",
            name, arg, min, max
        );
        return 0;
    }
    return (uint16_t)value;
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
        case 'p':
            params->port = parse_number_arg(arg, "p", 1024, 65535);
            break;
        case 'x':
            params->width = parse_number_arg(arg, "x", 10, 42);
            break;
        case 'y':
            params->height = parse_number_arg(arg, "y", 10, 42);
            break;
        case 'c':
            params->team_capacity = parse_number_arg(arg, "c", 1, 200);
            break;
        case 'f':
            params->frequency = parse_number_arg(arg, "f", 1, 10000);
            break;
        default:
            return fprintf(stderr, INVALID_ARG, USAGE), false;
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
            params->teams = parse_teams(argv, &optind);
            if (!params->teams) {
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
void print_params(const params_t *params)
{
    printf("===================Zappy Server===================\n");
    printf("port = %d\n", params->port);
    printf("width = %d\n", params->width);
    printf("heigth = %d\n", params->height);
    printf("clients_nb = %d\n", params->team_capacity);
    printf("freq = %d\n", params->frequency);
    printf("Teams:\n");
    for (size_t i = 0; params->teams[i] != NULL; i++)
        printf("  - %s\n", params->teams[i]);
    printf("==================================================\n");
}

bool parse_args(params_t *params, int argc, char *argv[])
{
    int opt;

    do {
        opt = getopt_long(argc, argv, "hp:x:y:n:c:f:", long_options, NULL);
        if (opt == -1)
            break;
        if (!arg_dispatcher(params, argv, opt))
            return false;
    } while (true);
    if (params->port == 0 || params->width == 0 || params->height == 0 ||
        params->team_capacity == 0 || params->teams == (char **)NULL)
        return free((void *)params->teams), false;
    if (params->frequency == 0)
        params->frequency = 100;
    DEBUG_CALL(print_params, params);
    return true;
}
