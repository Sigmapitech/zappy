#include <stdio.h>

#include "client/client.h"
#include "utils/common_macros.h"
#include "server.h"

char *serialize_inventory(inventory_t *inv)
{
    static char buffer[RES_COUNT * (SSTR_LEN(XCAT(INT_MAX)) + 1)];

    snprintf(buffer, sizeof(buffer), "%u %u %u %u %u %u %u",
        inv->food, inv->linemate, inv->deraumere, inv->sibur,
        inv->mendiane, inv->phiras, inv->thystame);
    return buffer;
}
