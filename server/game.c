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
    DEBUG("No handler found for command: %s", event->command[0]);
    if (event->client_id == EVENT_SERVER_ID)
        return;
    if (srv->cstates.buff[event->client_id].fd < 0)
        return;
    append_to_output(srv, &srv->cstates.buff[event->client_id], "ko\n");
}

void server_process_events(server_t *srv)
{
    bool (*handler)(server_t *, const event_t *);
    const event_t *event = event_heap_peek(&srv->events);

    if (event == nullptr)
        return;
    for (; event != nullptr && event->timestamp < get_timestamp();
            event_heap_pop(&srv->events)) {
        if (event->client_id != EVENT_SERVER_ID
            && srv->cstates.buff[event->client_id].fd < 0)
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
