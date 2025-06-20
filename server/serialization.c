#include <stdio.h>

#include "client.h"
#include "macro_utils.h"
#include "server.h"

char *serialize_inventory(inventory_t *inv)
{
    static char buffer[RES_COUNT * (SSTR_LEN(XCAT(INT_MAX)) + 1)];

    snprintf(buffer, sizeof(buffer), "%u %u %u %u %u %u %u",
        inv->food, inv->linemate, inv->deraumere, inv->sibur,
        inv->mendiane, inv->phiras, inv->thystame);
    return buffer;
}
