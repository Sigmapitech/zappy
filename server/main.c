#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "utils/common_macros.h"

#include "server.h"
#include "server_args_parser.h"

const char SERVER_USAGE[] = {
    "Usage: ./server [OPTIONS]\n"
    "Options:\n"
    "  -h, --help                Show this help message and exit\n"
    "  -p, --port <port>         Set the port number\n"
    "  -x, --width <width>       Set the width of the map\n"
    "  -y, --height <height>     Set the height of the map\n"
    "  -n, --names <team1> ...   Set team names\n"
    "  -c, --client-number <num> Set the number of clients per team\n"
    "  -f, --freq <frequency>    reciprocal of time unit (default: 100)\n"
};

static constexpr const int EXIT_TEK_FAILURE = 84;

/**
 * Have teams to identify server, unassigned and graphic "teams" for the
 * client states sectionning.
 **/
static
const char *RESERVED_TEAM_NAMES[] = {
    "-SERVER",
    "-UNASSIGNED",
    "GRAPHIC"
};

/**
 * @brief Program entry point.
 * @param argc size of argv
 * @param argv command line arguments
 * @return int Program exit status
 */
[[gnu::weak]]
int main(int argc, char *argv[])
{
    char *teams[TEAM_COUNT_LIMIT] = { };
    params_t params = {
        .teams = teams,
        .registered_team_count = LENGTH_OF(RESERVED_TEAM_NAMES)
    };

    memcpy(params.teams, RESERVED_TEAM_NAMES,
        LENGTH_OF(RESERVED_TEAM_NAMES) * sizeof *params.teams);
    if (!parse_args(&params, argc, argv))
        return EXIT_TEK_FAILURE;
    if (params.help)
        return printf("%s\n", SERVER_USAGE), EXIT_SUCCESS;
    if (!server_run(&params, get_timestamp()))
        return EXIT_TEK_FAILURE;
    return EXIT_SUCCESS;
}
