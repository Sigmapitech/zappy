#include <poll.h>
#include <stdarg.h>
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
static constexpr const size_t ITER_MAX = 4;

struct network_data_s {
    server_t *srv;
    client_state_t *client;
};

static
void error_helper(server_t *srv, const char *msg, uint32_t idx)
{
    perror(msg);
    remove_client(srv, idx);
}

static
bool recv_wrapper(server_t *srv, uint32_t idx, char *buffer, ssize_t *res)
{
    client_state_t *client = &srv->cstates.buff[idx - 1];
    ssize_t recv_res = recv(client->fd, buffer, BUFFER_SIZE - 1, 0);

    if (recv_res < 0) {
        error_helper(srv, "recv failed", idx);
        return false;
    }
    if (recv_res == 0) {
        remove_client(srv, idx);
        return false;
    }
    *res = recv_res;
    return true;
}

void read_client(server_t *srv, uint32_t idx)
{
    char buffer[BUFFER_SIZE] = {0};
    ssize_t recv_res = sizeof(buffer) - 1;
    client_state_t *client = &srv->cstates.buff[idx - 1];

    if (client == nullptr)
        return;
    for (size_t i = 0; i < ITER_MAX && recv_res == sizeof(buffer) - 1; i++) {
        if (!recv_wrapper(srv, idx, buffer, &recv_res))
            return;
        if (!sized_struct_ensure_capacity(
            &client->input, recv_res + 1, sizeof *client->input.buff)) {
            error_helper(srv, "Input buffer resize failed", idx);
            return;
        }
        memcpy(client->input.buff + client->input.nmemb, buffer, recv_res);
        client->input.nmemb += recv_res;
        client->input.buff[client->input.nmemb] = '\0';
    }
    DEBUG("Received from client %d: %s", client->fd, buffer);
}

void write_client(server_t *srv, uint32_t idx)
{
    client_state_t *cl = &srv->cstates.buff[idx - 1];
    ssize_t sent;
    size_t line_len;

    if (!cl || cl->output.nmemb <= cl->out_buff_idx)
        return;
    line_len = strcspn(cl->output.buff + cl->out_buff_idx, "\n");
    if ((cl->output.buff + cl->out_buff_idx)[line_len] != '\n')
        return;
    sent = send(cl->fd, cl->output.buff + cl->out_buff_idx, line_len + 1, 0);
    if (sent < 0) {
        error_helper(srv, "send failed", idx - 1);
        return;
    }
    cl->out_buff_idx += sent;
    if (cl->out_buff_idx != cl->output.nmemb)
        return;
    cl->output.nmemb = 0;
    cl->out_buff_idx = 0;
    srv->pfds.buff[idx].events &= ~POLLOUT;
}

void append_to_output(server_t *srv, client_state_t *client, const char *msg)
{
    size_t len = strlen(msg);
    size_t idx = client - srv->cstates.buff;

    if (!sized_struct_ensure_capacity(&client->output, len + 1,
        sizeof *client->output.buff)) {
        error_helper(srv, "Output buffer resize failed", idx + 1);
        return;
    }
    strncpy(client->output.buff + client->output.nmemb, msg, len + 1);
    client->output.nmemb += len;
    if (!strchr(msg, '\n'))
        return;
    srv->pfds.buff[idx + 1].events |= POLLOUT;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
static int compute_formatted_size(const char *fmt, va_list args)
{
    va_list args_copy;
    int len;

    va_copy(args_copy, args);
    len = vsnprintf(nullptr, 0, fmt, args_copy);
    va_end(args_copy);
    return len;
}

static void fill_and_append(
    struct network_data_s *data, size_t size, const char *fmt, va_list args)
{
    char buffer[size + 1];

    vsnprintf(buffer, size + 1, fmt, args);
    buffer[size] = '\0';
    append_to_output(data->srv, data->client, buffer);
}

void vappend_to_output(server_t *srv,
    client_state_t *client, const char *fmt, ...)
{
    va_list args;
    int size;
    struct network_data_s data = {srv, client};

    va_start(args, fmt);
    size = compute_formatted_size(fmt, args);
    if (size < 0) {
        perror("vsnprintf failed to compute size");
        va_end(args);
        return;
    }
    fill_and_append(&data, (size_t)size, fmt, args);
    va_end(args);
}

// TODO: Improve this function by splitting GUI team into its own v-array
void send_to_guis(server_t *srv, const char *fmt, ...)
{
    va_list args;
    int size;
    static char buff[BUFFER_SIZE] = {};

    va_start(args, fmt);
    size = compute_formatted_size(fmt, args);
    vsnprintf(buff, size + 1, fmt, args);
    for (size_t i = 0; i < srv->cstates.nmemb; i++)
        if (srv->cstates.buff[i].team_id == GRAPHIC_TEAM_ID)
            append_to_output(srv, &srv->cstates.buff[i], buff);
}
#pragma clang diagnostic pop
