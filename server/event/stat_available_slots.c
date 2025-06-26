#include "client.h"
#include "event/handler.h"

bool stat_available_slot_handler(server_t *srv, const event_t *event)
{
    size_t count = 0;
    client_state_t *cs = srv->cstates.buff + event->client_idx;

    if (event->arg_count != 1)
        return append_to_output(srv, cs, "ko\n"), true;
    for (size_t i = 0; i < srv->eggs.nmemb; i++)
        count += srv->eggs.buff[i].team_id == cs->team_id
            && srv->eggs.buff[i].hatch <= get_timestamp();
    vappend_to_output(srv, cs, "%zu\n", count);
    return true;
}
