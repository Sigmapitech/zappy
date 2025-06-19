#ifndef SERVER_H
    #define SERVER_H

    #include <limits.h>
    #include <stdbool.h>
    #include <stddef.h>
    #include <stdint.h>
    #include <sys/time.h>

    #include "args_parser.h"
    #include "data_structure/event.h"
    #include "data_structure/resizable_array.h"
    #include "debug.h"

    #define MAP_MAX_SIDE_SIZE 42

    #define LIKELY(cond) (__builtin_expect(!!(cond), 1))
    #define UNLIKELY(cond) (__builtin_expect(!!(cond), 0))

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

/**
 * @brief Maximum number of teams allowed in the server.
 *
 */
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

/**
 * @brief Structure representing a client state in the server.
 *
 */
typedef struct {
    resizable_array_t input;
    resizable_array_t output;
    inventory_t inv;
    uint8_t team_id;
    uint8_t x;
    uint8_t y;
    uint8_t tier;
    uint8_t orientation;
    uint16_t id;
    int fd;
    size_t in_buff_idx;
    size_t out_buff_idx;
} client_state_t;

/**
 * @brief Structure representing an egg in the server.
 *
 */
typedef struct {
    uint32_t hatch;
    uint8_t team_id;
    uint8_t x;
    uint8_t y;
} egg_t;

// For the next 3 structures, we use a resizable array pattern
// to manage dynamic arrays of client states, eggs, and poll file descriptors.
// It requires the following memory layout in order to work:
// - buff: pointer to the allocated memory
// - nmemb: number of elements currently in the array
// - capacity: total capacity of the allocated memory
// This allows us to use the same functions for all three types of arrays.

/**
 * @brief Structure representing an array of client states.
 *
 */
typedef struct {
    client_state_t *buff;
    size_t nmemb;
    size_t capacity;
} client_state_array_t;

/**
 * @brief Structure representing an array of eggs.
 *
 */
typedef struct {
    egg_t *buff;
    size_t nmemb;
    size_t capacity;
} egg_array_t;

/**
 * @brief Structure representing an array of poll file descriptors.
 *
 */
typedef struct {
    pollfd_t *buff;
    size_t nmemb;
    size_t capacity;
} pollfd_array_t;

/**
 * @brief Structure representing the server state.
 *
 */
typedef struct {
    int self_fd;
    volatile bool is_running;
    client_state_array_t cstates;
    egg_array_t eggs;
    pollfd_array_t pfds;
    char *team_names[TEAM_COUNT_LIMIT];
    uint8_t map_height;
    uint8_t map_width;
    inventory_t total_item_in_map;
    inventory_t map[MAP_MAX_SIDE_SIZE][MAP_MAX_SIDE_SIZE];
    event_heap_t events;
    uint64_t start_time;
    uint16_t frequency; // reciprocal of time unit
    uint16_t ia_id_counter; // Counter for AI clients
} server_t;

/**
 * @brief Initializes the server with the given parameters.
 *
 * @param p
 * @param timestamp
 * @return true
 * @return false
 */
bool server_run(params_t *p, uint64_t timestamp);
/**
 * @brief Initializes the server state.
 *
 * @param srv
 */
void server_process_events(server_t *srv);
/**
 * @brief Initializes the server state.
 *
 * @param srv
 * @param timeout
 */
void process_poll(server_t *srv, uint64_t timeout);
/**
 * @brief Processes file descriptors and client buffers in the server.
 *
 * @param srv
 */
void process_fds(server_t *srv);
/**
 * @brief Processes the clients' input buffers in the server.
 *
 * @param srv
 */
void process_clients_buff(server_t *srv);

/**
 * @brief Computes the timeout for the next event in the server.
 *
 */
static constexpr const int MICROSEC_IN_SEC = 1000000;
/**
 * @brief Computes the timeout for the next event in the server.
 *
 */
static constexpr const int MILISEC_IN_SEC = 1000;

/**
 * @brief Get the timestamp object
 *
 * @return uint64_t
 */
static inline uint64_t get_timestamp(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (uint64_t)((tv.tv_sec * MICROSEC_IN_SEC) + tv.tv_usec);
}

/**
 * @brief Adds a time interval to a timestamp.
 *
 * @param timestamp
 * @param sec
 * @param usec
 * @return uint64_t
 */
static inline uint64_t add_time(
    uint64_t timestamp, uint64_t sec, uint64_t usec
)
{
    return timestamp + (sec * MICROSEC_IN_SEC) + usec;
}

/**
 * @brief Computes the timeout for the next event in the server.
 *
 * @param srv
 * @return int32_t
 */
static inline int32_t compute_timeout(server_t *srv)
{
    int64_t current_time = get_timestamp();
    int64_t next_event_time = event_heap_peek(&srv->events)->timestamp;
    int32_t diff = (next_event_time - current_time);
    int32_t ms = diff / MILISEC_IN_SEC;

    if (ms > 0) {
        DEBUG("Timeout for next event: %u ms; diff µs: %d",
            ms, diff - (MILISEC_IN_SEC * ms));
    }
    return ms;
}
#endif
