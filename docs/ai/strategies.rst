AI Strategies
=============

This page describes several strategies we considered (and partly implemented or designed) for coordinating our AI players. Each approach balances complexity, coordination overhead, and robustness to adversarial teams.

Our goal was to provide flexibility to assign strategies dynamically depending on the game context.

1. Elder-Centric Hub Strategy
------------------------------

**Description:**

- Designate a single Elder as the "home base."
- Elder stays on a chosen central tile.
- All other players report status, deliver resources there, and await assignments.
- Elder decides when and where to launch incantations, assigns roles dynamically.

**Pros:**

- Centralized control enables fine-grained planning.
- Makes it easy to stockpile stones on one tile.
- Elder can optimize group formation for incantation.

**Cons:**

- Single point of failure (if Elder dies, assignment collapses).
- Requires all players to stay in communication range.
- Easy for enemies to locate and disrupt the hub.

**Best Use:**

- Large, coordinated team with stable communication.
- When enemy interference is minimal.

---

2. Opportunistic Evolution
---------------------------

**Description:**

- No Elder.
- Each player roams the map collecting stones independently.
- When a player has all stones for next level, it broadcasts a "call" for all same-level teammates nearby.
- Players move to the caller to attempt evolution.

**Pros:**

- Fully decentralized; no single leader.
- Resistant to Elder assassination or communication jams.
- Scales well with many players.

**Cons:**

- Harder to synchronize multiple players of same level.
- Players may fail to converge in time.
- Resource waste due to duplication (multiple players carry same stones).

**Best Use:**

- When the map is sparse or enemy interference breaks centralized planning.
- Small teams with limited coordination.

---

3. GPS / Triangulation Network
-------------------------------

**Description:**

- Assign specific players to act as "beacons" or "GPS nodes."
- Each beacon broadcasts its location regularly.
- Other players can triangulate their own positions by comparing signal directions.
- Enables distributed coordination: players can navigate to any rendezvous point reliably.

**Pros:**

- Map-wide navigation aid for all team members.
- Supports sophisticated positioning strategies (e.g. flanking, resource delivery).
- Works even on large maps with wraparound.

**Cons:**

- Requires dedicated players reducing effective workforce.
- Beacons must stay alive and avoid enemy ejection.
- Communication can be jammed or mimicked by enemies.

**Best Use:**

- Large teams wanting precise coordination.
- Maps with complex topology or high wraparound confusion.

---

4. Raider/Disruptor Cells
--------------------------

**Description:**

- Divide the team into small semi-autonomous cells.
- Assign a few players as Raiders or Disruptors.
- Raiders move toward enemy hubs to eject, steal, or confuse.
- Remaining players continue resource gathering and elevation.

**Pros:**

- Harasses enemy coordination (e.g. ejecting their Elder).
- Can disrupt enemy incantations.
- Forces enemies to spend resources defending.

**Cons:**

- Raiders often die or get stranded.
- Reduces overall resource-gathering capacity.
- Risk of friendly communication noise.

**Best Use:**

- When enemy teams are tightly coordinated.
- To delay enemy elevation while your team advances.

---

5. Resource Grid Delivery
--------------------------

**Description:**

- Players spread out in a grid to maximize map coverage.
- When stones are found, players broadcast availability.
- Nearest Collector or Basic player comes to fetch and deliver to the hub.
- Elder (or distributed leader) decides where incantations occur.

**Pros:**

- Efficient map-wide resource exploitation.
- Minimizes redundant searching.
- Adaptive to dynamic spawns.

**Cons:**

- High communication overhead.
- Needs strong coordination logic to avoid conflict over resources.
- Vulnerable to broadcast jamming.

**Best Use:**

- Medium to large teams.
- When resource density is low or scattered.

---

6. Egg Farming & Fast-Fork Expansion
------------------------------------

**Description:**

- Focus on forking early and often.
- Maintain a steady supply of eggs to keep slots full.
- Assign newcomers rapidly to resource or collector roles.
- Prioritize food gathering to sustain population.

**Pros:**

- Maximizes team population quickly.
- Provides backup even if some players die.
- Can overwhelm enemy teams via numbers.

**Cons:**

- Needs careful food management.
- Coordination becomes harder as numbers grow.
- Slower to reach higher elevations if stone stockpiles are neglected.

**Best Use:**

- On large maps with abundant food.
- Against teams that can't coordinate their expansions.

---

Choosing a Strategy
--------------------

In practice, the AI system was designed to switch between these strategies depending on:

- Number of connected players.
- Current resource stockpiles.
- Enemy activity (e.g. presence of Garblers).
- Map size and density.

By mixing these approaches, we hoped to create a robust and adaptable AI civilization capable of winning under varied conditions.

This flexibility allows the AI to respond dynamically to the evolving game state, maximizing its chances of success against both human and AI opponents.
