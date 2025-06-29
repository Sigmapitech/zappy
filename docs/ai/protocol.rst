AI Protocol
===========

This page explains the communication protocol between the AI client and the server,
as defined in the project specification.

Overview
--------

The AI connects to the server via TCP and exchanges text commands. The server
responds in plain text. The basic handshake:

::

   <-- WELCOME\n
   --> TEAM team_name\n
   <-- CLIENT_NUM\n
   <-- X Y\n

Where:

- `CLIENT_NUM` is the number of available slots for the team.
- `X`, `Y` are the map dimensions.

Command/Response Examples
--------------------------

.. list-table:: Player Commands
   :header-rows: 1

   * - Command
     - Time (1/f)
     - Response
   * - Forward
     - 7/f
     - ok
   * - Right
     - 7/f
     - ok
   * - Left
     - 7/f
     - ok
   * - Look
     - 7/f
     - [tile1, tile2, ...]
   * - Inventory
     - 1/f
     - [object count, ...]
   * - Broadcast text
     - 7/f
     - ok
   * - Connect_nbr
     - -
     - number of remaining slots
   * - Fork
     - 42/f
     - ok
   * - Eject
     - 7/f
     - ok / ko
   * - Take object
     - 7/f
     - ok / ko
   * - Set object
     - 7/f
     - ok / ko
   * - Incantation
     - 300/f
     - Elevation underway / ko

Vision
------

The "look" command returns a list of visible tiles. Vision grows with level:

- Level 1: 1 unit
- Level 2: 2 units, etc.

Example server response for "look":

::

   [player, linemate,, food]

Sound/Broadcast
----------------

Players can broadcast messages. The server responds:

::

   message K, text

Where K indicates direction relative to the receiver. Directions are determined
by the tile layout around the player, choosing the shortest wraparound path on
the toroidal map.

Reproduction
------------

Players can fork to create an egg:

- `Fork` command reserves a new slot for a teammate.
- When a new client joins, it hatches an egg and spawns.

Time
----

All actions have duration:

::

   duration = action_time / f

where `f` is the frequency set in server arguments.

