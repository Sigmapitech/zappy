#include <stdio.h>
#include <string.h>

#include "client.h"
#include "event/names.h"
#include "server.h"

static constexpr const uint64_t INITIAL_FOOD_INVENTORY = 10;
static constexpr const uint8_t FOUR_MASK = 0b11;
static const char *GRAPHIC_COMMAND = "GRAPHIC";

static
void send_guis_player_data(server_t *srv, client_state_t *client, size_t egg)
{
    for (size_t i = 0; i < srv->cstates.nmemb; i++) {
        if (srv->cstates.buff[i].team_id != GRAPHIC_TEAM_ID)
            continue;
        vappend_to_output(srv, &srv->cstates.buff[i],
            "pnw #%d %hhu %hhu %hhu %hhu %s\npin #%d %hhu %hhu %s\nebo #%zu\n",
            client->id, client->x, client->y, client->orientation + 1,
            client->tier, srv->team_names[client->team_id],
            client->id, client->x, client->y,
            serialize_inventory(&client->inv), egg + 1);
    }
}

static
bool assign_ai_egg_data(server_t *srv, client_state_t *client, size_t team_id)
{
    for (size_t i = 0; i < srv->eggs.nmemb; i++)
        if (srv->eggs.buff[i].team_id == team_id) {
            client->x = srv->eggs.buff[i].x;
            client->y = srv->eggs.buff[i].y;
            srv->eggs.buff[i] = srv->eggs.buff[srv->eggs.nmemb - 1];
            srv->eggs.nmemb--;
            send_guis_player_data(srv, client, i);
            return true;
        }
    __builtin_unreachable();
}

static
bool assign_ai_data(server_t *srv, client_state_t *client, size_t team_id)
{
    event_t event = {get_timestamp(), client - srv->cstates.buff,
        .command = {PLAYER_DEATH}};

    DEBUG("Player death incoming at %lu.%06lu sec since server start",
        (event.timestamp - srv->start_time) / MICROSEC_IN_SEC,
        (event.timestamp - srv->start_time) % MICROSEC_IN_SEC);
    client->team_id = team_id;
    client->orientation = (rand() & FOUR_MASK);
    client->id = srv->ia_id_counter;
    srv->ia_id_counter++;
    client->inv.food = INITIAL_FOOD_INVENTORY;
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
void send_players_info(server_t *srv, client_state_t *client)
{
    for (size_t i = 0; i < srv->cstates.nmemb; i++) {
        if (srv->cstates.buff[i].team_id == GRAPHIC_TEAM_ID
            || srv->cstates.buff[i].team_id == INVALID_TEAM_ID)
            continue;
        vappend_to_output(srv, client, "pnw #%hu %hhu %hhu %hu %s\n",
            srv->cstates.buff[i].id, srv->cstates.buff[i].x,
            srv->cstates.buff[i].y, srv->cstates.buff[i].tier,
            srv->team_names[srv->cstates.buff[i].team_id]);
        vappend_to_output(srv, client, "pin #%hu %s\n",
            srv->cstates.buff[i].id, serialize_inventory(
                &srv->cstates.buff[i].inv));
        vappend_to_output(srv, client, "plv #%hu %hhu\n",
            srv->cstates.buff[i].id, srv->cstates.buff[i].tier);
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
