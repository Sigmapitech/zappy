#ifndef MACRO_UTILS_H
    #define MACRO_UTILS_H

    #define CAT(x) #x
    #define XCAT(x) CAT(x)

    #define LENGTH_OF(arr) (sizeof (arr) / sizeof *(arr))
    #define SSTR_LEN(sstr) (LENGTH_OF(sstr) - 1)

#endif
