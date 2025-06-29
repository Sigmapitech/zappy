#include "client/client.h"
#include "handler.h"

#define __USE_MISC
// ^ above is for M_PI_[...]
#include <math.h>

int get_relative_sound_direction(
    server_t *srv,
    client_state_t *author, client_state_t *receiver)
{
    double rel_angle;
    int dx = (author->x - receiver->x) % srv->map_width;
    int dy = (author->y - receiver->y) % srv->map_height;

    if (dx > (srv->map_width >> 1))
        dx -= srv->map_width;
    if (dy > (srv->map_height >> 1))
        dy -= srv->map_height;
    if (dx == 0 && dy == 0)
        return 0;
    rel_angle = fmod(
        (((double)(receiver->orientation + 1) * M_PI_2)
        - atan2(-dy, dx) + (2 * M_PI)), (2 * M_PI));
    return (int)fmod((rel_angle / M_PI_4), 8);
}

bool player_broadcast_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = event_get_client(srv, event);

    if (cs == nullptr)
        return false;
    if (event->arg_count != 2)
        return append_to_output(srv, cs, "ko\n"), true;
    for (size_t i = srv->cm.idx_of_players; i < srv->cm.count; i++) {
        if (cs->id == srv->cm.clients[i].id)
            continue;
        vappend_to_output(srv, &srv->cm.clients[i],
            "message %d, %s\n",
            get_relative_sound_direction(srv, cs, &srv->cm.clients[i]),
            event->command[1]);
    }
    send_to_guis(srv, "pbc #%hd %s\n",
        srv->cm.clients[event->client_idx].id, event->command[1]);
    append_to_output(srv, cs, "ok\n");
    return true;
}
