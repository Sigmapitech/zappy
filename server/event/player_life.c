#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "client.h"
#include "event/names.h"
#include "handler.h"
#include "server.h"

static constexpr const size_t FOOD_SURVIVAL = 126;

static bool death_rescedule(server_t *srv, const event_t *event)
{
    uint64_t interval = (FOOD_SURVIVAL * MICROSEC_IN_SEC) / srv->frequency;
    client_state_t *client = &srv->cstates.buff[event->client_idx];
    event_t new = {
        .timestamp = get_timestamp() + interval,
        .client_idx = event->client_idx,
        .client_id = client->id,
        .command = { PLAYER_DEATH }
    };

    client->inv.food--;
    gui_player_get_inventory_handler(srv, event);
    if (!event_heap_push(&srv->events, &new)) {
        perror("Failed to reschedule death event");
        return false;
    }
    return true;
}

bool player_death_handler(server_t *srv, const event_t *event)
{
    client_state_t *client = &srv->cstates.buff[event->client_idx];

    if (client->inv.food > 0)
        return death_rescedule(srv, event);
    append_to_output(srv, client, "dead\n");
    write_client(srv, event->client_idx + 1);
    remove_client(srv, event->client_idx + 1);
    return true;
}
