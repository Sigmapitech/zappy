#include <poll.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "server.h"
#include "client.h"
#include "data_structure/resizable_array.h"

static constexpr const size_t BUFFER_SIZE = 1024;

static
client_state_t *get_client_state(server_t *srv, int fd)
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
        remove_client(srv, fd);
        return false;
    }
    *res = recv_res;
    return true;
}

void read_client(server_t *srv, uint32_t fd)
{
    char buffer[BUFFER_SIZE] = {0};
    ssize_t recv_res = sizeof(buffer) - 1;
    client_state_t *client = get_client_state(srv, fd);

    if (client == nullptr)
        return;
    while (recv_res == sizeof(buffer) - 1) {
        if (!recv_wrapper(srv, fd, buffer, &recv_res))
            return;
        if (!sized_struct_ensure_capacity(
            &client->input, recv_res + 1, sizeof *client->input.buff)) {
            perror("malloc");
            remove_client(srv, fd);
            return;
        }
        memcpy(client->input.buff + client->input.nmemb, buffer, recv_res);
        client->input.nmemb += recv_res + 1;
        client->input.buff[client->input.nmemb - 1] = '\0';
    }
    DEBUG("Received from client %d: %s", fd, buffer);
}

static
void reset_pollout(server_t *srv, int fd)
{
    for (size_t i = 0; i < srv->pfds.nmemb; i++) {
        if (srv->pfds.buff[i].fd == fd) {
            srv->pfds.buff[i].events &= ~POLLOUT;
            return;
        }
    }
}

void write_client(server_t *srv, int fd)
{
    client_state_t *cli = get_client_state(srv, fd);
    size_t remaining = cli ? cli->output.nmemb - cli->out_buff_idx : 0;
    ssize_t sent;

    if (!cli || cli->output.nmemb <= cli->out_buff_idx)
        return;
    sent = send(fd, cli->output.buff + cli->out_buff_idx, remaining, 0);
    if (sent < 0) {
        perror("send failed");
        remove_client(srv, fd);
        return;
    }
    cli->out_buff_idx += sent;
    if (cli->out_buff_idx != cli->output.nmemb)
        return;
    cli->output.nmemb = 0;
    cli->out_buff_idx = 0;
    reset_pollout(srv, fd);
}

void append_to_output(server_t *srv, client_state_t *client, const char *msg)
{
    size_t len = strlen(msg);

    if (!sized_struct_ensure_capacity(&client->output, len + 1,
        sizeof *client->output.buff)) {
        perror("malloc");
        remove_client(srv, client->fd);
        return;
    }
    strncpy(client->output.buff + client->output.nmemb, msg, len);
    client->output.nmemb += len;
    client->output.buff[client->output.nmemb] = '\0';
    if (!strchr(msg, '\n'))
        return;
    for (size_t i = 0; i < srv->pfds.nmemb; i++) {
        if (srv->pfds.buff[i].fd == client->fd) {
            srv->pfds.buff[i].events |= POLLOUT;
            break;
        }
    }
}
