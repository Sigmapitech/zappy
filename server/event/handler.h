#ifndef HANDLER_H_
    #define HANDLER_H_

    #include "server.h"

    #define CLENGTH_OF(arr) (sizeof (arr) / sizeof *(arr))

/**
 * @brief Handles the "meteor" event.
 *
 * @param srv
 * @param event
 * @return true
 * @return false
 */
bool meteor_handler(server_t *srv, const event_t *event);
bool player_death_handler(server_t *srv, const event_t *event);
bool player_inventory_handler(server_t *srv, const event_t *event);
bool player_broadcast_handler(server_t *srv, const event_t *event);

bool player_move_forward_handler(server_t *srv, const event_t *event);
bool player_turn_left_handler(server_t *srv, const event_t *event);
bool player_turn_right_handler(server_t *srv, const event_t *event);

bool stat_available_slot_handler(server_t *srv, const event_t *event);

bool gui_player_get_position_handler(server_t *srv, const event_t *event);
bool gui_player_get_level_handler(server_t *srv, const event_t *event);
bool gui_player_get_inventory_handler(server_t *srv, const event_t *event);

bool gui_time_get_handler(server_t *srv, const event_t *event);
bool gui_time_set_handler(server_t *srv, const event_t *event);

#endif /* !HANDLER_H_ */
