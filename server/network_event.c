#include <poll.h>
#include <stdio.h>

#include "server.h"
#include "client.h"

void process_poll(server_t *srv, uint64_t timeout)
{
    int poll_result = poll(srv->cm.server_pfds, srv->cm.count, timeout);

    if (poll_result < 0) {
        if (srv->is_running)
            perror("poll failed");
    }
}

static
void check_cm_liveness(server_t *srv)
{
    client_state_t *client;

    DEBUG("srv->self_fd = %zu", srv->self_fd);
    for (size_t i = 0; i < srv->cm.count; i++) {
        client = srv->cm.clients + i;
        DEBUG("cm[%zu]: id=%zu, fd=%zu, team=%zu", i,
            client->id, client->fd, client->team_id);
    }
    if (srv->cm.clients[0].fd != srv->self_fd) {
        DEBUG_MSG("SERVER CM BROKE");
        abort();
    }
}

void process_fds(server_t *srv)
{
    if (srv->cm.server_pfds[0].revents & POLLIN)
        add_client(srv);
    DEBUG_CALL(check_cm_liveness, srv);
    for (size_t i = 1; i < srv->cm.count; i++) {
        if (srv->cm.server_pfds[i].revents & POLLIN)
            read_client(srv, i);
        if (srv->cm.server_pfds[i].revents & POLLOUT)
            write_client(srv, i);
    }
}

void process_disconnection(server_t *srv)
{
    for (size_t i = 1; i < srv->cm.count; i++) {
        if (srv->cm.server_pfds[i].revents & POLLHUP
            || srv->cm.server_pfds[i].revents & POLLERR
        ) {
            client_manager_remove(&srv->cm, i);
            i--;
        }
    }
}
