#include <stdint.h>
#include <stdio.h>

#include "client/client.h"
#include "event.h"
#include "handler.h"
#include "names.h"

struct requirement_s {
    inventory_t resources;
    uint8_t player_count;
};

static const struct requirement_s INCANTATION_REQUIREMENTS[] = {
    {{{0, 1, 0, 0, 0, 0, 0}}, 1},
    {{{0, 1, 1, 1, 0, 0, 0}}, 2},
    {{{0, 2, 0, 1, 0, 2, 0}}, 2},
    {{{0, 1, 1, 2, 0, 1, 0}}, 4},
    {{{0, 1, 2, 1, 3, 0, 0}}, 4},
    {{{0, 1, 2, 3, 0, 1, 0}}, 6},
    {{{0, 2, 2, 2, 2, 2, 1}}, 6},
};

static constexpr const size_t INCANTATION = 300;

static
bool has_enough_resources(server_t *srv, uint8_t x, uint8_t y, uint8_t level)
{
    const struct requirement_s *req;
    size_t player_count = 0;

    if (level < 1 || level > 7)
        return false;
    req = &INCANTATION_REQUIREMENTS[level - 1];
    for (size_t i = 0; i < RES_COUNT; i++)
        if (srv->map[y][x].qnts[i] < req->resources.qnts[i])
            return false;
    for (size_t i = srv->cm.idx_of_players; i < srv->cm.count; i++)
        if (srv->cm.clients[i].x == x
            && srv->cm.clients[i].y == y
            && srv->cm.clients[i].tier == level)
            player_count++;
    return player_count >= req->player_count;
}

static
void send_to_participants(server_t *srv, client_state_t *cs,
    const char *message, bool end)
{
    client_state_t *client;

    for (size_t i = srv->cm.idx_of_players; i < srv->cm.count; i++) {
        client = srv->cm.clients + i;
        if (!end && client->is_in_incantation)
            continue;
        if (client->x != cs->x
            || client->y != cs->y
            || client->tier != cs->tier
        )
            continue;
        append_to_output(srv, client, message);
        client->tier += end;
        client->is_in_incantation = !end;
        if (!end)
            continue;
        send_to_guis(srv, "plv %u %hhu\n", client->id, client->tier);
    }
}

static
bool player_incantation_end_schedule(server_t *srv, const event_t *event)
{
    uint64_t interval = (INCANTATION * MICROSEC_IN_SEC) / srv->frequency;
    event_t new_event = {
        .timestamp = get_timestamp() + interval,
        .client_idx = event->client_idx,
        .client_id = event->client_id,
        .command = { PLAYER_END_INCANTATION }
    };

    if (!event_heap_push(&srv->events, &new_event)) {
        perror("Failed to schedule incantation end event");
        return false;
    }
    return true;
}

static
void player_lock_helper(
    server_t *srv,
    client_state_t *cs,
    uint32_t idx,
    const event_t *event
)
{
    uint64_t interval = (INCANTATION * MICROSEC_IN_SEC) / srv->frequency;
    event_t new_event = {
        .timestamp = get_timestamp() + interval,
        .client_idx = idx,
        .client_id = cs->id,
        .command = { PLAYER_LOCK }
    };

    if (event->client_id == (int)cs->id)
        return;
    if (!event_heap_push(&srv->events, &new_event)) {
        perror("Failed to schedule player lock event");
        return;
    }
}

bool player_start_incentation_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = event_get_client(srv, event);

    if (cs == nullptr)
        return false;
    if (event->arg_count != 1
        || !has_enough_resources(srv, cs->x, cs->y, cs->tier))
        return append_to_output(srv, cs, "ko\n"), true;
    send_to_guis(srv, "pic %hhu %hhu %hhu", cs->x, cs->y, cs->tier);
    send_to_participants(srv, cs, "Elevation underway\n", 0);
    for (size_t i = srv->cm.idx_of_players; i < srv->cm.count; i++)
        if (srv->cm.clients[i].x == cs->x
            && srv->cm.clients[i].y == cs->y
            && srv->cm.clients[i].tier == cs->tier
        ) {
            send_to_guis(srv, " #%d", srv->cm.clients[i].id);
            player_lock_helper(srv, &srv->cm.clients[i], i, event);
        }
    send_to_guis(srv, "\n");
    return player_incantation_end_schedule(srv, event);
}

bool player_end_incentation_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = event_get_client(srv, event);
    char buff[64];

    if (cs == nullptr)
        return false;
    if (!has_enough_resources(srv, cs->x, cs->y, cs->tier)) {
        send_to_guis(srv, "pie %hhu %hhu %hhu\n", cs->x, cs->y, cs->tier);
        append_to_output(srv, cs, "ko\n");
        return true;
    }
    snprintf(buff, sizeof(buff), "Current level: %d\n", cs->tier + 1);
    for (size_t i = 0; i < RES_COUNT; i++)
        srv->map[cs->y][cs->x].qnts[i] -=
            INCANTATION_REQUIREMENTS[cs->tier - 1].resources.qnts[i];
    send_to_participants(srv, cs, buff, 1);
    send_to_guis(srv, "pie %hhu %hhu %hhu\n", cs->x, cs->y, cs->tier);
    return true;
}

bool player_lock_handler(server_t *, const event_t *)
{
    return true;
}
