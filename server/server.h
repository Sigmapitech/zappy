#ifndef SERVER_H
    #define SERVER_H

    #include <limits.h>
    #include <stdbool.h>
    #include <stddef.h>
    #include <stdint.h>
    #include <sys/time.h>

    #include "args_parser.h"
    #include "debug.h"

    #define MAP_MAX_SIDE_SIZE 42

enum {
    RES_FOOD,
    RES_LINEMATE,
    RES_DERAUMERE,
    RES_SIBUR,
    RES_MENDIANE,
    RES_PHIRAS,
    RES_THYSTAME,

    RES_COUNT
};

DEBUG_USED static const char *RES_NAMES[RES_COUNT] = {
    "food", "linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"
};

typedef struct pollfd pollfd_t;

typedef union {
    struct {
        uint32_t linemate;
        uint32_t deraumere;
        uint32_t sibur;
        uint32_t mendiane;
        uint32_t phiras;
        uint32_t thystame;
        uint32_t food;
    };
    uint32_t qnts[RES_COUNT];
} inventory_t;

typedef struct {
    char *buff;
    inventory_t inv;
    uint8_t team_id;
    uint8_t x;
    uint8_t y;
    uint8_t tier;
    uint32_t fd;
} client_state_t;

typedef struct {
    uint32_t hatch;
    uint8_t team_id;
    uint8_t x;
    uint8_t y;
} egg_t;

// For the next 3 structures, we use a resizable array pattern
// to manage dynamic arrays of client states, eggs, and poll file descriptors.
// But for it to work it need to follow this exact memory layout:
// - buff: pointer to the allocated memory
// - nmemb: number of elements currently in the array
// - capacity: total capacity of the allocated memory
// This allows us to use the same functions for all three types of arrays.

typedef struct {
    client_state_t *buff;
    size_t nmemb;
    size_t capacity;
} client_state_array_t;

typedef struct {
    egg_t *buff;
    size_t nmemb;
    size_t capacity;
} egg_array_t;

typedef struct {
    pollfd_t *buff;
    size_t nmemb;
    size_t capacity;
} pollfd_array_t;


typedef struct {
    int self_fd;
    uint16_t client_count;
    volatile bool is_running;
    client_state_array_t cstates;
    egg_array_t eggs;
    pollfd_array_t pfds;
    char *team_names[TEAM_COUNT_LIMIT];
    uint8_t map_height;
    uint8_t map_width;
    inventory_t total_item_in_map;
    inventory_t map[MAP_MAX_SIDE_SIZE][MAP_MAX_SIDE_SIZE];
} server_t;

bool server_run(params_t *p, uint64_t timestamp);

static constexpr const int MIRCOSEC_IN_SEC = 1000000;

static inline uint64_t get_timestamp(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (uint64_t)((tv.tv_sec * MIRCOSEC_IN_SEC) + tv.tv_usec);
}

static inline uint64_t add_time(
    uint64_t timestamp, uint64_t sec, uint64_t usec
)
{
    return timestamp + (sec * MIRCOSEC_IN_SEC) + usec;
}

#endif
