Message Ciphering
==================

Our AI agents can encode/decode messages for secure communication within the team.

Overview
--------

We use a symmetric cipher with XOR + rotation and PBKDF2-derived team keys.

Key Derivation
--------------

.. code-block:: python

   from hashlib import pbkdf2_hmac

   def derive_team_key(team_name: str) -> bytes:
       return pbkdf2_hmac(
           'sha256', team_name.encode(), b'3301', 10000, 2048
       )

- Salt: 3301
- Iterations: 10,000
- Output length: 2048 bytes

Encryption Process
-------------------

1. Derive key from team name.
2. Choose rotation = counter % len(key).
3. XOR content with rotated key.
4. FNV-1a checksum over encrypted content.
5. Pack header:

::

   bot_id | counter | rotation | timestamp | checksum | length

6. Concatenate header + encrypted payload.
7. Nibble-encode:

::

   LUT = "AIOU aiou qpdb QPDB"

Decoding Process
-----------------

1. Nibble-decode string.
2. Unpack header.
3. Validate checksum.
4. XOR-decrypt using rotation.
5. Parse content.

Code Example
------------

.. literalinclude:: ../path/to/your/code.py
   :language: python

Alternatively, show excerpts explaining the header format and nibble encoding:

.. code-block:: python

   def build_header(bot_id, counter, rotation, timestamp, checksum, length):
       return struct.pack(">HBBI2sH", ...)

Nibble Encoding
---------------

Uses a 16-character LUT:

::

   AIOU aiou qpdb QPDB

- Maps high/low nibbles to characters.
- Enables safe transmission over text channels.

