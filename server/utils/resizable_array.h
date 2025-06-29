#ifndef RESIZABLE_ARRAY_H_
    #define RESIZABLE_ARRAY_H_

    #include <stdlib.h>

/**
 * @brief Structure representing a resizable array.
 *
 */
typedef struct {
    char *buff; // Pointer to the array of elements
    size_t nmemb; // Number of elements currently in the array
    size_t capacity; // Maximum number of elements the array can hold
} resizable_array_t;

bool sized_struct_ensure_capacity(
    resizable_array_t *arr, size_t requested, size_t objsize
);

#endif /* !RESIZABLE_ARRAY_H_ */
