#ifndef CLIENT_H_
    #define CLIENT_H_

    #include "server.h"

void add_client(server_t *srv);
void remove_client(server_t *srv, uint32_t idx);
void write_client(server_t *srv, uint32_t idx);
void read_client(server_t *srv, uint32_t idx);

void append_to_output(server_t *srv, client_state_t *client, const char *msg);

#endif /* !CLIENT_H_ */
