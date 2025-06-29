#include "client/client.h"
#include "handler.h"
#include "names.h"

bool gui_map_size_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = event_get_client(srv, event);

    if (cs == nullptr)
        return cs;
    if (event->arg_count != 1)
        return append_to_output(srv, cs, "sbp\n"), true;
    vappend_to_output(srv, cs, GUI_MAP_SIZE " %hhu %hhu\n",
        srv->map_width, srv->map_height);
    return true;
}

bool gui_map_content_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = event_get_client(srv, event);

    if (cs == nullptr)
        return cs;
    if (event->arg_count != 1)
        return append_to_output(srv, cs, "sbp\n"), true;
    for (size_t y = 0; y < srv->map_height; y++)
        for (size_t x = 0; x < srv->map_width; x++)
            vappend_to_output(srv, cs, "bct %zu %zu %s\n",
                x, y, serialize_inventory(&srv->map[y][x]));
    return true;
}

bool gui_tile_content_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = event_get_client(srv, event);
    char *arg1 = event->command[1];
    char *arg2 = event->command[2];
    char *endptr1;
    char *endptr2;
    size_t x;
    size_t y;

    if (cs == nullptr)
        return cs;
    if (event->arg_count != 3 || arg1 == nullptr || arg2 == nullptr)
        return append_to_output(srv, cs, "sbp\n"), true;
    x = strtoul(arg1, &endptr1, 10);
    y = strtoul(arg2, &endptr2, 10);
    if (endptr1 == arg1 || *endptr1 != '\0' || endptr2 == arg2 ||
        *endptr2 != '\0' || x >= srv->map_width || y >= srv->map_height)
        return append_to_output(srv, cs, "sbp\n"), true;
    vappend_to_output(srv, cs, "bct %zu %zu %s\n",
        x, y, serialize_inventory(&srv->map[y][x]));
    return true;
}

bool gui_team_names_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = event_get_client(srv, event);

    if (cs == nullptr)
        return cs;
    if (event->arg_count != 1)
        return append_to_output(srv, cs, "sbp\n"), true;
    for (size_t i = 0; srv->team_names[i] != nullptr; i++)
        vappend_to_output(srv, cs, GUI_TEAM_NAMES " %s\n", srv->team_names[i]);
    return true;
}
