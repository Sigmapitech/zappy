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
#include "event.h"
#include "game_events/names.h"
#include "utils/common_macros.h"
#include "server.h"

static constexpr const size_t MAX_GUI_CMD_LEN = 4;

struct ai_lut_entry {
    char *command;
    uint64_t time_needed;
};

struct gui_lut_entry {
    char command[MAX_GUI_CMD_LEN];
};

static const struct ai_lut_entry AI_LUT[] = {
    {"Broadcast", 7},
    {"Connect_nbr", 0},
    {"Eject", 7},
    {"Fork", 42},
    {"Forward", 7},
    {"Incantation", 0},
    {PLAYER_END_INCANTATION, 300}, // Internal use only
    {"Inventory", 1},
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

static constexpr const size_t AI_LUT_SIZE = LENGTH_OF(AI_LUT);
static constexpr const size_t GUI_LUT_SIZE = LENGTH_OF(GUI_LUT);

static
bool is_in_ai_lut(const char *command)
{
    for (size_t i = 0; i < AI_LUT_SIZE; i++) {
        if (strcmp(AI_LUT[i].command, command) == 0)
            return true;
    }
    return false;
}

static
uint64_t get_late_event(server_t *srv, client_state_t *client)
{
    uint64_t late_event = get_timestamp();
    int idx = client - srv->cm.clients;

    for (size_t i = 0; i < srv->events.nmemb; i++) {
        if (srv->events.buff[i].client_idx == idx
            && is_in_ai_lut(srv->events.buff[i].command[0])
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
    uint64_t interval = (time_needed * MICROSEC_IN_SEC) / srv->frequency;
    event_t event = {
        .client_idx = client - srv->cm.clients, .client_id = client->id
    };

    memcpy(event.command, split, sizeof(event.command));
    for (event.arg_count = 0; event.arg_count < COMMAND_WORD_COUNT
        && event.command[event.arg_count] != nullptr; event.arg_count++);
    if (client->team_id != TEAM_ID_GRAPHIC)
        event.timestamp = get_late_event(srv, client) + interval;
    else
        event.timestamp = get_timestamp();
    DEBUG("Creating event for client %d: '%s' in %lu ms",
        client->fd, event.command[0],
        (event.timestamp - get_timestamp()) / MILISEC_IN_SEC);
    if (!event_heap_push(&srv->events, &event))
        srv->is_running = false;
    if (!strcmp(split[0], PLAYER_FORK))
        send_to_guis(srv, "pfk #%hu\n", client->id);
}

static
void unknown_command(server_t *srv, client_state_t *client,
    const char *command DEBUG_USED)
{
    size_t idx = client - srv->cm.clients;
    event_t event = {
        .client_idx = idx,
        .command = {client->team_id == TEAM_ID_GRAPHIC ? "suc" : "ko"},
        .client_id = client->id
    };

    if (client->team_id != TEAM_ID_GRAPHIC)
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
    if (client->team_id == TEAM_ID_UNASSIGNED) {
        if (!handle_team(srv, client, split))
            append_to_output(srv, client, "ko\n");
        return;
    }
    for (size_t i = 0; i < AI_LUT_SIZE
        && client->team_id != TEAM_ID_GRAPHIC; i++) {
        if (strcmp(AI_LUT[i].command, split[0]) == 0) {
            event_create(srv, client, split, AI_LUT[i].time_needed);
            return;
        }
    }
    for (size_t i = 0; i < GUI_LUT_SIZE
        && client->team_id == TEAM_ID_GRAPHIC; i++) {
        if (strcmp(GUI_LUT[i].command, split[0]) == 0) {
            event_create(srv, client, split, 0);
            return;
        }
    }
    unknown_command(srv, client, split[0]);
}

static
void process_sub_command(server_t *srv, client_state_t *client)
{
    size_t command_len = 0;
    char *split[COMMAND_WORD_COUNT] = {nullptr};

    for (;client->in_buff_idx < client->input.nmemb;) {
        command_len = strcspn(client->input.buff + client->in_buff_idx, "\n");
        (client->input.buff + client->in_buff_idx)[command_len] = '\0';
        command_split(
            client->input.buff + client->in_buff_idx,
            split,
            command_len
        );
        client->in_buff_idx += command_len + 1;
        handle_command(srv, client, split);
    }
}

void process_clients_buff(server_t *srv)
{
    client_state_t *client = nullptr;

    for (size_t i = 1; i < srv->cm.count; i++) {
        client = srv->cm.clients + i;
        if (client->input.buff == nullptr)
            continue;
        if (!(srv->cm.server_pfds[i].revents & POLLIN)
            || client->in_buff_idx >= client->input.nmemb)
            continue;
        process_sub_command(srv, client);
    }
}
