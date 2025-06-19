#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "handler.h"

static constexpr const float DENSITIES[RES_COUNT] = {
    0.5F, 0.3F, 0.15F, 0.1F, 0.1F, 0.08F, 0.05F,
};

static const DEBUG_USED char *RES_NAMES[RES_COUNT] = {
    "food", "linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"
};

static constexpr const double METEOR_PERIODICITY_SEC = 20.0F;

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
    DEBUG_MSG("==================================================");
    DEBUG_MSG("Total items in map:");
    for (size_t n = 0; n < RES_COUNT; n++)
        DEBUG_RAW("%s: %u%s", RES_NAMES[n], srv->total_item_in_map.qnts[n],
            (n < RES_COUNT - 1) ? ", " : "");
    DEBUG_RAW_MSG("\n");
}

static bool meteor_rescedule(server_t *srv, const event_t *event)
{
    double interval_sec = METEOR_PERIODICITY_SEC / srv->frequency;
    size_t new_sec = (size_t)interval_sec;
    size_t new_usec = (size_t)((interval_sec - new_sec) * MICROSEC_IN_SEC);
    event_t new = {
        .client_id = event->client_id,
        .command = { "meteor" },
        .timestamp = add_time(event->timestamp, new_sec, new_usec),
    };

    if (!event_heap_push(&srv->events, &new)) {
        perror("Failed to reschedule meteor event");
        return false;
    }
    return true;
}

bool meteor_handler(server_t *srv, const event_t *event)
{
    size_t qty_needed = 0;
    size_t x = 0;
    size_t y = 0;

    DEBUG("Meteor incoming at %lu.%06lu sec since server start",
        (event->timestamp - srv->start_time) / MICROSEC_IN_SEC,
        (event->timestamp - srv->start_time) % MICROSEC_IN_SEC);
    for (size_t n = 0; n < RES_COUNT; n++) {
        qty_needed = (size_t)(srv->map_height * srv->map_width * DENSITIES[n])
            - srv->total_item_in_map.qnts[n];
        for (size_t i = 0; i < qty_needed; i++) {
            x = rand() % srv->map_width;
            y = rand() % srv->map_height;
            srv->map[y][x].qnts[n]++;
            srv->total_item_in_map.qnts[n]++;
        }
    }
    DEBUG_CALL(log_map, srv);
    return meteor_rescedule(srv, event);
}
