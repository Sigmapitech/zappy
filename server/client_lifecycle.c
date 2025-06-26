#include <arpa/inet.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "client.h"
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
    if (!sized_struct_ensure_capacity((resizable_array_t *)&srv->cstates,
        1, sizeof *srv->cstates.buff)) {
        perror("Can't resize client state");
        close(fd);
        return;
    }
    srv->cstates.buff[srv->cstates.nmemb] = client_state;
    append_to_output(srv, &srv->cstates.buff[srv->cstates.nmemb], "WELCOME\n");
    srv->cstates.nmemb++;
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
    if (!sized_struct_ensure_capacity((resizable_array_t *)&srv->pfds,
        1, sizeof *srv->pfds.buff)) {
        perror("Can't resize poll file descriptors");
        close(new_fd);
        return;
    }
    srv->pfds.buff[srv->pfds.nmemb] = (pollfd_t){
        .fd = new_fd, .events = POLLIN, .revents = 0};
    srv->pfds.nmemb++;
    add_client_state(srv, new_fd);
    DEBUG("New client connected: fd=%d, addr=%s:%d",
        new_fd, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
}

void remove_client(server_t *srv, uint32_t idx)
{
    if (srv->cstates.buff[idx - 1].team_id > TEAM_ID_GRAPHIC)
        send_to_guis(srv, "pdi #%hd\n", srv->cstates.buff[idx - 1].id);
    for (size_t i = 0; i < srv->events.nmemb; i++)
        if (srv->events.buff[i].client_idx == (int)(idx - 1))
            srv->events.buff[i].client_idx = -2;
    DEBUG("Client disconnected: fd=%d", srv->cstates.buff[idx - 1].fd);
    if (srv->cstates.buff[idx - 1].fd >= 0)
        close(srv->cstates.buff[idx - 1].fd);
    srv->cstates.buff[idx - 1].fd = -1;
    free(srv->cstates.buff[idx - 1].input.buff);
    srv->cstates.buff[idx - 1].input.buff = nullptr;
    free(srv->cstates.buff[idx - 1].output.buff);
    srv->cstates.buff[idx - 1].output.buff = nullptr;
    srv->cstates.buff[idx - 1] =
        srv->cstates.buff[srv->cstates.nmemb - 1];
    srv->cstates.nmemb--;
    srv->pfds.buff[idx] = srv->pfds.buff[srv->pfds.nmemb - 1];
    srv->pfds.nmemb--;
}
