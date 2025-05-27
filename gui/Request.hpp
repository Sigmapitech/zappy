#pragma once

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
} cmd_t;

typedef struct {
  cmd_t key;
  char *value;
} request_t;

static const request_t REQUEST[] = {
  {MAP_SIZE, "msz"},
  {TILE_CONTENT, "bct %lu %lu"},
  {MAP_CONTENT, "mct"},  // content of the map (all the tiles)
  {TEAM_NAMES, "tna"},
  {PLAYER_POSITION, "ppo #%i"},
  {PLAYER_LEVEL, "plv #%i"},
  {PLAYER_INVENTORY, "pin #%i"},
  {TIME_UNIT, "sgt"},
  {TIME_UNIT_MODIFICATION, "sst %d"}};

#define REQUEST_POOL_SIZE (int)(sizeof(REQUEST) / sizeof(request_t))
#define REQUEST_LENGTH_MAX 4096

static inline void commit(int fd, cmd_t code, ...)
{
  va_list list;
  char *tmp = NULL;
  static char resp[REQUEST_LENGTH_MAX];
  char format[REQUEST_LENGTH_MAX];
  size_t len;

  for (int i = 0; i < REQUEST_POOL_SIZE; i++)
    if (REQUEST[i].key == code)
      tmp = REQUEST[i].value;
  if (tmp == NULL)
    return;
  va_start(list, code);
  snprintf(format, REQUEST_LENGTH_MAX, "%d %s\r\n", code, tmp);
  len = vsnprintf(resp, REQUEST_LENGTH_MAX, format, list);
  va_end(list);
  write(fd, resp, len);
}
