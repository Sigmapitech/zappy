#include <arpa/inet.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "client.h"
#include "data_structure/event.h"
#include "data_structure/resizable_array.h"
#include "server.h"

static
void add_client_state(server_t *srv, int fd)
{
    static uint32_t id = 0;
    client_state_t client_state = {.input = {},
        .inv = {}, .team_id = TEAM_ID_UNASSIGNED, .x = 0, .y = 0, .tier = 0,
        .fd = fd, .in_buff_idx = 0, .id = id };

    id++;
    DEBUG("Client ID (fd %u): %u", fd, id);
    if (!sized_struct_ensure_capacity((void *)&srv->cm.clients,
        1, sizeof *srv->cm.clients)) {
        perror("Can't resize client state");
        close(fd);
        return;
    }
    srv->cm.clients[srv->cm.count] = client_state;
    append_to_output(srv, &srv->cm.clients[srv->cm.count], "WELCOME\n");
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
    if (!sized_struct_ensure_capacity((void *)&srv->cm.server_pfds,
        1, sizeof *srv->cm.server_pfds)) {
        perror("Can't resize poll file descriptors");
        close(new_fd);
        return;
    }
    srv->cm.server_pfds[srv->cm.count] = (pollfd_t){
        .fd = new_fd, .events = POLLIN, .revents = 0};
    add_client_state(srv, new_fd);
    srv->cm.count++;
    DEBUG("New client connected: fd=%d, addr=%s:%d",
        new_fd, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
}

void remove_client(server_t *srv, uint32_t idx)
{
    if (srv->cm.clients[idx].team_id > TEAM_ID_GRAPHIC)
        send_to_guis(srv, "pdi #%hd\n", srv->cm.clients[idx].id);
    for (size_t i = 0; i < srv->events.nmemb; i++)
        if (srv->events.buff[i].client_idx == (int)(idx))
            srv->events.buff[i].client_idx = CLIENT_DEAD;
    DEBUG("Client disconnected: fd=%d", srv->cm.clients[idx].fd);
    if (srv->cm.clients[idx].fd >= 0)
        close(srv->cm.clients[idx].fd);
    srv->cm.clients[idx].fd = -1;
    free(srv->cm.clients[idx].input.buff);
    srv->cm.clients[idx].input.buff = nullptr;
    free(srv->cm.clients[idx].output.buff);
    srv->cm.clients[idx].output.buff = nullptr;
    srv->cm.clients[idx] = srv->cm.clients[srv->cm.count - 1];
    srv->cm.server_pfds[idx] = srv->cm.server_pfds[srv->cm.count - 1];
    srv->cm.count--;
}
