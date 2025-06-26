#include "client.h"
#include "event/handler.h"

#include "event/names.h"

static
client_state_t *gui_handler_get_player(server_t *srv, const event_t *event)
{
    char *arg = event->command[1];
    char *endptr;
    long id;

    if (arg == nullptr)
        return nullptr;
    if (*arg != '#')
        return nullptr;
    arg++;
    id = strtol(arg, &endptr, 10);
    if (endptr == arg || *endptr != '\0' || id < 0)
        return nullptr;
    return client_from_id(srv, id);
}

bool gui_player_get_position_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = event_get_client(srv, event);
    client_state_t *player;

    if (cs == nullptr)
        return false;
    if (cs->team_id != TEAM_ID_GRAPHIC)
        return send_to_guis(srv, GUI_PLAYER_POS " #%hu %hhd %hhd %hhu\n",
            cs->id, cs->x, cs->y, cs->orientation + 1), true;
    if (event->arg_count != 2)
        return append_to_output(srv, cs, "sbp\n"), true;
    player = gui_handler_get_player(srv, event);
    if (player == nullptr)
        return append_to_output(srv, cs, "sbp\n"), true;
    vappend_to_output(srv, cs, GUI_PLAYER_POS " #%hu %hhd %hhd %hhu\n",
        player->id, player->x, player->y, player->orientation + 1);
    return true;
}

bool gui_player_get_level_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = event_get_client(srv, event);
    client_state_t *player;

    if (cs == nullptr)
        return false;
    if (cs->team_id != TEAM_ID_GRAPHIC) {
        send_to_guis(srv, GUI_PLAYER_LVL " #%hu %hhu\n", cs->id, cs->tier);
        return true;
    }
    if (event->arg_count != 2)
        return append_to_output(srv, cs, "sbp\n"), true;
    player = gui_handler_get_player(srv, event);
    if (player == nullptr)
        return append_to_output(srv, cs, "sbp\n"), true;
    vappend_to_output(srv, cs,
        GUI_PLAYER_LVL " #%hu %hhu\n", player->id, player->tier);
    return true;
}

bool gui_player_get_inventory_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = event_get_client(srv, event);
    client_state_t *player;

    if (cs == nullptr)
        return false;
    if (cs->team_id != TEAM_ID_GRAPHIC) {
        send_to_guis(srv, GUI_PLAYER_INV " #%hd %hhu %hhu %s\n",
            srv->cm.clients[event->client_idx].id,
            cs->x, cs->y, serialize_inventory(&cs->inv));
        return true;
    }
    if (event->arg_count != 2)
        return append_to_output(srv, cs, "sbp\n"), true;
    player = gui_handler_get_player(srv, event);
    if (player == nullptr)
        return append_to_output(srv, cs, "sbp\n"), true;
    vappend_to_output(srv, cs, GUI_PLAYER_INV " #%hd %hhu %hhu %s\n",
        player->id, cs->x, cs->y, serialize_inventory(&cs->inv));
    return true;
}
