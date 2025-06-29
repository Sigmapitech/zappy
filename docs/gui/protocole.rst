GUI Protocol
===========

This page explains the communication protocol between the GUI client and the server, as defined in the project specification.

Overview
--------

The GUI connects to the server via TCP and exchanges text commands. The server responds in plain text. The basic handshake:

::

<-- WELCOME\n
--> GRAPHIC\n
<-- msz X Y
<-- sgt TIME_UNIT
<-- bct X Y FOOD LINEMATE DERAUMERE SIBUR MENDIANE PHIRAS THYSTAME (* X²)
<-- tna team_name\n (* NB_TEAM)
<-- enw #egg_id #player_number X Y

Where:

- `X`, `Y` are the map dimensions.
- `TIME_UNIT` is the time unit for the server.
- `FOOD`, `LINEMATE`, `DERAUMERE`, `SIBUR`, `MENDIANE`, `PHIRAS`, `THYSTAME` are the resources on a tile.
- `team_name` is the name of a team (repeated for each team, `NB_TEAM` times).
- `egg_id` is the egg number, `player_number` is the player number, and `X Y` are the coordinates.

.. list-table:: Server Command Reference
   :header-rows: 1
   :widths: 20 40 40


Command/Response Examples
--------------------------

   * - Command
     - Description
     - Example
   * - msz X Y
     - Map size
     - ``msz 10 10``
   * - bct X Y q0 q1 q2 q3 q4 q5 q6
     - Content of a tile
     - ``bct 2 3 1 0 0 0 0 0 0``
   * - mct
     - Content of the entire map
     - ``mct``
   * - tna N
     - Name of a team
     - ``tna TeamName``
   * - pnw #n X Y O L N
     - New player connection
     - ``pnw #5 4 4 1 2 TeamName``
   * - ppo #n X Y O
     - Player position
     - ``ppo #5 4 4 1``
   * - plv #n L
     - Player level
     - ``plv #5 3``
   * - pin #n X Y q0 q1 q2 q3 q4 q5 q6
     - Player inventory
     - ``pin #5 4 4 1 0 0 0 0 0 0``
   * - pex #n
     - Expulsion
     - ``pex #5``
   * - pbc #n M
     - Broadcast message
     - ``pbc #5 Hello team!``
   * - pic X Y L #n ...
     - Start of an incantation
     - ``pic 2 3 2 #5 #6``
   * - pie X Y R
     - End of an incantation
     - ``pie 2 3 1``
   * - pfk #n
     - Egg laying
     - ``pfk #5``
   * - pdr #n i
     - Resource dropping
     - ``pdr #5 0``
   * - pgt #n i
     - Resource collecting
     - ``pgt #5 0``
   * - pdi #n
     - Player death
     - ``pdi #5``
   * - enw #e #n X Y
     - Egg laid by a player
     - ``enw #10 #5 4 4``
   * - ebo #e
     - Player connection for egg
     - ``ebo #10``
   * - edi #e
     - Death of an egg
     - ``edi #10``
   * - sgt T
     - Time unit request
     - ``sgt 100``
   * - sst T
     - Time unit modification
     - ``sst 50``
   * - seg N
     - End of game
     - ``seg TeamName``
   * - smg M
     - Message from the server
     - ``smg Welcome to Zappy!``
   * - suc
     - Unknown command
     - ``suc``
   * - sbp
     - Command parameter
     - ``sbp``
