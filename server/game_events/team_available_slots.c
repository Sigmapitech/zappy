#include "client/client.h"
#include "handler.h"

bool team_available_slot_handler(server_t *srv, const event_t *event)
{
    size_t count = 0;
    client_state_t *cs = event_get_client(srv, event);

    if (cs == nullptr)
        return false;
    if (event->arg_count != 1)
        return append_to_output(srv, cs, "ko\n"), true;
    for (size_t i = 0; i < srv->eggs.nmemb; i++)
        count += srv->eggs.buff[i].team_id == cs->team_id
            && srv->eggs.buff[i].hatch <= get_timestamp();
    vappend_to_output(srv, cs, "%zu\n", count);
    return true;
}
