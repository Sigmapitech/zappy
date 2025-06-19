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


static
uint64_t get_late_event(server_t *srv, client_state_t *client)
{
    uint64_t late_event = 0;
    int idx = client - srv->cstates.buff;

    for (size_t i = 0; i < srv->events.nmemb; i++) {
        if (srv->events.buff[i].client_id == idx
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
    double interval_sec = (double)time_needed / srv->frequency;
    size_t new_sec = (size_t)interval_sec;
    size_t new_usec = (size_t)((interval_sec - new_sec) * MICROSEC_IN_SEC);
    int idx = client - srv->cstates.buff;
    event_t event = {.client_id = idx};

    memcpy(event.command, split, sizeof(event.command));
    if (client->team_id != GRAPHIC_TEAM_ID)
        event.timestamp = add_time(
            get_late_event(srv, client), new_sec, new_usec);
    else
        event.timestamp = get_timestamp();
    DEBUG("Creating event for client %d: '%s' at %lu",
        client->fd, event.command[0], event.timestamp);
    if (!event_heap_push(&srv->events, &event))
        srv->is_running = false;
}

static
void unknown_command(server_t *srv, client_state_t *client,
    const char *command)
{
    size_t idx = client - srv->cstates.buff;
    event_t event = {.client_id = idx, .command =
        {client->team_id == GRAPHIC_TEAM_ID ? "suc" : "ko"}};

    if (client->team_id != GRAPHIC_TEAM_ID)
        event.timestamp = get_late_event(srv, client);
    else
        event.timestamp = get_timestamp();
    DEBUG("Unknown command '%s' from client %d", command, client->fd);
    if (!event_heap_push(&srv->events, &event)) {
        srv->is_running = false;
        return;
    }
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
    unknown_command(srv, client, split[0]);
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
