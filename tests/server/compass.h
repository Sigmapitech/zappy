#ifndef TEST_H
    #define TEST_H

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

    #include <stdbool.h>
    #include <sys/types.h>

typedef struct {
    char const *name;
    void (*func)(void);
} callback;

    #define TEST_SECTION_START __start_compass_test
    #define TEST_SECTION_STOP __stop_compass_test

    #define TEST_PROTO(func_name) static void (func_name)(void)

void assert_impl(bool res);
void assert_debug(
    char const *msg,
    char const *expr,
    char const *file,
    int line);

    #define Test(test_suite, test_case)                                       \
TEST_PROTO(test_suite ## __ ## test_case);                                    \
                                                                              \
[[gnu::section("compass_test"), gnu::used]]                                   \
static const callback info_ ## test_suite ## __ ## test_case = {              \
    .name = "\033[38;5;81m" #test_suite                                       \
        "\033[38;5;103m -> \033[38;5;117m" #test_case,                        \
    .func = &(test_suite ## __ ## test_case)                                  \
};                                                                            \
                                                                              \
TEST_PROTO(test_suite ## __ ## test_case)

    #define assert(msg, expr)                                                 \
do {                                                                          \
     assert_debug(msg, (#expr), __FILE__, __LINE__);                          \
     assert_impl((expr));                                                     \
} while (0)

#endif
