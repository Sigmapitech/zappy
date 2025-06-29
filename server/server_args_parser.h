#ifndef ARGS_H_
    #define ARGS_H_

    #include <limits.h>
    #include <stdbool.h>
    #include <stddef.h>
    #include <stdint.h>

    // Maximum number of teams allowed including GRAPHIC
    #define TEAM_COUNT_LIMIT 1 << (CHAR_BIT * sizeof (char))

/**
 * @brief Structure to hold command line parameters for the server.
 */
typedef struct params_s {
    char **teams; // Team names, NULL-terminated
    uint8_t registered_team_count;
    uint16_t frequency; // Range between 1 and 10000
    uint8_t map_width; // Range between 10 and 42
    uint8_t map_height; // Range between 10 and 42
    uint16_t port; // Range between 1024 and 65535
    uint8_t team_capacity; // Range between 1 and 200
    bool help; // Display help message
} params_t;

extern const char SERVER_USAGE[];   // Usage message for the server

/**
 * @brief Parses command line arguments and fills the params structure.
 *
 * @param params
 * @param argc
 * @param argv
 * @return true if all arguments are valid and parsed successfully
 * @return false if there are invalid arguments or if malloc fails
 */
bool parse_args(params_t *params, int argc, char *argv[]);

#endif /* !ARGS_H_ */
