#include "client.h"
#include "event/handler.h"
#include "event/names.h"

bool gui_map_size_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = srv->cstates.buff + event->client_id;

    vappend_to_output(srv, cs, GUI_MAP_SIZE " %hhu %hhu\n",
        srv->map_width, srv->map_height);
    return true;
}

bool gui_map_content_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = srv->cstates.buff + event->client_id;

    for (size_t y = 0; y < srv->map_height; y++)
        for (size_t x = 0; x < srv->map_width; x++)
            vappend_to_output(srv, cs, "bct %zu %zu %s\n",
                x, y, serialize_inventory(&srv->map[y][x]));
    return true;
}

bool gui_team_names_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = srv->cstates.buff + event->client_id;

    for (size_t i = 0; srv->team_names[i] != nullptr; i++)
        vappend_to_output(srv, cs, GUI_TEAM_NAMES " %s\n", srv->team_names[i]);
    return true;
}
