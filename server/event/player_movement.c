#include "client.h"
#include "event/handler.h"

bool player_move_forward_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = srv->cstates.buff + event->client_id;

    if (cs->orientation == OR_NORTH)
        cs->y = (cs->y + srv->map_height - 1) % srv->map_height;
    if (cs->orientation == OR_EAST)
        cs->x = (cs->x + 1) % srv->map_width;
    if (cs->orientation == OR_SOUTH)
        cs->y = (cs->y + 1) % srv->map_height;
    if (cs->orientation == OR_WEST)
        cs->x = (cs->x + srv->map_width - 1) % srv->map_width;
    append_to_output(srv, cs, "ok\n");
    send_to_guis(srv, "ppo #%d %hhd %hhd %hhu\n",
        cs->id, cs->x, cs->y, cs->orientation + 1);
    return true;
}

bool player_turn_left_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = srv->cstates.buff + event->client_id;

    cs->orientation--;
    cs->orientation &= 0x3;
    append_to_output(srv, cs, "ok\n");
    send_to_guis(srv, "ppo #%d %hhd %hhd %hhu\n",
        cs->id, cs->x, cs->y, cs->orientation + 1);
    return true;
}

bool player_turn_right_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = srv->cstates.buff + event->client_id;

    cs->orientation++;
    cs->orientation &= 0x3;
    append_to_output(srv, cs, "ok\n");
    send_to_guis(srv, "ppo #%d %hhd %hhd %hhu\n",
        cs->id, cs->x, cs->y, cs->orientation + 1);
    return true;
}
