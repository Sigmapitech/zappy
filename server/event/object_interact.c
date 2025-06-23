//  take and set object -> Pin and bct to all GUIS

#include <stdint.h>
#include <string.h>

#include "handler.h"
#include "client.h"

static constexpr const uint8_t INVALID_OBJECT_ID = 255;

static
uint8_t get_ressource_id(char *command)
{
    static const char *ressources[] = {
        "food", "linemate", "deraumere", "sibur", "mendiane",
        "phiras", "thystame"
    };

    if (!command)
        return INVALID_OBJECT_ID;
    for (size_t i = 0; i < sizeof(ressources) / sizeof(ressources[0]); i++) {
        if (!strcmp(command, ressources[i]))
            return i;
    }
    return INVALID_OBJECT_ID;
}


bool player_take_object_handler(server_t *srv, const event_t *event)
{
    uint8_t object_id = get_ressource_id(event->command[1]);
    client_state_t *client = srv->cstates.buff + event->client_id;
    inventory_t *tile = &srv->map[client->y][client->x];

    if (event->arg_count != 2 || object_id == INVALID_OBJECT_ID) {
        append_to_output(srv, client, "ko\n");
        return true;
    }
    tile->qnts[object_id]--;
    client->inv.qnts[object_id]++;
    append_to_output(srv, client, "ok\n");
    send_to_guis(srv, "pin #%hu %hhu %hhu %s\nbct %hhu %hhu %s\n",
        client->id, client->x, client->y, serialize_inventory(&client->inv),
        client->x, client->y, serialize_inventory(tile));
    return true;
}

bool player_set_object_handler(server_t *srv, const event_t *event)
{
    uint8_t object_id = get_ressource_id(event->command[1]);
    client_state_t *client = srv->cstates.buff + event->client_id;
    inventory_t *tile = &srv->map[client->y][client->x];

    if (event->arg_count != 2 || object_id == INVALID_OBJECT_ID) {
        append_to_output(srv, client, "ko\n");
        return true;
    }
    if (client->inv.qnts[object_id] == 0) {
        append_to_output(srv, client, "ko\n");
        return true;
    }
    tile->qnts[object_id]++;
    client->inv.qnts[object_id]--;
    srv->total_item_in_map.qnts[object_id]++;
    append_to_output(srv, client, "ok\n");
    send_to_guis(srv, "pin #%hu %hhu %hhu %s\nbct %hhu %hhu %s\n",
        client->id, client->x, client->y, serialize_inventory(&client->inv),
        client->x, client->y, serialize_inventory(tile));
    return true;
}
