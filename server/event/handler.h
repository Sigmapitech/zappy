#ifndef HANDLER_H_
    #define HANDLER_H_

    #include "server.h"

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

#endif /* !HANDLER_H_ */
