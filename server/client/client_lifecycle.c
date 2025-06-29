#include <arpa/inet.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils/resizable_array.h"

#include "client.h"
#include "event.h"
#include "server.h"

static
bool add_client_state(server_t *srv, int fd)
{
    static uint32_t id = 0;
    client_state_t *client = client_manager_add(&srv->cm);
    size_t idx;

    if (client == nullptr)
        return false;
    client->fd = fd;
    client->id = id;
    idx = srv->cm.idx_of_gui - 1;
    srv->cm.server_pfds[idx].fd = fd;
    srv->cm.server_pfds[idx].events = POLLIN;
    srv->cm.server_pfds[idx].revents = 0;
    id++;
    append_to_output(srv, client, "WELCOME\n");
    return true;
}

void add_client(server_t *srv)
{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    int new_fd = accept(srv->self_fd, (struct sockaddr *)&addr, &addr_len);

    if (new_fd < 0) {
        perror("accept failed");
        return;
    }
    if (!add_client_state(srv, new_fd)) {
        close(new_fd);
        perror("failed to register client");
        return;
    }
    DEBUG("New client connected: fd=%d, addr=%s:%d",
        new_fd, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
}

void remove_client(server_t *srv, uint32_t idx)
{
    if (idx >= srv->cm.count)
        return;
    if (srv->cm.clients[idx].team_id > TEAM_ID_GRAPHIC)
        send_to_guis(srv, "pdi #%hd\n", srv->cm.clients[idx].id);
    for (size_t i = 0; i < srv->events.nmemb; i++)
        if (srv->events.buff[i].client_idx == (int)(idx))
            srv->events.buff[i].client_idx = CLIENT_DEAD;
    DEBUG("Client disconnected: %u, fd=%d", idx, srv->cm.clients[idx].fd);
    if (srv->cm.clients[idx].fd >= 0)
        close(srv->cm.clients[idx].fd);
    free(srv->cm.clients[idx].input.buff);
    free(srv->cm.clients[idx].output.buff);
    client_manager_remove(&srv->cm, idx);
}
