#ifndef DEBUG_MODE_H
    #define DEBUG_MODE_H

// IWYU pragma: always_keep


    #define CFMT(n) "\033[" #n "m"

    #define RESET CFMT(0)
    #define BOLD CFMT(1)

    #define RED CFMT(31)
    #define GREEN CFMT(32)
    #define YELLOW CFMT(33)
    #define BLUE CFMT(34)
    #define PURPLE CFMT(35)
    #define CYAN CFMT(36)

    #ifdef DEBUG_MODE

        #include <stdio.h>  // IWYU pragma: keep

        #define HEAD __FILE_NAME__, __LINE__

        #define HEAD_FMT_FILE BOLD BLUE "%-24s" RESET
        #define HEAD_FMT_LINE ": " BOLD PURPLE "%-3d" RESET ":"

        #define HEAD_FMT(fmt) HEAD_FMT_FILE HEAD_FMT_LINE "  " fmt "\n"

        #define DEBUG_MSG(msg) printf(HEAD_FMT("%s"), HEAD, msg)
        #define DEBUG(fmt, ...) printf(HEAD_FMT(fmt), HEAD, __VA_ARGS__)
        #define DEBUG_RAW_MSG(msg) printf("%s", msg)
        #define DEBUG_RAW(fmt, ...) printf(fmt, __VA_ARGS__)

        #define DEBUG_CALL(func, ...) func(__VA_ARGS__)

        #define DEBUG_USED

        #define _TODO_HEAD YELLOW "TODO" RESET " `%s`"
        #define TODO(...) DEBUG(_TODO_HEAD, #__VA_ARGS__)

    #else
        #define DEBUG_MODE 0

        #define OMIT /* omitted */

        #define DEBUG_MSG(msg) OMIT
        #define DEBUG(fmt, ...) OMIT
        #define DEBUG_RAW_MSG(msg) OMIT
        #define DEBUG_RAW(fmt, ...) OMIT

        #define DEBUG_CALL(func, ...) OMIT
        #define DEBUG_USED __attribute__((unused))

        #define TODO(...) OMIT
    #endif

#endif
