#ifndef EVENT_H_
    #define EVENT_H_

    #include <stdbool.h>
    #include <stddef.h>
    #include <stdint.h>

static constexpr const int COMMAND_WORD_COUNT = 5;
static constexpr const int EVENT_SERVER_ID = -1;
bool command_split(char *buff, char *argv[static COMMAND_WORD_COUNT],
    size_t command_len);

typedef struct {
    uint64_t timestamp;
    int client_id;
    union {
        char *command[COMMAND_WORD_COUNT];
        char *action[COMMAND_WORD_COUNT];
    };
} event_t;

typedef struct {
    event_t *buff;
    size_t nmemb;
    size_t capacity;
} event_heap_t;

bool event_heap_init(event_heap_t *heap);
void event_heap_free(event_heap_t *heap);
bool event_heap_push(event_heap_t *heap, const event_t *event);
event_t event_heap_pop(event_heap_t *heap);
const event_t *event_heap_peek(const event_heap_t *heap);

static inline bool event_heap_is_empty(const event_heap_t *heap)
{
    return heap->nmemb == 0;
}

#endif /* EVENT_H_ */
