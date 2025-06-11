#include <arpa/inet.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <sys/resource.h>
#include <unistd.h>

#include "args_parser.h"
#include "data_structure/resizable_array.h"
#include "debug.h"
#include "server.h"


static
int socket_open(struct sockaddr_in *srv_sa)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0)
        return perror("zappy_server"), -1;
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
        return perror("malloc"), false;
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
    return true;
}

static DEBUG_USED
void log_map(server_t *srv)
{
    DEBUG_USED size_t y = 0;
    DEBUG_USED size_t x = 0;

    DEBUG_MSG("=======================MAP========================");
    DEBUG_MSG("Map contents:");
    for (size_t i = 0; i < (uint64_t)(srv->map_height * srv->map_width); i++) {
        y = i / srv->map_width;
        x = i % srv->map_width;
        DEBUG_RAW("(%zu, %zu): ", x, y);
        for (size_t n = 0; n < RES_COUNT; n++)
            DEBUG_RAW("%s: %u%s", RES_NAMES[n], srv->map[y][x].qnts[n],
                (n < RES_COUNT - 1) ? ", " : "");
        DEBUG_RAW_MSG("\n");
    }
    DEBUG_MSG("Total items in map:");
    for (size_t n = 0; n < RES_COUNT; n++)
        DEBUG_RAW("%s: %u%s", RES_NAMES[n], srv->total_item_in_map.qnts[n],
            (n < RES_COUNT - 1) ? ", " : "");
    DEBUG_RAW_MSG("\n");
    DEBUG_MSG("==================================================");
}

static
void fill_ressource(server_t *srv, uint8_t height, uint8_t width)
{
    size_t quantity;
    size_t x = 0;
    size_t y = 0;
    static constexpr const float DENSITIES[RES_COUNT] = {
        0.5F, 0.3F, 0.15F, 0.1F, 0.1F, 0.08F, 0.05F,
    };

    for (size_t n = 0; n < RES_COUNT; n++) {
        quantity = (size_t)(height * width * DENSITIES[n]);
        for (size_t i = 0; i < quantity; i++) {
            x = rand() % width;
            y = rand() % height;
            srv->map[y][x].qnts[n]++;
            srv->total_item_in_map.qnts[n]++;
        }
    }
    DEBUG_CALL(log_map, srv);
}

static
bool server_boot(server_t *srv, params_t *p)
{
    static constexpr const int BACKLOG = 32;
    struct sockaddr_in default_sa = {
        .sin_family = AF_INET,
        .sin_port = htons(p->port),
        .sin_addr.s_addr = INADDR_ANY
    };

    srv->self_fd = socket_open(&default_sa);
    if (srv->self_fd < 0 || listen(srv->self_fd, BACKLOG) < 0)
        return false;
    srv->client_count = 0;
    srv->map_height = p->map_height;
    srv->map_width = p->map_width;
    fill_ressource(srv, p->map_height, p->map_width);
    return true;
}

static
bool server_allocate(server_t *srv, params_t *p, uint64_t timestamp)
{

    if (!setup_teams(srv, p, timestamp))
        return false;
    if (!sized_struct_ensure_capacity((resizable_array_t *)&srv->pfds,
        1, sizeof(client_state_t)))
        return perror("malloc"), false;
    srv->pfds.buff[srv->pfds.nmemb] = (pollfd_t){
        .fd = srv->self_fd, .events = POLLIN, .revents = 0};
    srv->pfds.nmemb++;
    return true;
}

static
void server_destroy(server_t *srv)
{
    if (srv->self_fd >= 0)
        close(srv->self_fd);
    free(srv->eggs.buff);
    free(srv->pfds.buff);
    srv->is_running = false;
}

bool server_run(params_t *p, uint64_t timestamp)
{
    server_t srv = {.self_fd = -1, 0};

    if (!server_allocate(&srv, p, timestamp) || !server_boot(&srv, p))
        return server_destroy(&srv), false;
    srv.is_running = true;
    for (; srv.is_running;) {
        break;
        // Placeholder for actual server loop logic
        // Here you would typically handle incoming connections,
        // process commands, and manage the game state.
    }
    server_destroy(&srv);
    return true;
}
