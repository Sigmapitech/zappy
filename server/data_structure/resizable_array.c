#include "resizable_array.h"

static constexpr const size_t INITIAL_SIZE = 64;

bool sized_struct_ensure_capacity(
    resizable_array_t *arr, size_t requested, size_t objsize
)
{
    size_t endsize = INITIAL_SIZE;
    void *newp;

    if (arr->capacity == 0 && requested <= INITIAL_SIZE) {
        arr->buff = malloc(INITIAL_SIZE * objsize);
        arr->nmemb = 0;
        arr->capacity = INITIAL_SIZE;
        return arr->buff != nullptr;
    }
    if ((arr->nmemb + requested) < arr->capacity)
        return true;
    for (; endsize < arr->nmemb + requested; endsize <<= 1);
    if (endsize < arr->capacity)
        return true;
    newp = realloc(arr->buff, objsize * endsize);
    if (newp == nullptr)
        return false;
    arr->buff = newp;
    arr->capacity = endsize;
    return true;
}
