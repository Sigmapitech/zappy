#ifndef RING_BUFFER_H
    #define RING_BUFFER_H

    #include <stdbool.h>
    #include <stddef.h>

    #define MAX_CONCURRENT_REQUESTS 10

typedef struct {
    char *data[MAX_CONCURRENT_REQUESTS];
    size_t head;
    size_t tail;
    size_t count;
} ring_buffer_t;

static inline
void ring_buffer_init(ring_buffer_t *rb)
{
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}

static inline
bool ring_buffer_push(ring_buffer_t *rb, char *item)
{
    if (rb->count == MAX_CONCURRENT_REQUESTS)
        return false;
    rb->data[rb->tail] = item;
    rb->tail = (rb->tail + 1) % MAX_CONCURRENT_REQUESTS;
    rb->count++;
    return true;
}

static inline
char *ring_buffer_pop(ring_buffer_t *rb)
{
    void *item;

    if (rb->count == 0)
        return nullptr;
    item = rb->data[rb->head];
    rb->head = (rb->head + 1) % MAX_CONCURRENT_REQUESTS;
    rb->count--;
    return item;
}

#endif
