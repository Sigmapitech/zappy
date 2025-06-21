#include "client.h"
#include "event/handler.h"

#include "event/names.h"

bool gui_time_get_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = &srv->cstates.buff[event->client_id];

    if (event->arg_count != 1)
        return append_to_output(srv, cs, "sbp\n"), true;
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
        return append_to_output(srv, cs, "sbp\n"), true;
    if (event->arg_count != 2)
        return append_to_output(srv, cs, "sbp\n"), true;
    freq = strtol(arg, &endptr, 10);
    if (endptr == arg || *endptr != '\0'
        || freq < 0 || freq > 10'000)
        return append_to_output(srv, cs, "sbp\n"), true;
    srv->frequency = freq;
    vappend_to_output(srv, cs, GUI_TIME_SET " %hu \n", srv->frequency);
    return true;
}
