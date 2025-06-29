#include <stdio.h>
#include <string.h>

#include "client/client.h"
#include "game_events/handler.h"
#include "game_events/names.h"

#include "event.h"
#include "server.h"

struct command_handler_s {
    const char *name;
    bool (*handler)(server_t *, const event_t *);
};

static const struct command_handler_s COMMAND_HANDLERS[] = {
    { METEOR, game_meteor_handler },
    { PLAYER_DEATH, player_death_handler },

    { PLAYER_INVENTORY, player_inventory_handler },
    { PLAYER_BROADCAST, player_broadcast_handler },
    { PLAYER_LOOK, player_look_handler },
    { PLAYER_FORWARD, player_move_forward_handler },
    { PLAYER_LEFT, player_turn_left_handler },
    { PLAYER_RIGHT, player_turn_right_handler },
    { PLAYER_EJECT, player_eject_handler },
    { PLAYER_TAKE_OBJECT, player_take_object_handler },
    { PLAYER_SET_OBJECT, player_set_object_handler },
    { PLAYER_FORK, player_fork_handler },
    { PLAYER_START_INCANTATION, player_start_incentation_handler },
    { PLAYER_END_INCANTATION, player_end_incentation_handler },
    { PLAYER_LOCK, player_lock_handler },
    { TEAM_AVAILABLE_SLOTS, team_available_slot_handler },

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
    client_state_t *client = event_get_client(srv, event);

    DEBUG("No handler found for command: %s", event->command[0]);
    if (event->client_idx == 0 || client == nullptr || client->fd < 0)
        return;
    if (client->team_id == TEAM_ID_GRAPHIC)
        append_to_output(srv, client, "suc\n");
    else
        append_to_output(srv, client, "ko\n");
}

void server_handle_events(server_t *srv)
{
    bool (*handler)(server_t *, const event_t *);

    for (const event_t *e = event_heap_peek(&srv->events)
        ; e != nullptr && e->timestamp <= get_timestamp()
        ; event_heap_pop(&srv->events)) {
        e = event_heap_peek(&srv->events);
        DEBUG("event [%s] for client %d", e->command[0], e->client_id);
        if (e->client_idx == CLIENT_DEAD)
            continue;
        handler = find_handler(e->command[0]);
        if (handler == nullptr) {
            default_handler(srv, e);
            continue;
        }
        if (!handler(srv, e)) {
            DEBUG("Event handler failed for command [%s] from client %d",
                e->command[0], e->client_id);
        }
        e = event_heap_peek(&srv->events);
    }
}
