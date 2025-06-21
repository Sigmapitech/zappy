#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "args_parser.h"
#include "server.h"

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
 * @brief Program entry point.
 * @param argc size of argv
 * @param argv command line arguments
 * @return int Program exit status
 */
[[gnu::weak]]
int main(int argc, char *argv[])
{
    params_t params = {};

    if (!parse_args(&params, argc, argv))
        return EXIT_TEK_FAILURE;
    if (params.help)
        return printf("%s\n", SERVER_USAGE), free(params.teams), EXIT_SUCCESS;
    if (!server_run(&params, get_timestamp()))
        return free(params.teams), EXIT_TEK_FAILURE;
    free(params.teams);
    return EXIT_SUCCESS;
}
