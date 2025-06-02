#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "compass.h"

static
const char NAME[] = "\033[38;5;189mCom\033[3mpass\033[23m\033[38;5;103m";

static bool IS_LAST_TEST = false;

extern callback TEST_SECTION_START;
extern callback TEST_SECTION_STOP;

enum test_state_e {
    T_FORK_FAILURE = 1,
    T_IS_CHILD = 2,
    T_FAILURE = 4,
    T_SUCCESS = 8
};

static
enum test_state_e print_test_results(int status)
{
    if (status) {
        fprintf(stderr,
            "\033[48;5;210m\033[38;5;232mCRASHED" RESET
            " with status\033[38;5;248m: " YELLOW
            "%d" RESET "\n\n", status);
        return T_FAILURE;
    }
    printf("\n");
    return T_SUCCESS;
}

static
enum test_state_e run_test(callback *f)
{
    int status;
    pid_t pid = fork();

    if (pid < 0)
        return T_FORK_FAILURE;
    if (!pid) {
        f->func();
        IS_LAST_TEST = true;
        assert_debug("reach end of test", "", __FILE__, __LINE__);
        return T_IS_CHILD;
    }
    wait(&status);
    return print_test_results(status);
}

int main(void)
{
    int status = T_SUCCESS;

    fprintf(stderr, "\033[38;5;103m"
        "┌────────────────────────────┐\n"
        "│          %s           │\n"
        "└────────────────────────────┘\n\n", NAME);
    for (callback *f = &TEST_SECTION_START; f != &TEST_SECTION_STOP; f++) {
        if (f->func == NULL)
            continue;
        fprintf(stderr,
            "\033[38;5;103m╤══ \033[38;5;75m%s" RESET ":\n", f->name);
        status |= run_test(f);
        if (status & T_IS_CHILD)
            break;
        if (status & T_FORK_FAILURE)
            return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void assert_debug(
    char const *msg,
    char const *expr,
    char const *file,
    int line)
{
    char const *filename = strrchr(file, '/');

    if (filename == NULL)
        filename = file;
    else
        filename++;
    fprintf(
        stderr,
        "\033[38;5;103m%s" BLUE "\033[4m%s" RESET
        "\033[38;5;248m:" PURPLE "%d" RESET " %s",
        (IS_LAST_TEST ? "└── " : "├ "), filename, line, msg);
    if (*expr != '\0')
        fprintf(stderr, "\033[3m\033[38;5;103m // %s", expr);
    fprintf(stderr, RESET "\n");
}

#define $ // fix nested function false positive report
void assert_impl(bool res) $
{
    if (!res)
        fprintf(
            stderr,
            "\033[38;5;103m│ └ \033[38;5;197m"
            "🯀  Assertion failed\033[0m\n"
        );
}

Test(compass, is_properly_setup)
{
    assert("this test should pass", true);
}
