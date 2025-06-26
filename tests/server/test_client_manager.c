#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/poll.h>

#include "compass.h"
#include "client.h"
#include "client_manager.h"
#include "macro_utils.h"

enum {
    TEAM_SERVER = 0,
    TEAM_UNASSIGNED = 1,
    TEAM_GRAPHIC = 2,
    TEAM_PLAYER = 3
};

char *client_manager_render(client_manager_t *cm)
{
    static char buff[64] = { };

    memset(buff, '\0', sizeof buff);
    for (size_t i = 0; i < cm->count; i++) {
        switch (cm->clients[i].team_id) {
            case TEAM_SERVER:
                buff[i] = 'S';
                break;
            case TEAM_UNASSIGNED:
                buff[i] = 'U';
                break;
            case TEAM_GRAPHIC:
                buff[i] = 'G';
                break;
            default:
                buff[i] = 'P';
                break;
        }
    }
    return buff;
}

struct counts {
    uint8_t unassigned;
    uint8_t guis;
    uint8_t player;
};

client_manager_t *client_manager_from_counts(struct counts counts)
{
    static client_manager_t cm = { };
    static struct pollfd pfds[64];
    static client_state_t clients[64];

    memset(&cm, 0, sizeof cm);
    memset(clients, 0, sizeof clients);
    cm.clients = clients;
    cm.server_pfds = pfds;

    for (size_t i = 0; i < 64; i++)
        cm.server_pfds[i].fd = cm.clients[i].fd = i;

    cm.clients[0].team_id = TEAM_SERVER; // this is used to align indices
    size_t idx = 1;

    for (size_t i = 0; i < counts.unassigned; i++)
        cm.clients[idx++].team_id = TEAM_UNASSIGNED;
    cm.idx_of_gui = idx;
    for (size_t i = 0; i < counts.guis; i++)
        cm.clients[idx++].team_id = TEAM_GRAPHIC;
    cm.idx_of_players = idx;
    for (size_t i = 0; i < counts.player; i++)
        cm.clients[idx++].team_id = 3 + i;

    cm.count = idx;
    cm.capacity = 64;
    return &cm;
}

enum call {
    CALL_ADD,
    CALL_PROMOTE_PLAYER,
    CALL_PROMOTE_GUI,
    CALL_REMOVE,
    CALL_NONE
};

typedef struct {
    struct counts start_counts;
    const char *team_repr;
    const int *fds;
    size_t fd_count;
    enum call func_id;
    size_t idx;
} checker;

#define MK_FD_ARRAY(...) (int []){ __VA_ARGS__ }, VA_COUNT(__VA_ARGS__)

[[gnu::used]] static
const checker TEST_CLIENT_MANAGER_STATES[] = {
    { { 1, 1, 1 }, "SUGP", MK_FD_ARRAY(0, 1, 2, 3), CALL_NONE, 0 },
    { { 0, 0, 4 }, "SPPPP", MK_FD_ARRAY(0, 1, 2, 3, 4), CALL_NONE, 0 },

    { { 0, 0, 0 }, "SU", MK_FD_ARRAY(0, -1), CALL_ADD, 0 },
    { { 0, 1, 0 }, "SUG", MK_FD_ARRAY(0, -1, 1), CALL_ADD, 0 },
    { { 0, 0, 1 }, "SUP", MK_FD_ARRAY(0, -1, 1), CALL_ADD, 0 },
    { { 0, 1, 1 }, "SUGP", MK_FD_ARRAY(0, -1, 1, 2), CALL_ADD, 0 },
    { { 1, 0, 0 }, "SUU", MK_FD_ARRAY(0, 1, -1), CALL_ADD, 0 },
    { { 1, 1, 0 }, "SUUG", MK_FD_ARRAY(0, 1, -1, 2), CALL_ADD, 0 },
    { { 1, 0, 1 }, "SUUP", MK_FD_ARRAY(0, 1, -1, 2), CALL_ADD, 0 },
    { { 1, 1, 1 }, "SUUGP", MK_FD_ARRAY(0, 1, -1, 2, 3), CALL_ADD, 0 },

    { { 1, 0, 0 }, "SU", MK_FD_ARRAY(0, 1), CALL_REMOVE, 0 },
    { { 1, 0, 0 }, "SU", MK_FD_ARRAY(0, 1), CALL_REMOVE, 2 },

    { { 1, 0, 0 }, "S", MK_FD_ARRAY(0), CALL_REMOVE, 1 },
    { { 1, 1, 0 }, "SG", MK_FD_ARRAY(0, 2), CALL_REMOVE, 1 },
    { { 1, 0, 1 }, "SP", MK_FD_ARRAY(0, 2), CALL_REMOVE, 1 },
    { { 1, 1, 1 }, "SGP", MK_FD_ARRAY(0, 2, 3), CALL_REMOVE, 1 },
    { { 2, 1, 1 }, "SUGP", MK_FD_ARRAY(0, 2, 3, 4), CALL_REMOVE, 1 },

    { { 0, 1, 0 }, "S", MK_FD_ARRAY(0), CALL_REMOVE, 1 },
    { { 1, 1, 0 }, "SU", MK_FD_ARRAY(0, 1), CALL_REMOVE, 2 },
    { { 1, 2, 0 }, "SUG", MK_FD_ARRAY(0, 1, 3), CALL_REMOVE, 2 },
    { { 1, 1, 1 }, "SUP", MK_FD_ARRAY(0, 1, 3), CALL_REMOVE, 2 },
    { { 1, 2, 1 }, "SUGP", MK_FD_ARRAY(0, 1, 3, 4), CALL_REMOVE, 2 },

    { { 0, 0, 1 }, "S", MK_FD_ARRAY(0), CALL_REMOVE, 1 },
    { { 0, 1, 1 }, "SG", MK_FD_ARRAY(0, 1), CALL_REMOVE, 2 },
    { { 0, 0, 2 }, "SP", MK_FD_ARRAY(0, 2), CALL_REMOVE, 1 },
    { { 0, 1, 2 }, "SGP", MK_FD_ARRAY(0, 1, 3), CALL_REMOVE, 2 },

    { { 1, 0, 0 }, "SG", MK_FD_ARRAY(0, 1), CALL_PROMOTE_GUI, 1 },
    { { 1, 0, 0 }, "SP", MK_FD_ARRAY(0, 1), CALL_PROMOTE_PLAYER, 1 },
    { { 2, 0, 0 }, "SUG", MK_FD_ARRAY(0, 2, 1), CALL_PROMOTE_GUI, 1 },
    { { 2, 0, 0 }, "SUP", MK_FD_ARRAY(0, 2, 1), CALL_PROMOTE_PLAYER, 1 },
    { { 2, 0, 0 }, "SUG", MK_FD_ARRAY(0, 1, 2), CALL_PROMOTE_GUI, 2 },
    { { 2, 0, 0 }, "SUP", MK_FD_ARRAY(0, 1, 2), CALL_PROMOTE_PLAYER, 2 },

    { { 1, 0, 1 }, "SPP", MK_FD_ARRAY(0, 1, 2), CALL_PROMOTE_PLAYER, 1 },
    { { 1, 1, 1 }, "SGPP", MK_FD_ARRAY(0, 2, 1, 3), CALL_PROMOTE_PLAYER, 1 },
};

Test(client_manager, batch_test)
{
    for (size_t i = 0; i < LENGTH_OF(TEST_CLIENT_MANAGER_STATES); i++) {
        checker c = TEST_CLIENT_MANAGER_STATES[i];
        client_manager_t *cm = client_manager_from_counts(c.start_counts);

        fprintf(stderr, "\033[38;5;103m╤══\033[0m batch test %zu\n", i);
        assert("test coherency", strlen(c.team_repr) == c.fd_count);

        client_state_t *client;
        switch (c.func_id) {
            case CALL_ADD:
                client_manager_add(cm);
                break;
            case CALL_PROMOTE_GUI:
                client = cm->clients + c.idx;
                client->team_id = TEAM_GRAPHIC;
                if (0) {
            case CALL_PROMOTE_PLAYER:
                client = cm->clients + c.idx;
                client->team_id = TEAM_PLAYER;
                }

                client_manager_promote(cm, c.idx);
                break;
            case CALL_REMOVE:
                client_manager_remove(cm, c.idx);
                break;
            default:
                break;
        }

        assert("has correct count", cm->count == strlen(c.team_repr));
        assert(c.team_repr, !strcmp(client_manager_render(cm), c.team_repr));

        for (size_t j = 0; j < cm->count; j++)
            assert("is the correct fd", c.fds[j] == cm->clients[j].fd);
    }
}
