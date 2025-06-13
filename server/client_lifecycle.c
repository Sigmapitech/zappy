#include <arpa/inet.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client.h"
#include "data_structure/resizable_array.h"
#include "server.h"

static
void add_client_state(server_t *srv, int fd)
{
    client_state_t client_state = {.input = {},
        .inv = {}, .team_id = 0, .x = 0, .y = 0, .tier = 0, .fd = fd,
        .in_buff_idx = 0};

    if (!sized_struct_ensure_capacity((resizable_array_t *)&srv->cstates,
        1, sizeof *srv->cstates.buff)) {
        perror("malloc");
        close(fd);
        return;
    }
    append_to_output(srv, &client_state, "WELCOME\n");
    srv->cstates.buff[srv->cstates.nmemb] = client_state;
    srv->cstates.nmemb++;
}

void add_client(server_t *srv)
{
    struct sockaddr_in addr;
    socklen_t addr_len;
    int new_fd = accept(srv->self_fd, (struct sockaddr *)&addr, &addr_len);

    if (new_fd < 0) {
        perror("accept failed");
        return;
    }
    if (!sized_struct_ensure_capacity((resizable_array_t *)&srv->pfds,
        1, sizeof *srv->pfds.buff)) {
        perror("malloc");
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

void remove_client(server_t *srv, int fd)
{
    for (size_t i = 0; i < srv->pfds.nmemb; i++) {
        if (srv->pfds.buff[i].fd == fd) {
            close(fd);
            free(srv->cstates.buff[i - 1].input.buff);
            srv->cstates.buff[i - 1] =
                srv->cstates.buff[srv->cstates.nmemb - 1];
            srv->cstates.nmemb--;
            srv->pfds.buff[i] = srv->pfds.buff[srv->pfds.nmemb - 1];
            srv->pfds.nmemb--;
            DEBUG("Client disconnected: fd=%d", fd);
            return;
        }
    }
}
