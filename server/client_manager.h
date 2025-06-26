#ifndef CLIENT_STATE_SEGMENT_H
    #define CLIENT_STATE_SEGMENT_H

typedef struct client_state_s client_state_t;

    #include <stddef.h>

/** Segment for the client state:
                                           v capacity
`----------`----------`-----`--------`-----`--
| untagged | untagged | gui | player | ... | ...
`---------`----------`-----`--------`-----`---
                     ^      |
                     idx_of_gui
                             ^ idx_of_playerss

1st segment contains "untagged" client (they are connected, and thus their tag
is unclear until the handshake done).

The layout also for simple iteration on each segment, all client while keeping
constraints from the client indexing (required by the server).

It will change the pdfs struct upon updates. **/

typedef struct {
    client_state_t *clients;
    size_t count;
    size_t capacity;
    size_t idx_of_gui;
    size_t idx_of_players;
    struct pollfd *server_pfds;
} client_manager_t;

/** Adds a new client as untagged */
client_state_t *client_manager_add(client_manager_t *cm);

/** Remove a client, and reflect to pdfs */
void client_manager_remove(client_manager_t *cm, size_t idx);

/** MUST set team_id BEFORE promoting:
 *
 * client->team_id = TEAM_GUI;
 * segment_promote(cm, client->idx);
 *
 * -> moved to the gui section
 * (will also change the id of 1 player, for contrainsts)
 **/
client_state_t *client_manager_promote(client_manager_t *cm, size_t idx);


#endif
