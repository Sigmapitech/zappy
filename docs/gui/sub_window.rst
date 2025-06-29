GUI SUB_WINDOW
==============

This page describes the sub-window, its usage, and how it works.

Goal
--------

- To get a better visual and avoid flooding the screen with data.
- To show a lot of data in the cleanest way.

Usage
--------

There are three pages.

**Menu (the main page)**
- Contains the time the simulation is running (in seconds)
- Contains the number of resources on the whole map
- Displays all events from AI received from the server

**Teams (the team page)**
- Contains every team; each team contains all of their members.
- Each line of a team contains:
    - the id of the member
    - the position of the member
    - the inventory of the member
    - the level of the member

**Setting (the setting page)**
- Allows changing the color of all text from the whole server.

How does it work
--------

This sub-window is made with ImGui, which simplifies the creation of a sub-window with OpenGL.
The sub-window uses a state system. It allows you to use buttons to change the content of the pages. Each page has its own assigned state.
To change the page, you use a button that changes the state of the sub-window and its content.
