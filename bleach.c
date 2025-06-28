#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

enum modes {
    CSI = 1, OSC = 2, DCS = 4, NOP = 0
};

static
char *skip_ansi_sequence(char *reader)
{
    int mode = ((*reader == '[') * CSI)
        | ((*reader == ']') * OSC)
        | ((*reader == 'P') * DCS);

    if (mode == NOP)
        return reader;
    for (reader++; *reader != '\0'; reader++) {
        if (mode & OSC && *reader == '\x07')
            return reader + 1;
        if (mode & CSI && *reader >= 0x40 && *reader <= 0x7F)
            return reader + 1;
        if (mode & (OSC | DCS) && *reader == '\x1b' && reader[1] == '\\')
            return reader + 2;
    }
    return reader;
}

static
size_t remove_ansi_sequences(char *line)
{
    char *writer = line;
    char *reader = line;

    while (*reader != '\0') {
        if (*reader == '\x1b') {
            reader = skip_ansi_sequence(reader + 1);
            continue;
        }
        *writer = *reader;
        writer++;
        reader++;
    }
    *writer = '\0';
    return writer - line;
}

int main(void)
{
    char *line = NULL;
    ssize_t len = 0;

    for (size_t n_track;;) {
        len = getline(&line, &n_track, stdin);
        if (len < 0)
            break;
        len = remove_ansi_sequences(line);
        write(STDOUT_FILENO, line, len);
    }
}
