#define _GNU_SOURCE

#include <arpa/inet.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <sys/resource.h>
#include <unistd.h>

#include "args_parser.h"
#include "client.h"
#include "data_structure/event.h"
#include "data_structure/resizable_array.h"
#include "debug.h"
#include "event/names.h"
#include "server.h"

static
void signal_handler(int signum, siginfo_t *info, void *context)
{
    static server_t *server = nullptr;

    if (info == nullptr && !signum) {
        server = (server_t *)context;
        return;
    }
    if (signum == SIGINT || signum == SIGTERM) {
        DEBUG_MSG("Received signal, shutting down server");
        server->is_running = false;
    }
}

static
int socket_open(struct sockaddr_in *srv_sa)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0)
        return -1;
    if (
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0
        || bind(fd, (struct sockaddr *)srv_sa, sizeof *srv_sa) < 0
    ) {
        close(fd);
        return -1;
    }
    return fd;
}

static
bool setup_teams(server_t *srv, params_t *p, uint64_t timestamp)
{
    size_t t_counter = 0;

    for (; p->teams[t_counter] != nullptr; t_counter++);
    if (!sized_struct_ensure_capacity((resizable_array_t *)&srv->eggs,
        t_counter * p->team_capacity, sizeof(egg_t)))
        return perror("Allocation for start's egg fail"), false;
    for (size_t t_idx = 0; t_idx < t_counter; t_idx++) {
        srv->team_names[t_idx] = p->teams[t_idx];
        for (size_t t_egg_id = 0; t_egg_id < p->team_capacity; t_egg_id++) {
            srv->eggs.buff[(t_idx * p->team_capacity) + t_egg_id] = (egg_t){
                .hatch = timestamp, .team_id = t_idx,
                .x = rand() % p->map_width, .y = rand() % p->map_height};
            DEBUG("Egg %zu for team %s at (%u, %u)",
                t_egg_id, srv->team_names[t_idx],
                srv->eggs.buff[(t_idx * p->team_capacity) + t_egg_id].x,
                srv->eggs.buff[(t_idx * p->team_capacity) + t_egg_id].y);
        }
    }
    srv->eggs.nmemb = t_counter * p->team_capacity;
    return true;
}

static
bool server_boot(server_t *srv, params_t *p)
{
    static constexpr const int BACKLOG = 32;
    struct sockaddr_in default_sa = {
        .sin_family = AF_INET, .sin_port = htons(p->port),
        .sin_addr.s_addr = INADDR_ANY};
    event_t meteor = {.timestamp = srv->start_time, .client_id = 0,
        .command = { METEOR }};

    srv->self_fd = socket_open(&default_sa);
    if (srv->self_fd < 0 || listen(srv->self_fd, BACKLOG) < 0)
        return perror("Can't open server socket"), false;
    srv->map_height = p->map_height;
    srv->map_width = p->map_width;
    srv->start_time = get_timestamp();
    srv->frequency = p->frequency;
    srv->pfds.buff[srv->pfds.nmemb] = (pollfd_t){
        .fd = srv->self_fd, .events = POLLIN, .revents = 0};
    srv->pfds.nmemb++;
    meteor.timestamp = srv->start_time;
    meteor.client_id = EVENT_SERVER_ID;
    return event_heap_push(&srv->events, &meteor);
}

static
bool server_allocate(server_t *srv, params_t *p, uint64_t timestamp)
{
    struct sigaction sa = {
        .sa_flags = SA_SIGINFO,
        .sa_sigaction = signal_handler
    };

    if (sigaction(SIGINT, &sa, nullptr) < 0
        || sigaction(SIGTERM, &sa, nullptr) < 0)
        return perror("Can't set signal handler"), false;
    if (!setup_teams(srv, p, timestamp))
        return false;
    if (!sized_struct_ensure_capacity((resizable_array_t *)&srv->pfds,
        1, sizeof(client_state_t)))
        return perror("Can't allocate pollfd array"), false;
    if (!event_heap_init(&srv->events))
        return perror("Can't initialise event priority queue"), false;
    signal_handler(0, nullptr, srv);
    return true;
}

static
void server_destroy(server_t *srv)
{
    size_t fd_count = srv->pfds.nmemb;

    for (size_t i = 0; i < fd_count; i++) {
        if (srv->pfds.buff[i].fd == srv->self_fd)
            close(srv->self_fd);
        if (srv->pfds.buff[i].fd != srv->self_fd)
            remove_client(srv, i);
    }
    free(srv->eggs.buff);
    free(srv->pfds.buff);
    free(srv->cstates.buff);
    event_heap_free(&srv->events);
    srv->is_running = false;
}

bool server_run(params_t *p, uint64_t timestamp)
{
    server_t srv = {.self_fd = -1, 0};
    int32_t to;

    if (!server_allocate(&srv, p, timestamp) || !server_boot(&srv, p))
        return server_destroy(&srv), false;
    for (srv.is_running = true; srv.is_running;) {
        server_process_events(&srv);
        to = compute_timeout(&srv);
        if (UNLIKELY(to > 0)) {
            process_poll(&srv, to);
            process_fds(&srv);
            process_clients_buff(&srv);
            continue;
        }
        if (UNLIKELY(to < 0))
            fprintf(stderr, "WANRING: Server can't keep up with the events, "
                "timeout is negative (%d ms), skipping tick\n", to);
    }
    server_destroy(&srv);
    return true;
}
