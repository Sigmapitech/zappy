#ifndef CLIENT_H_
    #define CLIENT_H_

    #include "server.h"

static const char *GRAPHIC_COMMAND = "GRAPHIC";

static constexpr const uint8_t INVALID_TEAM_ID = 255;
static constexpr const uint8_t GRAPHIC_TEAM_ID = 254;

void add_client(server_t *srv);
void remove_client(server_t *srv, uint32_t idx);
void write_client(server_t *srv, uint32_t idx);
void read_client(server_t *srv, uint32_t idx);

void append_to_output(server_t *srv, client_state_t *client, const char *msg);
[[gnu::format(printf, 3, 4)]]
void vappend_to_output(
    server_t *srv, client_state_t *client, const char *fmt, ...);

#endif /* !CLIENT_H_ */
