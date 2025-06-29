AI Job System
==============

This page explains the *Legacy AI Behavior* we experimented with for the project.

Our unfinished AI design used well-defined behavioral roles called **Jobs**. Each job had its own file in `ai/behavior/`, implementing the abstract `JobTemplate` class. This class provided a shared structure for all specialized behaviors.

Overview
--------

- **Newcomer**
  - Default job for any freshly spawned player.
  - Waits to be assigned a role by the existing *Elder*.
  - If no Elder broadcast is heard for 50 ticks, promotes itself to Elder.

- **Elder**
  - A single Elder exists per civilization.
  - Acts as the central coordinator:
    - Chooses a "home tile" near civilization center.
    - Receives stones, food, and players for incantations.
    - Assigns jobs to newcomers.
    - Tracks the roles and locations of all known players.
  - Garblers are a partial exception: they are not tracked as precisely, and assignment is rarer to avoid confusion.

- **Basic**
  - Default worker role.
  - Randomly paths around the map, collecting any stones in its way.
  - Broadcasts when it has enough resources to evolve.

- **Collector**
  - Focuses on gathering specific stones.
  - Delivers resources to the Elder's designated home tile.

- **Garbler**
  - Leaves the civilization to embed itself in an enemy team.
  - Goal is to disrupt enemy communication.
  - For every message it sends, it cycles modes:
    - **50%** Raven: emits nonsense (including invalid encodings like `\feff`).
    - **40%** Mimicry: repeats verbatim any intercepted enemy message.
    - **10%** Parrot: repeats messages with heavy modification.

Job Assignment System
----------------------

The Elder assigns jobs dynamically, using our team's broadcast protocol. Each player listens for instructions and responds appropriately.

Communication Protocol
-----------------------

All inter-player instructions passed through team broadcasts during the game are defined below:

+------------+-----------------------------------+-----------------------------------------------+----------------------------------------------------------+-----------------------------------------+
| **Job**    | **Instruction (Recv)**           | **Instruction (Send)**                        | **Effect**                                                | **Args / Type**                         |
+============+===================================+===============================================+==========================================================+=========================================+
| Basic      | `assign_job <id> <job>`          | `evolution_ready <level>`                     | Elder-directed reassignment; immediate job transition    | id: `int`, job: `str`                   |
|            | `evolution_call <count>`         | `evolution_response <id>`                     | Respond to Elder's group evolution call                   | id: `int`                               |
|            | `status_report_request`          | `basic_status level:<L> needs:<N> food:<F>`   | Periodic status update to Elder                           | level: `int`, needs: `int`, food: `int` |
|            | `resource_request <res> <amt>`   | `resource_available <res>`                    | Reply if the player holds excess resources                | resource: `str`, amt: `int`             |
+------------+-----------------------------------+-----------------------------------------------+----------------------------------------------------------+-----------------------------------------+
| Elder      | `newcomer_seeking_assignment`    | `assign_job <id> <job>`                       | Assigns job to a newcomer                                 | id: `int`, job: `str`                   |
|            | `resource_delivery <res> <amt>`  | `status_report_request`                       | Updates depot stock from Collector; asks for reports      | resource: `str`, amt: `int`             |
|            |                                   | `elder_location <x> <y>`                      | Announces Elder's tile location                           | x: `int`, y: `int`                      |
+------------+-----------------------------------+-----------------------------------------------+----------------------------------------------------------+-----------------------------------------+
| Newcomer   | `elder_announce <id>`            | `newcomer_seeking_assignment <level>`         | Announces presence and level; awaits Elder assignment     | id: `int`, level: `int`                 |
|            | `assign_job <id> <job>`          | `newcomer_joined`                             | Confirms receiving role assignment                        | id: `int`, job: `str`                   |
+------------+-----------------------------------+-----------------------------------------------+----------------------------------------------------------+-----------------------------------------+
| Collector  |                                   | `resource_delivery <res> <amt>`               | Delivers collected resources to the Elder                 | resource: `str`, amt: `int`             |
+------------+-----------------------------------+-----------------------------------------------+----------------------------------------------------------+-----------------------------------------+
| Garbler    |                                   | *variable garbage*                            | Disrupts enemies with random unreadable text              | `str`                                   |
|            |                                   | *intercepted_message (mimicry/parrot)*        | Mimic or distort intercepted enemy messages               | `str`                                   |
|            | *any enemy message*               |                                               | Stores for mimicry/parrot modes                           | `str`                                   |
+------------+-----------------------------------+-----------------------------------------------+----------------------------------------------------------+-----------------------------------------+

Behavior Philosophy
--------------------

This system was designed to:

- Decentralize coordination via broadcast-based communication.
- Have a single Elder coordinate overall strategy.
- Allow role reassignment and adaptability during play.
- Disrupt enemy teams by embedding Garblers who mimic or garble enemy messages.

Although unfinished, this "Job System" represents an ambitious attempt to create complex AI behaviors resembling a true in-game society.
