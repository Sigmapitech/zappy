Architecture Overview
=====================

The Zappy project consists of three main components:

- **Server** (`zappy_server`) - written in C, handles game logic and networking.
- **AI Client** (`zappy_ai`) - written in Python, acts as a bot.
- **GUI Client** (`zappy_gui`) - written in C++, visualizes the world.

Each of these components interacts via TCP/IP using a defined protocol.
