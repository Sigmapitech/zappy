#include <stdio.h>

#include "client.h"
#include "event/handler.h"

static
const char *INVENTORY_RESSOURCE_NAMES[RES_COUNT] = {
    "food",
    "linemate",
    "deraumere",
    "sibur",
    "mendiane",
    "phiras",
    "thystame"
};

bool player_inventory_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = event_get_client(srv, event);
    uint32_t *slots = cs->inv.qnts;

    if (cs == nullptr)
        return false;
    if (event->arg_count != 1)
        return append_to_output(srv, cs, "ko\n"), true;
    vappend_to_output(srv, cs, "[");
    for (size_t i = 0; i < RES_COUNT; i++) {
        if (i)
            vappend_to_output(srv, cs, ", ");
        vappend_to_output(srv, cs, "%s %u",
            INVENTORY_RESSOURCE_NAMES[i], slots[i]);
    }
    vappend_to_output(srv, cs, "]\n");
    return true;
}
