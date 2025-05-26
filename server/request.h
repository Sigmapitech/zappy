#ifndef REQUEST_H_
    #define REQUEST_H_

    #include <stdarg.h>
    #include <stdio.h>
    #include <string.h>
    #include <unistd.h>

typedef enum {
    MAP_SIZE,
    TILE_CONTENT,
    MAP_CONTENT,
    TEAM_NAMES,
    PLAYER_POSITION,
    PLAYER_LEVEL,
    PLAYER_INVENTORY,
    TIME_UNIT,
    TIME_UNIT_MODIFICATION
} resp_code_t;

typedef enum {
    NEW_PLAYER_CONNECTION,
    EXPULSION,
    BROADCAST,
    INCANTATION_START,
    INCANTATION_END,
    EGG_LAY_START,
    EGG_LAY_END,
    RESOURCE_DROP,
    RESOURCE_COLLECT,
    PLAYER_DEATH,
    EGG_DEATH,
    PLAYER_CONNECT_FOR_EGG,
    GAME_END,
    SERVER_MESSAGE,
    UNKOWN_COMMAND,
    COMMAND_PARAMETER
} notif_code_t;

typedef struct {
    resp_code_t key;
    char *value;
} response_t;

typedef struct {
    notif_code_t key;
    char *value;
} notification_t;

static const response_t RESPONSE[] = {
    {MAP_SIZE, "msz %lu %lu"},
    {TILE_CONTENT, "bct %lu %lu %d %d %d %d %d %d"},
    {TEAM_NAMES, "tna %s"}, // * nbr_teams
    {PLAYER_POSITION, "ppo #%i %lu %lu %c"},
    {PLAYER_LEVEL, "plv #%i %d"},
    {PLAYER_INVENTORY, "pin #%i %lu %lu %d %d %d %d %d %d"},
    {TIME_UNIT, "sgt %d"},
    {TIME_UNIT_MODIFICATION, "sst %d"}
};

static const notification_t NOTIFICATION[] = {
    {NEW_PLAYER_CONNECTION, "pnw #%i %lu %lu %c %d %s"},
    {EXPULSION, "pex #%i"},
    {BROADCAST, "pbc #%i %s"},
    {INCANTATION_START, "pic %lu %lu %d %s"}, // %s = #n #n ...
    {INCANTATION_END, "pie %lu %lu %s"},      // %s = "success" or "failure"
    {EGG_LAY_START, "pfk #%i"},
    {EGG_LAY_END, "enw #%i #%i %lu %lu"},
    {RESOURCE_DROP, "pdr #%i %i"},
    {RESOURCE_COLLECT, "pgt #%i %i"},
    {PLAYER_DEATH, "pdi #%i"},
    {EGG_DEATH, "edi #%i"},
    {PLAYER_CONNECT_FOR_EGG, "ebo #%i"},
    {GAME_END, "seg %s"},
    {SERVER_MESSAGE, "smg %s"},
    {UNKOWN_COMMAND, "suc"},
    {COMMAND_PARAMETER, "sbp"}
};

    #define RESPONSE_POOL_SIZE (int)(sizeof(RESPONSE) / sizeof(response_t))
    #define MESSAGE_LENGTH_MAX 4096

static inline void commit_resp(int fd, resp_code_t code, ...)
{
    va_list list;
    char *tmp = NULL;
    static char resp[MESSAGE_LENGTH_MAX];
    char format[MESSAGE_LENGTH_MAX];
    size_t len;

    for (int i = 0; i < RESPONSE_POOL_SIZE; i++)
        if (RESPONSE[i].key == code)
            tmp = RESPONSE[i].value;
    if (tmp == NULL)
        return;
    va_start(list, code);
    snprintf(format, MESSAGE_LENGTH_MAX, "%d %s\r\n", code, tmp);
    len = vsnprintf(resp, MESSAGE_LENGTH_MAX, format, list);
    va_end(list);
    write(fd, resp, len);
}

static inline void commit_notif(int fd, notif_code_t code, ...)
{
    va_list list;
    char *tmp = NULL;
    static char resp[MESSAGE_LENGTH_MAX];
    char format[MESSAGE_LENGTH_MAX];
    size_t len;

    for (int i = 0; i < (sizeof(NOTIFICATION) / sizeof(notification_t)); i++)
        if (NOTIFICATION[i].key == code)
            tmp = NOTIFICATION[i].value;
    if (tmp == NULL)
        return;
    va_start(list, code);
    snprintf(format, MESSAGE_LENGTH_MAX, "%d %s\r\n", code, tmp);
    len = vsnprintf(resp, MESSAGE_LENGTH_MAX, format, list);
    va_end(list);
    write(fd, resp, len);
}

#endif /* !REQUEST_H_ */
