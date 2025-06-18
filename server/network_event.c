#include <poll.h>
#include <stdio.h>

#include "server.h"
#include "client.h"

void process_poll(server_t *srv, uint64_t timeout)
{
    int poll_result = poll(srv->pfds.buff, srv->pfds.nmemb, timeout);

    if (poll_result < 0) {
        if (srv->is_running)
            perror("poll failed");
    }
}

void process_fds(server_t *srv)
{
    if (srv->pfds.buff[0].revents & POLLIN)
        add_client(srv);
    for (size_t i = 1; i < srv->pfds.nmemb; i++) {
        if (srv->pfds.buff[i].revents & POLLHUP)
            remove_client(srv, i);
        if (srv->pfds.buff[i].revents & POLLIN)
            read_client(srv, i);
        if (srv->pfds.buff[i].revents & POLLOUT)
            write_client(srv, i);
    }
}
