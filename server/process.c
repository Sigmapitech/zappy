#include <arpa/inet.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "client.h"
#include "data_structure/event.h"
#include "server.h"

void process_poll(server_t *srv, uint64_t timeout)
{
    int poll_result = poll(srv->pfds.buff, srv->pfds.nmemb, timeout);

    if (poll_result < 0) {
        if (srv->is_running)
            perror("poll failed");
    }
}

void process_fds(server_t *srv)
{
    for (size_t i = 0; i < srv->pfds.nmemb; i++) {
        if ((srv->pfds.buff[i].revents & POLLIN)
            && srv->pfds.buff[i].fd == srv->self_fd)
            add_client(srv);
        if (srv->pfds.buff[i].revents & POLLHUP)
            remove_client(srv, i);
        if ((srv->pfds.buff[i].revents & POLLIN)
            && srv->pfds.buff[i].fd != srv->self_fd)
            read_client(srv, i);
        if ((srv->pfds.buff[i].revents & POLLOUT)
            && srv->pfds.buff[i].fd != srv->self_fd)
            write_client(srv, i);
    }
}

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

static
bool command_split(char *buff, char *argv[static COMMAND_WORD_COUNT],
    size_t command_len)
{
    size_t next;
    size_t i = 1;
    char *ptr = buff;

    for (; *buff == ' '; buff++);
    argv[0] = buff;
    for (; i < COMMAND_WORD_COUNT; i++) {
        if (*buff == '"' && !eat_quoted_argument(&buff, argv, i))
            return false;
        next = strcspn(buff, " ");
        buff += next;
        *buff = '\0';
        if ((size_t)(buff - ptr) == command_len)
            return true;
        for (buff++; *buff == ' '; buff++);
        if (*buff == '\n')
            return true;
        argv[i] = buff;
    }
    return true;
}

static
void split_command(char *command,
    char *dest[static COMMAND_WORD_COUNT], size_t command_len)
{
    char sub[command_len + 1];

    DEBUG("Command '%.*s' with length %zu",
        (int)command_len, command, command_len);
    strncpy(sub, command, command_len);
    sub[command_len] = '\0';
    DEBUG("Splitting command: '%s' with length %zu", sub, command_len);
    command_split(sub, dest, command_len);
}

static
void handle_command(server_t *srv, client_state_t *client,
    char *split[static COMMAND_WORD_COUNT])
{
    DEBUG("Processing command from client %d", client->fd);
    for (size_t i = 0; i < COMMAND_WORD_COUNT; i++)
        DEBUG("Command word %zu: '%s'", i, split[i] ? split[i] : "(null)");
}

void process_clients_buff(server_t *srv)
{
    size_t command_len = 0;
    client_state_t *client = nullptr;
    char *split[COMMAND_WORD_COUNT] = {nullptr};

    if (srv->cstates.nmemb == 0)
        return;
    for (size_t i = 0; i < srv->cstates.nmemb; i++) {
        client = &srv->cstates.buff[i];
        if (client->input.buff == nullptr)
            continue;
        if (!(srv->pfds.buff[i + 1].revents & POLLIN)
            || client->in_buff_idx >= client->input.nmemb)
            return;
        command_len = strcspn(client->input.buff + client->in_buff_idx, "\n");
        split_command(client->input.buff + client->in_buff_idx,
            split, command_len);
        client->in_buff_idx += command_len + 1;
        handle_command(srv, client, split);
    }
}
