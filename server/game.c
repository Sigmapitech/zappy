#include <stdio.h>
#include <string.h>

#include "server.h"
#include "client.h"
#include "data_structure/event.h"
#include "event/handler.h"
#include "event/names.h"

struct command_handler_s {
    const char *name;
    bool (*handler)(server_t *, const event_t *);
};

static const struct command_handler_s COMMAND_HANDLERS[] = {
    { METEOR, meteor_handler },
    { PLAYER_DEATH, player_death_handler },
    { PLAYER_INVENTORY, player_inventory_handler },
    { PLAYER_BROADCAST, player_broadcast_handler },
    { PLAYER_FORWARD, player_move_forward_handler },
    { PLAYER_LEFT, player_turn_left_handler },
    { PLAYER_RIGHT, player_turn_right_handler },
    { STAT_AVAILABLE_SLOTS, stat_available_slot_handler },
    { GUI_PLAYER_INV, gui_player_get_inventory_handler },
    { GUI_PLAYER_LVL, gui_player_get_level_handler },
    { GUI_PLAYER_POS, gui_player_get_position_handler },
    { GUI_TIME_SET, gui_time_set_handler, },
    { GUI_TIME_GET, gui_time_get_handler, },

    { GUI_MAP_SIZE, gui_map_size_handler },
    { GUI_TILE_CONTENT, gui_tile_content_handler },
    { GUI_MAP_CONTENT, gui_map_content_handler },
    { GUI_TEAM_NAMES, gui_team_names_handler },
    // Add more command handlers here as needed
};

static constexpr const size_t COMMAND_HANDLERS_COUNT = (
    sizeof(COMMAND_HANDLERS) / sizeof(COMMAND_HANDLERS[0])
);

static
bool (*find_handler(const char *command))(server_t *, const event_t *)
{
    for (size_t i = 0; i < COMMAND_HANDLERS_COUNT; i++)
        if (!strcmp(COMMAND_HANDLERS[i].name, command))
            return COMMAND_HANDLERS[i].handler;
    return nullptr;
}

static
void default_handler(server_t *srv, const event_t *event)
{
    client_state_t *client;

    DEBUG("No handler found for command: %s", event->command[0]);
    if (event->client_id == EVENT_SERVER_ID)
        return;
    client = &srv->cstates.buff[event->client_id];
    if (client->fd < 0)
        return;
    if (client->team_id == GRAPHIC_TEAM_ID)
        append_to_output(srv, client, "suc\n");
    else
        append_to_output(srv, client, "ko\n");
}

void server_process_events(server_t *srv)
{
    bool (*handler)(server_t *, const event_t *);

    for (const event_t *event = event_heap_peek(&srv->events)
        ; event != nullptr && event->timestamp < get_timestamp()
        ; event_heap_pop(&srv->events)) {
        if (event->client_id == CLIENT_DEAD)
            continue;
        handler = find_handler(event->command[0]);
        if (handler == nullptr) {
            default_handler(srv, event);
            continue;
        }
        if (!handler(srv, event))
            DEBUG("Event handler failed for command [%s] from client %d",
                event->command[0], event->client_id);
    }
}
