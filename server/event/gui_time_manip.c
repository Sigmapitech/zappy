#include "client.h"
#include "event/handler.h"

#include "event/names.h"

bool gui_time_get_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = &srv->cstates.buff[event->client_id];

    vappend_to_output(srv, cs, GUI_TIME_GET " %hu\n", srv->frequency);
    return true;
}

bool gui_time_set_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = &srv->cstates.buff[event->client_id];
    char *arg = event->command[1];
    char *endptr;
    long freq;

    if (arg == nullptr)
        return append_to_output(srv, cs, "spb\n"), true;
    freq = strtol(arg, &endptr, 10);
    if (endptr == arg || *endptr != '\0'
        || freq < 0 || freq > 10'000)
        return append_to_output(srv, cs, "spb\n"), true;
    srv->frequency = freq;
    vappend_to_output(srv, cs, GUI_TIME_SET " %hu \n", srv->frequency);
    return true;
}
