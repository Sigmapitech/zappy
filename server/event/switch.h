#ifndef SWITCH_H_
    #define SWITCH_H_

    #include "../server.h"

bool meteor_handler(server_t *srv, const event_t *event);

typedef struct {
    const char *name;
    bool (*handler)(server_t *, const event_t *);
} command_handler_t;

static const command_handler_t COMMAND_HANDLERS[] = {
    { "meteor", meteor_handler },
    // Add more command handlers here as needed
};

static constexpr const size_t COMMAND_HANDLERS_COUNT = (
    sizeof(COMMAND_HANDLERS) / sizeof(COMMAND_HANDLERS[0])
);

#endif /* !SWITCH_H_ */
