#ifndef HANDLER_H_
    #define HANDLER_H_

    #include "server.h"

    #define CLENGTH_OF(arr) (sizeof (arr) / sizeof *(arr))

bool game_meteor_handler(server_t *srv, const event_t *event);

bool player_death_handler(server_t *srv, const event_t *event);

bool player_inventory_handler(server_t *srv, const event_t *event);
bool player_broadcast_handler(server_t *srv, const event_t *event);
bool player_look_handler(server_t *srv, const event_t *event);

bool player_move_forward_handler(server_t *srv, const event_t *event);
bool player_turn_left_handler(server_t *srv, const event_t *event);
bool player_turn_right_handler(server_t *srv, const event_t *event);
bool player_eject_handler(server_t *srv, const event_t *event);

bool player_take_object_handler(server_t *srv, const event_t *event);
bool player_set_object_handler(server_t *srv, const event_t *event);

bool player_fork_handler(server_t *srv, const event_t *event);
bool player_start_incentation_handler(server_t *srv, const event_t *event);
bool player_end_incentation_handler(server_t *srv, const event_t *event);
bool player_lock_handler(server_t *, const event_t *);

bool team_available_slot_handler(server_t *srv, const event_t *event);

bool gui_player_get_position_handler(server_t *srv, const event_t *event);
bool gui_player_get_level_handler(server_t *srv, const event_t *event);
bool gui_player_get_inventory_handler(server_t *srv, const event_t *event);

bool gui_time_get_handler(server_t *srv, const event_t *event);
bool gui_time_set_handler(server_t *srv, const event_t *event);

bool gui_map_size_handler(server_t *srv, const event_t *event);
bool gui_map_content_handler(server_t *srv, const event_t *event);
bool gui_tile_content_handler(server_t *srv, const event_t *event);
bool gui_team_names_handler(server_t *srv, const event_t *event);

#endif /* !HANDLER_H_ */
