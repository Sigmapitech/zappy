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
bool send_ai_team_assignment_respone(
    server_t *srv, client_state_t *client,
    size_t team_id)
{
    unsigned int count = 0;

    DEBUG("Client %d assigned to the team with id %zu", client->fd, team_id);
    for (size_t i = 0; i < srv->eggs.nmemb; i++)
        count += srv->eggs.buff[i].team_id == team_id
            && srv->eggs.buff[i].hatch <= get_timestamp();
    if (count == 0)
        return vappend_to_output(srv, client, "ko\n"), false;
    client->team_id = team_id;
    vappend_to_output(srv, client, "%u\n%hhu %hhu\n",
        count - 1, srv->map_width, srv->map_height);
    for (size_t i = 0; i < srv->eggs.nmemb; i++)
        if (srv->eggs.buff[i].team_id == team_id) {
            client->x = srv->eggs.buff[i].x;
            client->y = srv->eggs.buff[i].y;
            srv->eggs.buff[i] = srv->eggs.buff[srv->eggs.nmemb - 1];
            srv->eggs.nmemb--;
            return true;
        }
    __builtin_unreachable();
}

static
char *serialize_inventory(inventory_t *inv)
{
    static constexpr const uint8_t BUFFER_SIZE = 128;
    static char buffer[BUFFER_SIZE];

    snprintf(buffer, sizeof(buffer), "%u %u %u %u %u %u %u",
        inv->food, inv->linemate, inv->deraumere, inv->sibur,
        inv->mendiane, inv->phiras, inv->thystame);
    return buffer;
}

static
void send_players_info(server_t *srv, client_state_t *client)
{
    for (size_t i = 0; i < srv->cstates.nmemb; i++) {
        if (srv->cstates.buff[i].team_id == GRAPHIC_TEAM_ID
            || srv->cstates.buff[i].team_id == INVALID_TEAM_ID)
            continue;
        vappend_to_output(srv, client, "pnw #%zu %hhu %hhu %hu %s\n",
            i, srv->cstates.buff[i].x, srv->cstates.buff[i].y,
            srv->cstates.buff[i].tier,
            srv->team_names[srv->cstates.buff[i].team_id]);
        vappend_to_output(srv, client, "pin #%zu %s\n", i, serialize_inventory(
            &srv->cstates.buff[i].inv));
        vappend_to_output(srv, client, "plv #%zu %hhu\n", i,
            srv->cstates.buff[i].tier);
    }
}

static
bool send_gui_team_assignment_respone(server_t *srv, client_state_t *client)
{
    client->team_id = GRAPHIC_TEAM_ID;
    DEBUG("Client %d assigned to GRAPHIC team", client->fd);
    vappend_to_output(srv, client, "msz %hhu %hhu\nsgt %hu\n",
        srv->map_width, srv->map_height, srv->frequency);
    for (size_t y = 0; y < srv->map_height; y++)
        for (size_t x = 0; x < srv->map_width; x++)
            vappend_to_output(srv, client, "bct %zu %zu %s\n",
                x, y, serialize_inventory(&srv->map[y][x]));
    for (size_t i = 0; srv->team_names[i] != nullptr; i++)
        vappend_to_output(srv, client, "tna %s\n", srv->team_names[i]);
    send_players_info(srv, client);
    for (size_t i = 0; i < srv->eggs.nmemb; i++)
        vappend_to_output(srv, client, "enw #%zu #-1 %hhu %hhu\n", i,
            srv->eggs.buff[i].x, srv->eggs.buff[i].y);
    return true;
}

static
bool handle_team(server_t *srv, client_state_t *client,
    char *split[static COMMAND_WORD_COUNT])
{
    if (client->team_id != INVALID_TEAM_ID)
        return false;
    if (!strcmp(split[0], GRAPHIC_COMMAND))
        return send_gui_team_assignment_respone(srv, client);
    for (size_t i = 0; srv->team_names[i] != nullptr; i++)
        if (!strcmp(srv->team_names[i], split[0]))
            return send_ai_team_assignment_respone(srv, client, i);
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
void unknown_command(server_t *srv, client_state_t *client,
    const char *command)
{
    event_t event = {.trigger_fd = client->fd, .command =
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
