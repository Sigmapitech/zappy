#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "client.h"
#include "event/handler.h"
#include "server.h"

static const char *RES_NAMES[RES_COUNT] = {
    "food", "linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"
};

static
void rotate(int8_t delta[2], uint8_t direction)
{
    switch (direction) {
        case OR_NORTH:
            delta[1] = -delta[1];
            break;
        case OR_EAST:
            break;
        case OR_SOUTH:
            delta[0] = -delta[0];
            break;
        case OR_WEST:
            delta[0] = -delta[0];
            delta[1] = -delta[1];
            break;
        default:
            return;
    }
}

static
void fill_coords(
    uint8_t coords[][2], size_t, server_t *srv, client_state_t *cs)
{
    int8_t delta[2];
    size_t idx = 0;

    for (uint8_t l = 0; l < cs->tier + 1; l++) {
        for (int i = -l; i < l + 1; i++) {
            delta[0] = i;
            delta[1] = l;
            rotate(delta, cs->orientation);
            coords[idx][0] = (cs->x + delta[0]) % srv->map_width;
            coords[idx][1] = (cs->y + delta[1]) % srv->map_height;
            idx++;
        }
    }
}

static
uint16_t get_player_on_tile(
    server_t *srv, int8_t x, int8_t y)
{
    uint16_t count = 0;

    for (size_t i = 0; i < srv->cstates.nmemb; i++) {
        if (srv->cstates.buff[i].x == x && srv->cstates.buff[i].y == y
            && srv->cstates.buff[i].team_id != GRAPHIC_TEAM_ID
            && srv->cstates.buff[i].team_id != INVALID_TEAM_ID) {
            count++;
        }
    }
    return count;
}

static
void serialiaze_tile(
    server_t *srv, client_state_t *cs, uint8_t coords[][2], size_t idx)
{
    inventory_t *tile = &srv->map[coords[idx][1]][coords[idx][0]];
    uint16_t players_on_tile =
        get_player_on_tile(srv, coords[idx][0], coords[idx][1]);

    for (size_t i = 0; i < players_on_tile; i++) {
        if (i != 0)
            append_to_output(srv, cs, " ");
        append_to_output(srv, cs, "player");
    }
    for (size_t i = 0; i < RES_COUNT; i++) {
        if (tile->qnts[i] == 0)
            continue;
        for (size_t j = 0; j < tile->qnts[i]; j++) {
            append_to_output(srv, cs,
                j != 0 || i != 0 || players_on_tile != 0 ? " " : "");
            vappend_to_output(srv, cs, "%s", RES_NAMES[i]);
        }
    }
}

bool player_look_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = srv->cstates.buff + event->client_id;
    uint8_t view = (cs->tier + 1) * (cs->tier + 1);
    uint8_t coords[view][2];

    fill_coords(coords, view, srv, cs);
    append_to_output(srv, cs, "[");
    for (size_t i = 0; i < view; i++) {
        if (i != 0)
            append_to_output(srv, cs, ",");
        serialiaze_tile(srv, cs, coords, i);
    }
    append_to_output(srv, cs, "]\n");
    return true;
}
