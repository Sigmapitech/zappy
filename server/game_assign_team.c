#include <stdio.h>
#include <string.h>

#include "client/client.h"
#include "game_events/names.h"
#include "server.h"

static constexpr const uint64_t INITIAL_FOOD_INVENTORY = 10;
static constexpr const uint8_t FOUR_MASK = 0b11;
static const char *GRAPHIC_COMMAND = "GRAPHIC";

static
void send_guis_player_data(server_t *srv, client_state_t *client, size_t egg)
{
    for (size_t i = srv->cm.idx_of_gui; i < srv->cm.idx_of_players; i++) {
        vappend_to_output(srv, &srv->cm.clients[i],
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
    event_t event = {
        .timestamp = get_timestamp(),
        .client_idx = client - srv->cm.clients,
        .client_id = client->id,
        .command = {PLAYER_DEATH}
    };

    DEBUG("Player (id: %u) death incoming in %lu ms",
        client->id, (event.timestamp - get_timestamp()) / MILISEC_IN_SEC);
    client->team_id = team_id;
    client->orientation = (rand() & FOUR_MASK);
    client->inv.food = INITIAL_FOOD_INVENTORY;
    client->tier = 1;
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

    client->team_id = team_id;
    client = client_manager_promote(&srv->cm, client - srv->cm.clients);
    if (client == nullptr)
        return false;
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
    for (size_t i = srv->cm.idx_of_players; i < srv->cm.count; i++) {
        vappend_to_output(srv, client, "pnw #%hu %hhu %hhu %hu %s\n",
            srv->cm.clients[i].id, srv->cm.clients[i].x,
            srv->cm.clients[i].y, srv->cm.clients[i].tier,
            srv->team_names[srv->cm.clients[i].team_id]);
        vappend_to_output(srv, client, "pin #%hu %s\n",
            srv->cm.clients[i].id, serialize_inventory(
                &srv->cm.clients[i].inv));
        vappend_to_output(srv, client, "plv #%hu %hhu\n",
            srv->cm.clients[i].id, srv->cm.clients[i].tier);
    }
}

static
bool send_gui_team_assignment_respone(server_t *srv, client_state_t *client)
{
    client->team_id = TEAM_ID_GRAPHIC;
    client = client_manager_promote(&srv->cm, client - srv->cm.clients);
    if (client == nullptr)
        return false;
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
    if (client->team_id != TEAM_ID_UNASSIGNED)
        return false;
    if (!strcmp(split[0], GRAPHIC_COMMAND))
        return send_gui_team_assignment_respone(srv, client);
    for (size_t i = 0; srv->team_names[i] != nullptr; i++)
        if (!strcmp(srv->team_names[i], split[0]))
            return send_ai_team_assignment_respone(srv, client, i);
    return false;
}
