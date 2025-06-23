#include <string.h>

#include "data_structure/event.h"

static
bool eat_quoted_argument(
    char **buffp, char *argv[static COMMAND_WORD_COUNT], size_t i
)
{
    size_t next;
    char *buff = *buffp;

    buff++;
    argv[i - 1]++;
    next = strcspn(buff, "\"");
    if (buff[next] != '\"')
        return false;
    buff += next;
    *buff = ' ';
    *buffp = buff;
    return true;
}

bool command_split(char *buff, char *argv[static COMMAND_WORD_COUNT],
    size_t command_len)
{
    size_t next;
    size_t i = 1;
    char *ptr = buff;

    for (; *buff == ' '; buff++);
    argv[0] = buff;
    for (; i < COMMAND_WORD_COUNT - 1; i++) {
        if (*buff == '"' && !eat_quoted_argument(&buff, argv, i))
            return false;
        next = strcspn(buff, " ");
        buff += next;
        *buff = '\0';
        if ((size_t)(buff - ptr) == command_len)
            return true;
        for (buff++; *buff == ' '; buff++);
        if (*buff == '\0' || *buff == '\n')
            return true;
        argv[i] = buff;
    }
    return false;
}
