#ifndef ARGS_H_
    #define ARGS_H_

    #include <stdbool.h>
    #include <stddef.h>
    #include <stdint.h>

/**
 * @brief Structure to hold command line parameters for the server.
 */
typedef struct params_s {
    bool help;              // Display help message
    uint8_t width;          // Range between 10 and 42
    uint8_t height;         // Range between 10 and 42
    uint8_t team_capacity;  // Range between 1 and 200
    uint16_t port;          // Range between 1024 and 65535
    uint16_t frequency;     // Range between 1 and 10000
    char **teams;           // Teams name (without GRAPHIC), NULL-terminated
} params_t;

extern const char *USAGE;   // Usage message for the server

/**
 * @brief Parses command line arguments and fills the params structure.
 *
 * @param params pointer to the params_t structure to fill
 * @param argc size of argv
 * @param argv command line arguments
 * @return true if all arguments are valid and parsed successfully
 * @return false if there are invalid arguments or if malloc fails
 */
bool parse_args(params_t *params, int argc, char *argv[]);

#endif /* !ARGS_H_ */
