#include <stdio.h>
#include <string.h>

#include "server.h"
#include "client.h"
#include "data_structure/event.h"
#include "event/handler.h"

struct command_handler_s {
    const char *name;
    bool (*handler)(server_t *, const event_t *);
};

static const struct command_handler_s COMMAND_HANDLERS[] = {
    { "meteor", meteor_handler },
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
client_state_t *find_client_by_fd(server_t *srv, int fd)
{
    for (size_t i = 0; i < srv->cstates.nmemb; i++) {
        if (srv->cstates.buff[i].fd == fd)
            return &srv->cstates.buff[i];
    }
    DEBUG("Client with fd %d not found", fd);
    return nullptr;
}

static
void default_handler(server_t *srv, const event_t *event)
{
    client_state_t *client;

    if (event->trigger_fd == srv->self_fd)
        return;
    client = find_client_by_fd(srv, event->trigger_fd);
    append_to_output(srv, client, "ko\n");
}

void server_process_events(server_t *srv)
{
    bool (*handler)(server_t *, const event_t *);

    if (event_heap_peek(&srv->events) == nullptr)
        return;
    for (; event_heap_peek(&srv->events) != nullptr
        && event_heap_peek(&srv->events)->timestamp < get_timestamp();) {
        handler = find_handler(event_heap_peek(&srv->events)->command[0]);
        if (handler == nullptr) {
            DEBUG("No handler found for command: %s",
                event_heap_peek(&srv->events)->command[0]);
            default_handler(srv, event_heap_peek(&srv->events));
            event_heap_pop(&srv->events);
            continue;
        }
        if (!handler(srv, event_heap_peek(&srv->events)))
            DEBUG("Event handler failed for command [%s] from client %d",
                event_heap_peek(&srv->events)->command[0],
                event_heap_peek(&srv->events)->trigger_fd);
        event_heap_pop(&srv->events);
    }
}
