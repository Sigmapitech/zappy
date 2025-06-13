#include <arpa/inet.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "data_structure/resizable_array.h"
#include "server.h"
#include "sys/types.h"

static constexpr const size_t BUFFER_SIZE = 1024;

void process_poll(server_t *srv, uint64_t timeout)
{
    int poll_result = poll(srv->pfds.buff, srv->pfds.nmemb, timeout);

    if (poll_result < 0) {
        perror("poll failed");
        srv->is_running = false;
    }
}

static
void add_client_state(server_t *srv, int fd)
{
    client_state_t client_state = {
        .buff = {}, .inv = {}, .team_id = 0,
        .x = 0, .y = 0, .tier = 0, .fd = fd
    };

    if (!sized_struct_ensure_capacity((resizable_array_t *)&srv->cstates,
        1, sizeof *srv->cstates.buff)) {
        perror("malloc");
        close(fd);
        return;
    }
    srv->cstates.buff[srv->cstates.nmemb] = client_state;
    srv->cstates.nmemb++;
}

static
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

static
void remove_client(server_t *srv, int fd)
{
    for (size_t i = 0; i < srv->pfds.nmemb; i++) {
        if (srv->pfds.buff[i].fd == fd) {
            close(fd);
            srv->pfds.buff[i] = srv->pfds.buff[srv->pfds.nmemb - 1];
            srv->pfds.nmemb--;
            DEBUG("Client disconnected: fd=%d", fd);
            return;
        }
    }
}

static
client_state_t *get_client_state(server_t *srv, uint32_t fd)
{
    for (size_t i = 0; i < srv->cstates.nmemb; i++) {
        if (srv->cstates.buff[i].fd == fd) {
            return &srv->cstates.buff[i];
        }
    }
    DEBUG("Client state not found for fd=%d", fd);
    return nullptr;
}

static
bool recv_wrapper(server_t *srv, uint32_t fd, char *buffer, ssize_t *res)
{
    ssize_t recv_res = recv(fd, buffer, BUFFER_SIZE - 1, 0);

    if (recv_res < 0) {
        perror("recv failed");
        remove_client(srv, fd);
        return false;
    }
    if (recv_res == 0) {
        DEBUG("Client disconnected: fd=%d", fd);
        remove_client(srv, fd);
        return false;
    }
    *res = recv_res;
    return true;
}

static
void read_client(server_t *srv, uint32_t fd)
{
    char buffer[1024];
    ssize_t recv_res = sizeof(buffer) - 1;
    client_state_t *client = get_client_state(srv, fd);

    if (client == nullptr)
        return;
    while (recv_res == sizeof(buffer) - 1) {
        if (!recv_wrapper(srv, fd, buffer, &recv_res))
            return;
        if (!sized_struct_ensure_capacity(
            &client->buff, recv_res + 1, sizeof *client->buff.buff)) {
            perror("malloc");
            remove_client(srv, fd);
            return;
        }
        memcpy(client->buff.buff + client->buff.nmemb, buffer, recv_res);
        client->buff.nmemb += recv_res + 1;
        client->buff.buff[client->buff.nmemb - 1] = '\0';
    }
    DEBUG("Received from client %d: %s", fd, buffer);
}

void process_fds(server_t *srv)
{
    for (size_t i = 0; i < srv->pfds.nmemb; i++) {
        if ((srv->pfds.buff[i].revents & POLLIN)
            && srv->pfds.buff[i].fd == srv->self_fd)
            add_client(srv);
        if (srv->pfds.buff[i].revents & POLLHUP)
            remove_client(srv, srv->pfds.buff[i].fd);
        if ((srv->pfds.buff[i].revents & POLLIN)
            && srv->pfds.buff[i].fd != srv->self_fd)
            read_client(srv, srv->pfds.buff[i].fd);
    }
}

void process_clients_buff(server_t *srv)
{
}
