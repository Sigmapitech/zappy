#include <stdio.h>
#include <string.h>

#include "client.h"
#include "server.h"

static constexpr const uint64_t INITIAL_STOMACH_FILL = 1260;
static const char *GRAPHIC_COMMAND = "GRAPHIC";

static
bool assign_ai_egg_data(server_t *srv, client_state_t *client, size_t team_id)
{
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
bool assign_ai_data(server_t *srv, client_state_t *client, size_t team_id)
{
    double interval_sec = (double)INITIAL_STOMACH_FILL / srv->frequency;
    size_t new_sec = (size_t)interval_sec;
    size_t new_usec = (size_t)((interval_sec - new_sec) * MICROSEC_IN_SEC);
    event_t event = {add_time(get_timestamp(), new_sec, new_usec),
        client - srv->cstates.buff, .command = {"player_death"}};

    DEBUG("Player death incoming at %lu.%06lu sec since server start",
        (event.timestamp - srv->start_time) / MICROSEC_IN_SEC,
        (event.timestamp - srv->start_time) % MICROSEC_IN_SEC);
    client->team_id = team_id;
    if (!event_heap_push(&srv->events, &event)) {
        srv->is_running = false;
        return false;
    }
    return assign_ai_egg_data(srv, client, team_id);
}

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
    vappend_to_output(srv, client, "%u\n%hhu %hhu\n",
        count - 1, srv->map_width, srv->map_height);
    return assign_ai_data(srv, client, team_id);
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
