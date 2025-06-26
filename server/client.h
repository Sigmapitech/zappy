#ifndef CLIENT_H_
    #define CLIENT_H_

    #include "server.h"
    #include "data_structure/resizable_array.h"

/**
 * @brief Structure representing a client state in the server.
 *
 */
typedef struct client_state_s {
    resizable_array_t input;
    resizable_array_t output;
    inventory_t inv;
    uint8_t team_id;
    uint8_t x;
    uint8_t y;
    uint8_t tier;
    uint8_t orientation;
    uint32_t id;
    int fd;
    size_t in_buff_idx;
    size_t out_buff_idx;
} client_state_t;

typedef enum {
    OR_NORTH = 0,
    OR_EAST = 1,
    OR_SOUTH = 2,
    OR_WEST = 3,
} orientation_t;

enum {
    TEAM_ID_SERVER = 0,
    TEAM_ID_UNASSIGNED = 1,
    TEAM_ID_GRAPHIC = 2
};

/**
 * @brief Adds a new client to the server.
 *
 * @param srv
 */
void add_client(server_t *srv);
/**
 * @brief Removes a client from the server.
 *
 * @param srv
 * @param idx Index of the client to remove.
 */
void remove_client(server_t *srv, uint32_t idx);
/**
 * @brief Handles a client command.
 *
 * @param srv
 * @param idx Index of the client.
 * @param command Command to handle.
 */
void write_client(server_t *srv, uint32_t idx);
/**
 * @brief Reads a client command.
 *
 * @param srv
 * @param idx Index of the client.
 */
void read_client(server_t *srv, uint32_t idx);

/**
 * @brief Appends a message to the client's output buffer.
 *
 * @param srv
 * @param client
 * @param msg
 */
void append_to_output(server_t *srv, client_state_t *client, const char *msg);
/**
 * @brief Appends a formatted message to the client's output buffer.
 *
 * @param srv
 * @param client
 * @param fmt
 * @param ...
 */
[[gnu::format(printf, 3, 4)]]
void vappend_to_output(
    server_t *srv, client_state_t *client, const char *fmt, ...);

[[gnu::format(printf, 2, 3)]]
void send_to_guis(server_t *srv, const char *fmt, ...);

bool handle_team(server_t *srv, client_state_t *client,
    char *split[static COMMAND_WORD_COUNT]);

static inline
client_state_t *client_from_id(server_t *srv, uint32_t id)
{
    for (size_t i = 0; i < srv->cm.count; i++)
        if (srv->cm.clients[i].id == id)
            return &srv->cm.clients[i];
    return nullptr;
}

char *serialize_inventory(inventory_t *inv);

#endif /* !CLIENT_H_ */
