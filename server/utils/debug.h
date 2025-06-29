#ifndef DEBUG_MODE_H
    #define DEBUG_MODE_H

// IWYU pragma: always_keep



    #ifndef COLORS_H
        #define COLORS_H

        #define COL_FMT(n) "\033[" #n "m"

        #define RESET COL_FMT(0)
        #define BOLD COL_FMT(1)

        #define RED COL_FMT(31)
        #define GREEN COL_FMT(32)
        #define YELLOW COL_FMT(33)
        #define BLUE COL_FMT(34)
        #define PURPLE COL_FMT(35)
        #define CYAN COL_FMT(36)

    #endif

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
        #define DEBUG_USED [[gnu::unused]]

        #define TODO(...) OMIT
    #endif

#endif
