#ifndef CLIENT_H_
    #define CLIENT_H_

    #include "server.h"

typedef enum {
    OR_NORTH = 0,
    OR_EAST = 1,
    OR_SOUTH = 2,
    OR_WEST = 3,
} orientation_t;

/**
 * @brief Invalid team ID.
 *
 */
static constexpr const uint8_t INVALID_TEAM_ID = 255;
/**
 * @brief Team ID for the graphic client.
 *
 * This is used to identify the graphic client in the server.
 */
static constexpr const uint8_t GRAPHIC_TEAM_ID = 254;

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
client_state_t *client_from_id(server_t *srv, uint16_t id)
{
    for (size_t i = 0; i < srv->cstates.nmemb; i++)
        if (srv->cstates.buff[i].id == id)
            return &srv->cstates.buff[i];
    return nullptr;
}

#endif /* !CLIENT_H_ */
