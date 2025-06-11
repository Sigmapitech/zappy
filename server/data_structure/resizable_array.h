#ifndef RESIZABLE_ARRAY_H_
    #define RESIZABLE_ARRAY_H_

    #include <stdlib.h>

typedef struct {
    char *buff;
    size_t nmemb;
    size_t capacity;
} resizable_array_t;

bool sized_struct_ensure_capacity(
    resizable_array_t *arr, size_t requested, size_t objsize
);

#endif /* !RESIZABLE_ARRAY_H_ */
