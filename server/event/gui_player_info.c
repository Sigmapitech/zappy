#include <stdio.h>

#include "client.h"
#include "event/handler.h"

#include "event/names.h"
#include "macro_utils.h"

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
    client_state_t *cs = srv->cstates.buff + event->client_id;
    client_state_t *player;

    if (cs->team_id != GRAPHIC_TEAM_ID) {
        send_to_guis(srv, GUI_PLAYER_POS " #%hu %hhd %hhd %hhu\n",
            cs->id, cs->x, cs->y, cs->orientation + 1);
        return true;
    }
    player = gui_handler_get_player(srv, event);
    if (player == nullptr) {
        append_to_output(srv, cs, "spb\n");
        return true;
    }
    vappend_to_output(srv, cs, GUI_PLAYER_POS " #%hu %hhd %hhd %hhu\n",
        player->id, player->x, player->y, player->orientation + 1);
    return true;
}

bool gui_player_get_level_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = srv->cstates.buff + event->client_id;
    client_state_t *player;

    if (cs->team_id != GRAPHIC_TEAM_ID) {
        send_to_guis(srv, GUI_PLAYER_LVL " #%hu %hhu\n", cs->id, cs->tier);
        return true;
    }
    player = gui_handler_get_player(srv, event);
    if (player == nullptr) {
        append_to_output(srv, cs, "spb\n");
        return true;
    }
    vappend_to_output(srv, cs,
        GUI_PLAYER_LVL " #%hu %hhu\n", player->id, player->tier);
    return true;
}

static
char *serialize_inventory(inventory_t *inv)
{
    static char buffer[RES_COUNT * (SSTR_LEN(XCAT(INT_MAX)) + 1)];

    snprintf(buffer, sizeof(buffer), "%u %u %u %u %u %u %u",
        inv->food, inv->linemate, inv->deraumere, inv->sibur,
        inv->mendiane, inv->phiras, inv->thystame);
    return buffer;
}

bool gui_player_get_inventory_handler(server_t *srv, const event_t *event)
{
    client_state_t *cs = &srv->cstates.buff[event->client_id];
    client_state_t *player;

    if (cs->team_id != GRAPHIC_TEAM_ID) {
        send_to_guis(srv, GUI_PLAYER_INV " #%hd %hhu %hhu %s\n",
            srv->cstates.buff[event->client_id].id,
            cs->x, cs->y, serialize_inventory(&cs->inv));
        return true;
    }
    player = gui_handler_get_player(srv, event);
    if (player == nullptr) {
        append_to_output(srv, cs, "spb\n");
        return true;
    }
    vappend_to_output(srv, cs, GUI_PLAYER_INV " #%hd %hhu %hhu %s\n",
        player->id, cs->x, cs->y, serialize_inventory(&cs->inv));
    return true;
}
