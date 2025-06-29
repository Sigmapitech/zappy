## Legacy AI behavior

This (unfinished) AI is based on well-defined behaviors called **Jobs**. Every job has its
own file in the `ai/legacy/` directory (former `ai/behavior/`), containing its implementation
of the abstract class `JobTemplate`. This class defines the layout for a *job well done*.

* `Newcomer`
    - Every player that spawns in the map have this job as default; the player can be assigned
    a job by the already existing `Elder`, or become one if no `Elder` broadcast has been
    detected for 50 ticks.
* `Elder`
    - There can only be one per civilization, as it is the *quite literal* pillar of this
    species.
    - Will choose a tile that's in the center of its civilization, to receive stones, food and
    players to perform the incantation on this home tile.
    - Dispatches newcomers around different jobs.
    - Knows every player on the map, and their job. `Garblers` are an exception and aren't
    precisely counted. To account for that, they're not assigned often to players.
* `Basic`
    - Collects the stones that are in its way, randomly pathing around the map, and sends
    a broadcast when he has enough stones to evolve.
* `Collector`
    - Collects stones and drops them off on the tile of the `Elder`. As simple as that.
* `Garbler`
    - Goes away from his civilization to immerse himself in another one and garble their
    communications.
    - For every message he sends, it alternates between the following modes:
        + [**50%**] Raven (send incomprehensible garbage -including `\feff` for example)
        + [**40%**] Mimicry (repeat every message he hears)
        + [**10%**] Parrot (repeat what he hears after modifying it substantially)

---

Every instruction that can pass through the team's broadcast system during the game is
detailed here:

| Job       | Instruction (Recv)               | Instruction (Send)                              | Effect                                                    | Args / Type                             |
|-----------|----------------------------------|-------------------------------------------------|-----------------------------------------------------------|-----------------------------------------|
| Basic     | `assign_job <id> <job>`          | `evolution_ready <level>`                       | Elder-directed reassignment; immediate job transition     | id: `int`, job: `str`                   |
|           | `evolution_call <count>`         | `evolution_response <id>`                       | Respond to group evolution call                           | id: `int`                               |
|           | `status_report_request`          | `basic_status level:<L> needs:<N> food:<F>`     | Periodic status update to elder                           | level: `int`, needs: `int`, food: `int` |
|           | `resource_request <res> <amt>`   | `resource_available <res>`                      | Respond if excess resource is held                        | resource: `str`, amt: `int`             |
| Elder     | `newcomer_seeking_assignment`    | `assign_job <id> <job>`                         | Assigns newcomer to job                                   | id: `int`, job: `str`                   |
|           | `resource_delivery <res> <amt>`  | `status_report_request`                         | Update depot stock from collector; ask for reports        | resource: `str`, amt: `int`             |
|           |                                  | `elder_location <x> <y>`                        | Updates elder position for everyone                       | x: `int`, y: `int`                      |
| Newcomer  | `elder_announce <id>`            | `newcomer_seeking_assignment <level>`           | Announces presence and level; awaits elder reply          | id: `int`, level: `int`                 |
|           | `assign_job <id> <job>`          | `newcomer_joined`                               | Elder assigns role; confirms join                         | id: `int`, job: `str`                   |
| Collector |                                  | `resource_delivery <res> <amt>`                 | Dispose of the collected stones on the Elder's tile       | resource: `str`, amt: `int`             |
| Garbler   |                                  | *variable garbage*                              | Disruption: random unreadable text                        | `str`                                   |
|           |                                  | *intercepted_message (mimicry/parrot)*          | Disruption: repeated or modified enemy message            | `str`                                   |
|           | *any enemy message*              |                                                 | Stores for mimicry or parrot use                          | `str`                                   |
