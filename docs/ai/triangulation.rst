Triangulation Strategy
=======================

Sound propagation in Zappy is toroidal. The map wraps horizontally and vertically.

When a message is broadcast, all players receive:

::

   message K, text

Where K is the tile direction relative to the receiver.

Strategy
--------

- Build a lookup of relative positions vs. directions.
- Use map width/height to compute shortest wraparound.
- When multiple paths exist, choose the shortest.

Diagram
-------

.. figure:: /_static/sound_diagram.png
   :alt: Sound direction example
   :align: center

   Example of tile numbering and shortest path for sound.

Implementation Idea
--------------------

- On receiving message, compare possible paths.
- Simulate wraparound to find the shortest path to sender.
- Store recent directions for source estimation over time.

