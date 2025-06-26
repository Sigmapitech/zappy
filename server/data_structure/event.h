#ifndef EVENT_H_
    #define EVENT_H_

    #include <stdbool.h>
    #include <stddef.h>
    #include <stdint.h>

/**
 * @brief Maximum number of words in a command.
 *
 */
static constexpr const int COMMAND_WORD_COUNT = 5;
/**
 * @brief Maximum number of words in a command for AI clients.
 *
 */
static constexpr const int EVENT_SERVER_ID = -1;

constexpr const int CLIENT_DEAD = -2;

/**
 * @brief Splits a command string into words.
 *
 * @param buff
 * @param argv
 * @param command_len
 * @return true
 * @return false
 */
bool command_split(char *buff, char *argv[static COMMAND_WORD_COUNT],
    size_t command_len);

/**
 * @brief Structure representing an event in the server.
 *
 */
typedef struct {
    uint64_t timestamp; // Timestamp of the event in milliseconds
    int client_idx;
    int client_id;
    uint8_t arg_count;
    union {
        char *command[COMMAND_WORD_COUNT]; // Command words for the event
        char *action[COMMAND_WORD_COUNT]; // Action words for the event
    };
} event_t;

/**
 * @brief Structure representing a heap of events.
 *
 */
typedef struct {
    event_t *buff; // Pointer to the array of events
    size_t nmemb; // Number of events currently in the heap
    size_t capacity; // Maximum number of events the heap can hold
} event_heap_t;

/**
 * @brief Initializes an event heap.
 *
 * @param heap
 * @return true
 * @return false
 */
bool event_heap_init(event_heap_t *heap);
/**
 * @brief Frees the memory allocated for an event heap.
 *
 * @param heap
 */
void event_heap_free(event_heap_t *heap);
/**
 * @brief Pushes an event onto the event heap.
 *
 * @param heap
 * @param event
 * @return true
 * @return false
 */
bool event_heap_push(event_heap_t *heap, const event_t *event);
/**
 * @brief Pops the top event from the event heap.
 *
 * @param heap
 * @return event_t
 */
event_t event_heap_pop(event_heap_t *heap);
/**
 * @brief Peeks at the top event in the event heap without removing it.
 *
 * @param heap
 * @return const event_t*
 */
const event_t *event_heap_peek(const event_heap_t *heap);

/**
 * @brief Checks if the event heap is empty.
 *
 * @param heap
 * @return true
 * @return false
 */
static inline bool event_heap_is_empty(const event_heap_t *heap)
{
    return heap->nmemb == 0;
}

#endif /* EVENT_H_ */
