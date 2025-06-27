#include <poll.h>
#include <stdio.h>

#include "client.h"
#include "debug.h"
#include "server.h"

void process_poll(server_t *srv, uint64_t timeout)
{
    int poll_result = poll(srv->cm.server_pfds, srv->cm.count, timeout);

    if (poll_result < 0) {
        if (srv->is_running)
            perror("poll failed");
    }
}

void process_fds(server_t *srv)
{
    if (srv->cm.server_pfds[0].revents & POLLIN)
        add_client(srv);
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
            remove_client(srv, i);
            i--;
        }
    }
}
