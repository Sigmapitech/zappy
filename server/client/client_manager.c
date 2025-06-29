#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <unistd.h>

#include "client.h"
#include "client_manager.h"
#include "utils/resizable_array.h"

enum {
    SECTION_SERVER = 0,
    SECTION_UNASSIGNED = 1,
    SECTION_GRAPHIC = 2,
    SECTION_PLAYER = 3
};

static
client_state_t *swap_clients(client_manager_t *cm, size_t i, size_t j)
{
    client_state_t tmp;
    struct pollfd tmpfd;

    if (i == j)
        return &cm->clients[j];
    tmp = cm->clients[i];
    cm->clients[i] = cm->clients[j];
    cm->clients[j] = tmp;
    tmpfd = cm->server_pfds[i];
    cm->server_pfds[i] = cm->server_pfds[j];
    cm->server_pfds[j] = tmpfd;
    return &cm->clients[i];
}

static
bool client_manager_ensure_capacity(client_manager_t *cm, size_t request)
{
    resizable_array_t arr = {
        .buff = (char *)cm->server_pfds,
        .nmemb = cm->count,
        .capacity = cm->capacity,
    };

    if (!sized_struct_ensure_capacity(&arr, request, sizeof *cm->server_pfds)
        || !sized_struct_ensure_capacity(
            (resizable_array_t *)cm, request, sizeof *cm->clients))
        return false;
    cm->server_pfds = (struct pollfd *)(void *)arr.buff;
    return true;
}

bool client_manager_init(client_manager_t *cm)
{
    struct pollfd srv_pollfd = { .events = POLLIN };
    client_state_t srv_client = {
        .team_id = TEAM_ID_SERVER,
        .id = 0,
        0,
    };

    if (!client_manager_ensure_capacity(cm, 1))
        return perror("can't allocate memory for clients"), false;
    cm->count = 1;
    *cm->server_pfds = srv_pollfd;
    *cm->clients = srv_client;
    cm->idx_of_gui++;
    cm->idx_of_players++;
    return true;
}

client_state_t *client_manager_add(client_manager_t *cm)
{
    if (!client_manager_ensure_capacity(cm, 1))
        return perror("can't reallocate memory for clients"), nullptr;
    memset(cm->clients + cm->count, 0, sizeof *cm->clients);
    memset(cm->server_pfds + cm->count, 0, sizeof *cm->server_pfds);
    cm->clients[cm->count].fd = -1;
    cm->server_pfds[cm->count].fd = -1;
    cm->clients[cm->count].team_id = SECTION_UNASSIGNED;
    swap_clients(cm, cm->count, cm->idx_of_gui);
    if (cm->idx_of_gui != cm->idx_of_players)
        swap_clients(cm, cm->count, cm->idx_of_players);
    cm->count++;
    cm->idx_of_players++;
    cm->idx_of_gui++;
    return &cm->clients[cm->idx_of_gui - 1];
}

client_state_t *client_manager_promote(
    client_manager_t *cm, size_t idx)
{
    if (idx >= cm->count)
        return nullptr;
    if (cm->clients[idx].team_id == SECTION_GRAPHIC) {
        swap_clients(cm, idx, cm->idx_of_gui - 1);
        cm->idx_of_gui--;
        return cm->clients + cm->idx_of_gui;
    }
    if (cm->clients[idx].team_id > SECTION_GRAPHIC) {
        swap_clients(cm, idx, cm->idx_of_gui - 1);
        swap_clients(cm, cm->idx_of_gui - 1, cm->idx_of_players - 1);
        cm->idx_of_players--;
        cm->idx_of_gui--;
        return cm->clients + cm->idx_of_players;
    }
    return nullptr;
}

static
void client_manager_remove_gui(client_manager_t *cm, size_t idx)
{
    swap_clients(cm, idx, cm->idx_of_players - 1);
    swap_clients(cm, cm->idx_of_players - 1, cm->count - 1);
    cm->idx_of_players--;
}

static
void client_manager_remove_unassigned(client_manager_t *cm, size_t idx)
{
    swap_clients(cm, idx, cm->idx_of_gui - 1);
    swap_clients(cm, cm->idx_of_gui - 1, cm->count - 1);
    if (cm->clients[cm->idx_of_gui - 1].team_id > SECTION_GRAPHIC)
        swap_clients(cm, cm->idx_of_gui - 1, cm->idx_of_players - 1);
    cm->idx_of_gui--;
    cm->idx_of_players--;
}

void client_manager_remove(client_manager_t *cm, size_t idx)
{
    if (idx >= cm->count)
        return;
    switch (cm->clients[idx].team_id) {
        case SECTION_SERVER:
            break;
        case SECTION_UNASSIGNED:
            client_manager_remove_unassigned(cm, idx);
            cm->count--;
            break;
        case SECTION_GRAPHIC:
            client_manager_remove_gui(cm, idx);
            cm->count--;
            break;
        default:
            swap_clients(cm, idx, cm->count - 1);
            cm->count--;
            break;
    }
}
