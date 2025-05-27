#include <stdio.h>
#include <stdlib.h>

#include "args_parser.h"

/**
 * @brief Program entry point.
 * @param argc size of argv
 * @param argv command line arguments
 * @return int Program exit status
 */
[[gnu::weak]]
int main(int argc, char *argv[])
{
    params_t params = {0};

    if (!parse_args(&params, argc, argv))
        return 84;
    if (params.help)
        return printf("%s\n", USAGE), free((void *)params.teams), EXIT_SUCCESS;
    free((void *)params.teams);
    return EXIT_SUCCESS;
}
