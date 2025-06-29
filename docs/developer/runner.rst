================
Zappy Runner
================

The **Zappy Runner** is a command-line orchestrator that builds and launches the full Zappy server/AI/GUI suite for development or testing. It uses `nix` (or `nom`) to build binaries (from local or remote sources) and spawns the server, multiple AIs, and the GUI with consistent configuration.

It is designed to simplify:

- Building derivations with custom settings (branches, local sources, debug modes)
- Managing multiple team AI clients
- Logging output for debugging
- Coordinating the launch of all components

This document explains how to use the runner, its options, and how it works internally.

----------------
Running the tool
----------------

You can invoke the runner via Nix:

.. code-block:: bash

   nix run github:Sigmapitech/zappy/zappy-runner -- --help

Or to launch a full local game with defaults:

.. code-block:: bash

   nix run github:Sigmapitech/zappy/zappy-runner

----------------
Basic Usage
----------------

By default, this will:

- Build the Zappy server, AI, and GUI derivations
- Launch the server on port 4242 with a 10x10 map
- Start 2 AI clients per team (3 teams)
- Start the GUI
- Connect all clients automatically

You can customize almost every parameter with CLI flags.

----------------
Example invocations
----------------

Use local server and GUI sources (without pulling from GitHub):

.. code-block:: bash

   nix run github:Sigmapitech/zappy/zappy-runner -- --local-server --local-gui

Pin server and GUI to specific branches:

.. code-block:: bash

   nix run github:Sigmapitech/zappy/zappy-runner -- --branch-server fix-map-bug --branch-gui new-theme

Run with debug server build:

.. code-block:: bash

   nix run github:Sigmapitech/zappy/zappy-runner -- --debug-server

----------------
Full CLI reference
----------------

These are all available command-line options:

.. list-table::
   :header-rows: 1

   * - Option
     - Description
     - Default
   * - ``--map-width``
     - Map width in tiles
     - 10
   * - ``--map-height``
     - Map height in tiles
     - 10
   * - ``--team-count``
     - Number of teams
     - 3
   * - ``--team-init-count``
     - Number of AI clients per team
     - 2
   * - ``--team-init-cap``
     - Initial capacity of players per team on the server
     - 200
   * - ``--freq``
     - Server frequency (game ticks per second)
     - 100
   * - ``--port``
     - Server port
     - 4242
   * - ``--host``
     - Server host address for AI and GUI connections
     - 0.0.0.0
   * - ``--ref-gui``
     - Force using reference GUI derivation
     - -
   * - ``--ref-server``
     - Force using reference server derivation
     - -
   * - ``--nom``
     - Use ``nom`` instead of ``nix`` for building
     - -
   * - ``--respective-branches``
     - Build GUI, server, and AI from independent branches
     - -
   * - ``--branch-gui``
     - Override GUI branch
     - None
   * - ``--branch-server``
     - Override server branch
     - None
   * - ``--branch-ai``
     - Override AI branch
     - None
   * - ``--local-ai``
     - Use local source for AI
     - -
   * - ``--local-gui``
     - Use local source for GUI
     - -
   * - ``--local-server``
     - Use local source for server
     - -
   * - ``--debug-server``
     - Use server build with debug instrumentation
     - -
   * - ``--basic-team-names``
     - Use simple team names like team1, team2, team3 instead of generated names
     - -
   * - ``--no-server``
     - Don't launch the server process
     - -
   * - ``--no-gui``
     - Don't launch the GUI process
     - -
   * - ``--no-ai``
     - Don't launch any AI clients
     - -
   * - ``--split-logs``
     - Write separate log files in ``logs/`` directory
     - -
   * - ``--pause-before-connections``
     - Wait for Enter before starting AI and GUI
     - -

----------------
Log files
----------------

If you use ``--split-logs``, the runner will create a ``logs/`` directory and save separate log files for:

- Server output
- Each AI client
- GUI

It also writes a ``.gitignore`` to exclude logs from version control.

----------------
Team names
----------------

By default, the runner generates unique, random, pronounceable team names (like *Zavria* or *Plouvenek*). Use ``--basic-team-names`` to get deterministic names: ``team1``, ``team2``, etc.

----------------
Branches and Sources
----------------

You can control precisely which code is built:

- By default, all components build from the ``zappy-runner`` flake input (same branch).
- ``--respective-branches`` lets you specify different branches for server, GUI, AI.
- You can also force use of local sources (no branch) or official reference derivations.

----------------
How it works
----------------

When you run the runner:

1. It parses CLI options and computes build settings (bitmask flags).
2. It resolves source derivations (with branches, local overrides, or reference builds).
3. It runs ``nix build`` or ``nom build`` for the required derivations.
4. It extracts the binaries from the Nix build outputs.
5. It launches:
   - The server process (if not disabled)
   - Multiple AI processes (per team and init count)
   - The GUI process (if not disabled)
6. It manages their lifecycles and shuts them down cleanly on interrupt.

----------------
Nix Build System
----------------

All building is done via Nix (or Nom if specified). The runner supports:

- Selecting branches (for testing specific changes)
- Local development builds (no GitHub fetch)
- Reference derivations (stable baseline)

This ensures reproducible builds and simplifies testing multiple configurations.

----------------
Interrupt Handling
----------------

If you interrupt the runner (Ctrl+C), it gracefully terminates all subprocesses to avoid zombie processes.
