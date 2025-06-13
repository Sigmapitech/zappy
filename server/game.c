#include <string.h>

#include "server.h"
#include "event/switch.h"

static
bool (*find_handler(const char *command))(server_t *, const event_t *)
{
    for (size_t i = 0; i < COMMAND_HANDLERS_COUNT; i++)
        if (!strcmp(COMMAND_HANDLERS[i].name, command))
            return COMMAND_HANDLERS[i].handler;
    return nullptr;
}

void server_process_events(server_t *srv)
{
    bool (*handler)(server_t *, const event_t *);

    if (event_heap_peek(&srv->events) == nullptr)
        return;
    for (; event_heap_peek(&srv->events)->timestamp
        >= get_timestamp() && event_heap_peek(&srv->events) != nullptr;) {
        handler = find_handler(event_heap_peek(&srv->events)->command[0]);
        if (handler == nullptr) {
            DEBUG("No handler found for command: %s",
                event_heap_peek(&srv->events)->command[0]);
            event_heap_pop(&srv->events);
            continue;
        }
        if (!handler(srv, event_heap_peek(&srv->events)))
            perror("Event handler failed");
        event_heap_pop(&srv->events);
    }
}
