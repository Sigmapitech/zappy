#include <limits.h>
#include <stdlib.h>

#include "compass.h"
#include "data_structure/ring_buffer.h"

Test(ring_buffer, heavy_load)
{
    ring_buffer_t rb = { };
    char *p = "plop";

    ring_buffer_init(&rb);
    for (size_t i = 0; i < 10'000'000; i++) {
        if (rand() < (INT_MAX / 3))
            ring_buffer_push(&rb, p);
        else
            ring_buffer_pop(&rb);
    }
    assert("It should have not crahsed", true);
}

Test(ring_buffer, hold_10_elements)
{
    ring_buffer_t rb = { };
    char *arr = "012345789";

    ring_buffer_init(&rb);
    for (size_t i = 0; i < 10; i++)
        ring_buffer_push(&rb, &arr[i]);
    ring_buffer_push(&rb, nullptr);
    for (size_t i = 0; i < 10; i++)
        ring_buffer_push(&rb, "ono");

    for (size_t i = 0; i < 10; i++)
        assert("has kept element, in order", *ring_buffer_pop(&rb) == arr[i]);
}
