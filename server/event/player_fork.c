#include <stdio.h>

#include "data_structure/resizable_array.h"
#include "client.h"
#include "handler.h"
#include "server.h"

static constexpr const uint64_t EGG_HATCH = 600;

bool player_fork_handler(server_t *srv, const event_t *event)
{
    client_state_t *client = srv->cstates.buff + event->client_idx;
    uint64_t interval_sec = (EGG_HATCH * MICROSEC_IN_SEC) / srv->frequency;

    if (!sized_struct_ensure_capacity((resizable_array_t *)&srv->eggs, 1,
        sizeof *srv->eggs.buff)) {
        perror("Failed to ensure capacity for eggs array\n");
        return false;
    }
    srv->eggs.buff[srv->eggs.nmemb] = (egg_t){get_timestamp() + interval_sec,
        .team_id = client->team_id, .x = client->x, .y = client->y};
    srv->eggs.nmemb++;
    send_to_guis(srv, "enw #%zu %hu %hhu %hhu\n",
        srv->eggs.nmemb, event->client_idx, client->x, client->y);
    append_to_output(srv, client, "ok\n");
    return true;
}
