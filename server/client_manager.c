#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <unistd.h>

#include "client.h"
#include "client_manager.h"

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

client_state_t *client_manager_add(client_manager_t *cm)
{
    return nullptr;
}

client_state_t *client_manager_promote(
    client_manager_t *cm, client_state_t *client)
{
    return nullptr;
}

client_state_t *client_manager_remove(client_manager_t *cm, size_t idx)
{
    return nullptr;
}
