#ifndef MACRO_UTILS_H
    #define MACRO_UTILS_H

    #define CAT(x) #x
    #define XCAT(x) CAT(x)

    #define LENGTH_OF(arr) (sizeof (arr) / sizeof *(arr))
    #define SSTR_LEN(sstr) (LENGTH_OF(sstr) - 1)

    #define VA_COUNT(...) _COUNT(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
    #define _COUNT(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, count, ...) count

#endif
