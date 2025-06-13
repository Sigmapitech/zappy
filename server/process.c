#include <arpa/inet.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "client.h"
#include "server.h"

void process_poll(server_t *srv, uint64_t timeout)
{
    int poll_result = poll(srv->pfds.buff, srv->pfds.nmemb, timeout);

    if (poll_result < 0) {
        perror("poll failed");
        srv->is_running = false;
    }
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
        if ((srv->pfds.buff[i].revents & POLLOUT)
            && srv->pfds.buff[i].fd != srv->self_fd)
            write_client(srv, srv->pfds.buff[i].fd);
    }
}

void process_clients_buff(server_t *srv)
{
}
