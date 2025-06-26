#include "client.h"
#include "event/handler.h"

static
void player_move(
    server_t *srv, client_state_t *player, orientation_t orientation)
{
    if (orientation == OR_NORTH)
        player->y = (player->y + srv->map_height - 1) % srv->map_height;
    if (orientation == OR_EAST)
        player->x = (player->x + 1) % srv->map_width;
    if (orientation == OR_SOUTH)
        player->y = (player->y + 1) % srv->map_height;
    if (orientation == OR_WEST)
        player->x = (player->x + srv->map_width - 1) % srv->map_width;
}

bool player_move_forward_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = srv->cstates.buff + event->client_id;

    if (event->arg_count != 1)
        return append_to_output(srv, cs, "ko\n"), true;
    player_move(srv, cs, cs->orientation);
    append_to_output(srv, cs, "ok\n");
    gui_player_get_position_handler(srv, event);
    return true;
}

bool player_turn_left_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = srv->cstates.buff + event->client_id;

    if (event->arg_count != 1)
        return append_to_output(srv, cs, "ko\n"), true;
    cs->orientation--;
    cs->orientation &= 0x3;
    append_to_output(srv, cs, "ok\n");
    gui_player_get_position_handler(srv, event);
    return true;
}

bool player_turn_right_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = srv->cstates.buff + event->client_id;

    if (event->arg_count != 1)
        return append_to_output(srv, cs, "ko\n"), true;
    cs->orientation++;
    cs->orientation &= 0x3;
    append_to_output(srv, cs, "ok\n");
    gui_player_get_position_handler(srv, event);
    return true;
}

static
int relative_eject_direction(orientation_t p1, orientation_t p2)
{
    int diff;

    if (p1 == p2)
        return 5;
    diff = p2 - p1;
    if (diff < 1)
        diff = -diff;
    if (diff == 2)
        return 1;
    if ((p1 - p2 == 1) || ((p2 == 4 && (p1 == 1))))
        return 3;
    if ((p2 - p1 == 1) || ((p2 == 1 && (p1 == 4))))
        return 7;
    return 0;
}

static
void destroy_ejected_eggs(server_t *srv, client_state_t *cs)
{
    for (size_t i = 0; i < srv->eggs.nmemb; i++) {
        if (srv->eggs.buff[i].x == cs->x && srv->eggs.buff[i].y == cs->y) {
            send_to_guis(srv, "edi %hu\n", srv->eggs.buff[i].id);
            srv->eggs.buff[i] = srv->eggs.buff[srv->eggs.nmemb - 1];
            srv->eggs.nmemb--;
        }
    }
}

bool player_eject_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = srv->cstates.buff + event->client_id;
    client_state_t *pl;

    if (event->arg_count != 1)
        return append_to_output(srv, cs, "ko\n"), true;
    append_to_output(srv, cs, "ok\n");
    for (size_t i = 0; i < srv->cstates.nmemb; i++) {
        pl = srv->cstates.buff + i;
        if (LIKELY(pl == cs || pl->x != cs->x || pl->y != cs->y
            || pl->team_id == TEAM_ID_GRAPHIC
            || pl->team_id == TEAM_ID_UNASSIGNED))
            continue;
        player_move(srv, pl, cs->orientation);
        vappend_to_output(srv, pl, "eject: %hhu\n",
            relative_eject_direction(pl->orientation, cs->orientation));
        send_to_guis(srv, "pex %hu\nppo %hu %hhu %hhu %hhu\n",
            pl->id, pl->id, pl->x, pl->y, pl->orientation);
    }
    destroy_ejected_eggs(srv, cs);
    return true;
}
