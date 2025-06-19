#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "client.h"
#include "handler.h"
#include "server.h"

static bool death_rescedule(server_t *srv, const event_t *event, size_t food)
{
    static constexpr const size_t FOOD_SURVIVAL = 126;
    double interval_sec = ((double)food * FOOD_SURVIVAL) / srv->frequency;
    size_t new_sec = (size_t)interval_sec;
    size_t new_usec = (size_t)((interval_sec - new_sec) * MICROSEC_IN_SEC);
    event_t new = {
        .client_id = event->client_id,
        .command = { "player_death" },
        .timestamp = add_time(event->timestamp, new_sec, new_usec),
    };

    if (!event_heap_push(&srv->events, &new)) {
        perror("Failed to reschedule death event");
        return false;
    }
    return true;
}

bool player_death_handler(server_t *srv, const event_t *event)
{
    client_state_t *client = &srv->cstates.buff[event->client_id];
    size_t food = client->inv.food;

    if (food > 0) {
        if (!death_rescedule(srv, event, food))
            return false;
        client->inv.food = 0;
    }
    append_to_output(srv, client, "dead\n");
    write_client(srv, event->client_id + 1);
    for (size_t i = 0; i < srv->cstates.nmemb; i++) {
        if (srv->cstates.buff[i].team_id != GRAPHIC_TEAM_ID)
            continue;
        vappend_to_output(srv, &srv->cstates.buff[i], "pdi #%d\n",
            srv->cstates.buff[event->client_id].fd);
    }
    remove_client(srv, event->client_id + 1);
    return true;
}
