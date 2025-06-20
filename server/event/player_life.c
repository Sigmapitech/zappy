#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "client.h"
#include "event/names.h"
#include "handler.h"
#include "server.h"

static constexpr const size_t FOOD_SURVIVAL = 126;

static
char *serialize_inventory(inventory_t *inv)
{
    static constexpr const uint8_t BUFFER_SIZE = 128;
    static char buffer[BUFFER_SIZE];

    snprintf(buffer, sizeof(buffer), "%u %u %u %u %u %u %u",
        inv->food, inv->linemate, inv->deraumere, inv->sibur,
        inv->mendiane, inv->phiras, inv->thystame);
    return buffer;
}

static bool death_rescedule(server_t *srv, const event_t *event)
{
    uint64_t interval = (FOOD_SURVIVAL * MICROSEC_IN_SEC) / srv->frequency;
    client_state_t *client = &srv->cstates.buff[event->client_id];
    event_t new = {get_timestamp() + interval,
        event->client_id, .command = { PLAYER_DEATH }};

    client->inv.food--;
    send_to_guis(srv, "pni #%hd %hhu %hhu %s\n",
        srv->cstates.buff[event->client_id].id,
        client->x, client->y, serialize_inventory(&client->inv));
    if (!event_heap_push(&srv->events, &new)) {
        perror("Failed to reschedule death event");
        return false;
    }
    return true;
}

bool player_death_handler(server_t *srv, const event_t *event)
{
    client_state_t *client = &srv->cstates.buff[event->client_id];

    if (client->inv.food > 0)
        return death_rescedule(srv, event);
    append_to_output(srv, client, "dead\n");
    write_client(srv, event->client_id + 1);
    send_to_guis(srv, "pdi #%hd\n", srv->cstates.buff[event->client_id].id);
    remove_client(srv, event->client_id + 1);
    return true;
}
