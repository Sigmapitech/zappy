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

struct ai_lut_entry {
    char *command;
    uint64_t time_needed;
};

struct gui_lut_entry {
    char *command;
};

static const struct ai_lut_entry AI_LUT[] = {
    {"Broadcast", 7},
    {"Connect_nbr", 0},
    {"Eject", 42},
    {"Fork", 42},
    {"Forward", 7},
    {"Incantation", 300},
    {"Inventroy", 1},
    {"Left", 7},
    {"Look", 7},
    {"Right", 7},
    {"Set", 7},
    {"Take", 7},
};

static const struct gui_lut_entry GUI_LUT[] = {
    {"msz"},
    {"bct"},
    {"mct"},
    {"tna"},
    {"ppo"},
    {"plv"},
    {"pin"},
    {"sgt"},
    {"sst"},
};

static constexpr const size_t AI_LUT_SIZE = (
    sizeof(AI_LUT) / sizeof(AI_LUT[0])
);
static constexpr const size_t GUI_LUT_SIZE = (
    sizeof(GUI_LUT) / sizeof(GUI_LUT[0])
);

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

//TODO: send starting data to clients
static
bool handle_team(server_t *srv, client_state_t *client,
    char *split[static COMMAND_WORD_COUNT])
{
    if (client->team_id != INVALID_TEAM_ID)
        return false;
    if (!strcmp(split[0], GRAPHIC_COMMAND)) {
        client->team_id = GRAPHIC_TEAM_ID;
        DEBUG("Client %d assigned to GRAPHIC team", client->fd);
        return true;
    }
    for (size_t i = 0; srv->team_names[i] != nullptr; i++) {
        DEBUG("Checking team '%s' against '%s'", srv->team_names[i], split[0]);
        if (!strcmp(srv->team_names[i], split[0])) {
            client->team_id = i;
            DEBUG("Client %d assigned to team '%s' with id %zu",
                client->fd, srv->team_names[i], i);
            return true;
        }
    }
    return false;
}

static
uint64_t get_late_event(server_t *srv, client_state_t *client)
{
    uint64_t late_event = 0;

    for (size_t i = 0; i < srv->events.nmemb; i++) {
        if (srv->events.buff[i].trigger_fd == client->fd
            && srv->events.buff[i].timestamp > late_event) {
            late_event = srv->events.buff[i].timestamp;
        }
    }
    return late_event;
}

static
void event_create(server_t *srv, client_state_t *client,
    char *split[static COMMAND_WORD_COUNT], uint64_t time_needed)
{
    event_t event = {.trigger_fd = client->fd};

    memcpy(event.command, split, sizeof(event.command));
    if (client->team_id != GRAPHIC_TEAM_ID)
        event.timestamp = get_late_event(srv, client) + (time_needed);
    else
        event.timestamp = get_timestamp();
    DEBUG("Creating event for client %d: '%s' at %lu",
        client->fd, event.command[0], event.timestamp);
    if (!event_heap_push(&srv->events, &event))
        srv->is_running = false;
}

static
void handle_command(server_t *srv, client_state_t *client,
    char *split[static COMMAND_WORD_COUNT])
{
    if (client->team_id == INVALID_TEAM_ID) {
        if (!handle_team(srv, client, split))
            append_to_output(srv, client, "ko\n");
        return;
    }
    for (size_t i = 0; i < AI_LUT_SIZE
        && client->team_id != GRAPHIC_TEAM_ID; i++) {
        if (strcmp(AI_LUT[i].command, split[0]) == 0) {
            event_create(srv, client, split, AI_LUT[i].time_needed);
            return;
        }
    }
    for (size_t i = 0; i < GUI_LUT_SIZE
        && client->team_id == GRAPHIC_TEAM_ID; i++) {
        if (strcmp(GUI_LUT[i].command, split[0]) == 0) {
            event_create(srv, client, split, 0);
            return;
        }
    }
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
            continue;
        command_len = strcspn(client->input.buff + client->in_buff_idx, "\n");
        (client->input.buff + client->in_buff_idx)[command_len] = '\0';
        command_split(client->input.buff + client->in_buff_idx,
            split, command_len);
        client->in_buff_idx += command_len + 1;
        handle_command(srv, client, split);
    }
}
