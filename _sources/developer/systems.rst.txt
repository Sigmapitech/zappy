Systems Overview
================

Game Loop
---------

The server uses a single-threaded `poll()` loop to manage sockets and timed events.

Resource Management
-------------------

Resources are respawned every 20 time units based on board density.

Elevation Ritual
----------------

Players must gather resources and allies to level up.

AI Logic
--------

The AI parses commands and responds autonomously using state-driven behavior trees.
