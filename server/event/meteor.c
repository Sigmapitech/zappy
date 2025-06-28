#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "event/names.h"
#include "handler.h"

static constexpr const float DENSITIES[RES_COUNT] = {
    0.5F, 0.3F, 0.15F, 0.1F, 0.1F, 0.08F, 0.05F,
};

DEBUG_USED
static const char *RES_NAMES[RES_COUNT] = {
    "food", "linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"
};

static constexpr const double METEOR_PERIODICITY_SEC = 20.0F;

[[gnu::unused]] static
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
    uint64_t interval_sec =
        (METEOR_PERIODICITY_SEC * MICROSEC_IN_SEC) / srv->frequency;
    event_t new = {
        .client_idx = event->client_idx,
        .command = { METEOR },
        .timestamp = event->timestamp + interval_sec,
    };

    DEBUG("Meteor incoming in %ld ms",
        (new.timestamp - get_timestamp()) / MILISEC_IN_SEC);
    if (!event_heap_push(&srv->events, &new)) {
        perror("Failed to reschedule meteor event");
        return false;
    }
    return true;
}

bool meteor_handler(server_t *srv, const event_t *event)
{
    ssize_t qty_needed = 0;
    size_t x = 0;
    size_t y = 0;

    for (size_t n = 0; n < RES_COUNT; n++) {
        qty_needed = (ssize_t)(srv->map_height * srv->map_width * DENSITIES[n])
            - srv->total_item_in_map.qnts[n];
        DEBUG("meteor: %u %s, missing %ld",
            srv->total_item_in_map.qnts[n], RES_NAMES[n], qty_needed);
        for (ssize_t i = 0; i < qty_needed; i++) {
            x = rand() % srv->map_width;
            y = rand() % srv->map_height;
            srv->map[y][x].qnts[n]++;
            srv->total_item_in_map.qnts[n]++;
        }
    }
    return meteor_rescedule(srv, event);
}
