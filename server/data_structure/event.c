#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "event.h"
#include "resizable_array.h"

static void swap(event_t *a, event_t *b)
{
    event_t tmp = *a;

    *a = *b;
    *b = tmp;
}

static void heapify_up(event_heap_t *heap, int idx)
{
    int parent;

    while (idx > 0) {
        parent = (idx - 1) / 2;
        if (heap->buff[parent].timestamp <= heap->buff[idx].timestamp)
            break;
        swap(&heap->buff[parent], &heap->buff[idx]);
        idx = parent;
    }
}

static void heapify_down(event_heap_t *heap, size_t idx)
{
    size_t left;
    size_t right;
    size_t smallest;

    while (true) {
        left = 2 * idx + 1;
        right = 2 * idx + 2;
        smallest = idx;
        if (left < heap->nmemb
            && heap->buff[left].timestamp < heap->buff[smallest].timestamp)
            smallest = left;
        if (right < heap->nmemb
            && heap->buff[right].timestamp < heap->buff[smallest].timestamp)
            smallest = right;
        if (smallest == idx)
            break;
        swap(&heap->buff[idx], &heap->buff[smallest]);
        idx = smallest;
    }
}

bool event_heap_init(event_heap_t *heap)
{
    heap->buff = nullptr;
    heap->nmemb = 0;
    heap->capacity = 0;
    return sized_struct_ensure_capacity(
        (resizable_array_t *)heap, 0, sizeof(event_t));
}

void event_heap_free(event_heap_t *heap)
{
    for (size_t i = 0; i < heap->nmemb; i++) {
        for (size_t j = 0; heap->buff[i].command[j] != nullptr; j++) {
            free(heap->buff[i].command[j]);
            heap->buff[i].command[j] = nullptr;
        }
    }
    free(heap->buff);
    heap->buff = nullptr;
    heap->nmemb = 0;
    heap->capacity = 0;
}

bool event_heap_push(event_heap_t *heap, const event_t *event)
{
    size_t i = 0;

    if (!sized_struct_ensure_capacity(
        (resizable_array_t *)heap, 1, sizeof(event_t)))
        return false;
    for (; event->command[i] != nullptr; i++) {
        heap->buff[heap->nmemb].command[i] = strdup(event->command[i]);
        if (heap->buff[heap->nmemb].command[i] == nullptr) {
            DEBUG_MSG("Failed to allocate memory for command string");
            return false;
        }
    }
    heap->buff[heap->nmemb].command[i] = nullptr;
    heap->buff[heap->nmemb].arg_count = i;
    heap->buff[heap->nmemb].client_idx = event->client_idx;
    heap->buff[heap->nmemb].timestamp = event->timestamp;
    heapify_up(heap, heap->nmemb);
    heap->nmemb++;
    return true;
}

event_t event_heap_pop(event_heap_t *heap)
{
    event_t ret = {0};

    if (heap->nmemb == 0)
        return ret;
    ret = heap->buff[0];
    heap->nmemb--;
    if (heap->nmemb > 0) {
        for (size_t i = 0; heap->buff[0].command[i] != nullptr; i++) {
            free(heap->buff[0].command[i]);
            heap->buff[0].command[i] = nullptr;
        }
        heap->buff[0] = heap->buff[heap->nmemb];
        heapify_down(heap, 0);
    }
    return heap->buff[0];
}

const event_t *event_heap_peek(const event_heap_t *heap)
{
    if (heap->nmemb == 0) {
        DEBUG_MSG("Event heap is empty, Can't peek");
        return nullptr;
    }
    return &heap->buff[0];
}
