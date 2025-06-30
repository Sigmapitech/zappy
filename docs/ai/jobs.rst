Job class system
==============

This page explains the *Legacy AI Behavior* we experimented with for the project.

Our unfinished AI design used well-defined behavioral roles called **Jobs**.
Each job had its own file in `ai/behavior/`, implementing the abstract `JobTemplate` class.
This class provided a shared structure for all specialized behaviors.

Overview
--------

- **Newcomer**
    - Every player that spawns in the map have this job as default; the player can be assigned job by the already existing or become one if no *Elder* broadcast has been for 50 ticks.
- **Elder**
    - There can only be one per civilization, as it is the *quite literal* pillar of this species.
    - Will choose a tile that's in the center of its civilization, to receive stones, food and players players to perform the incantation on this home tile.
    - Dispatches newcomers around different jobs.
    - Knows every player on the map, and their job. *Garblers* are an exception and aren't precisely counted. To account for that, they're not assigned often to players.
- **Basic**
    - Collects the stones that are in its way, randomly pathing around the map, and sends a broadcast when he has enough stones to evolve.
- **Collector**
    - Collects stones and drops them off on the tile of the *Elder*. As simple as that.
- **Garbler**
    - Goes away from his civilization to immerse himself in another one and garble their communications.
    - For every message he sends, it alternates between the following modes:
        - [**50%**] Raven (send incomprehensible garbage -including \ufeff for example)
        - [**40%**] Mimicry (repeat every message he hears)
        - [**10%**] Parrot (repeat what he hears after modifying it substantially)


Job Assignment System
----------------------

The Elder assigns jobs dynamically, using our team's broadcast protocol.
Each player listens for instructions and responds appropriately.

Communication Protocol
-----------------------

All inter-player instructions passed through team broadcasts during the game are defined below:

.. list-table:: Job Protocol
   :header-rows: 1
   :widths: 10 25 25 25 15

   * - Job
     - Instruction (Recv)
     - Instruction (Send)
     - Effect
     - Args / Type
   * - Basic
     - ``assign_job <id> <job>``
     - ``evolution_ready <level>``
     - Elder-directed reassignment; immediate job transition
     - id: ``int``, job: ``str``
   * -
     - ``evolution_call <count>``
     - ``evolution_response <id>``
     - Respond to group evolution call
     - id: ``int``
   * -
     - ``status_report_request``
     - ``basic_status level:<L> needs:<N> food:<F>``
     - Periodic status update to elder
     - level: ``int``, needs: ``int``, food: ``int``
   * -
     - ``resource_request <res> <amt>``
     - ``resource_available <res>``
     - Respond if excess resource is held
     - resource: ``str``, amt: ``int``
   * - Elder
     - ``newcomer_seeking_assignment``
     - ``assign_job <id> <job>``
     - Assigns newcomer to job
     - id: ``int``, job: ``str``
   * -
     - ``resource_delivery <res> <amt>``
     - ``status_report_request``
     - Update depot stock from collector; ask for reports
     - resource: ``str``, amt: ``int``
   * -
     -
     - ``elder_location <x> <y>``
     - Updates elder position for everyone
     - x: ``int``, y: ``int``
   * - Newcomer
     - ``elder_announce <id>``
     - ``newcomer_seeking_assignment <level>``
     - Announces presence and level; awaits elder reply
     - id: ``int``, level: ``int``
   * -
     - ``assign_job <id> <job>``
     - ``newcomer_joined``
     - Elder assigns role; confirms join
     - id: ``int``, job: ``str``
   * - Collector
     -
     - ``resource_delivery <res> <amt>``
     - Dispose of the collected stones on the Elder's tile
     - resource: ``str``, amt: ``int``
   * - Garbler
     -
     - *variable garbage*
     - Disruption: random unreadable text
     - ``str``
   * -
     -
     - *intercepted_message (mimicry/parrot)*
     - Disruption: repeated or modified enemy message
     - ``str``
   * -
     - *any enemy message*
     -
     - Stores for mimicry or parrot use
     - ``str``

Behavior Philosophy
--------------------

This system was designed to:

- Decentralize coordination via broadcast-based communication.
- Have a single Elder coordinate overall strategy.
- Allow role reassignment and adaptability during play.
- Disrupt enemy teams by embedding Garblers who mimic or garble enemy messages.

Although unfinished, this "Job System" represents an ambitious attempt to create
complex AI behaviors resembling a true in-game society.
