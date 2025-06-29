Message Ciphering
==================

Our AI agents implement a custom encoding/decoding scheme to allow secure
communication among teammates. This page explains the chosen design in detail,
as well as other models and improvements we considered.

Overview
--------

We use multiple components to ensure secure communication:

- PBKDF2-derived team keys: generates a shared, team-specific secret from the
team name using a password-based key derivation function, ensuring all
teammates have the same base key without exchanging it in-game.
- XOR with rotation offset: performs lightweight symmetric encryption by
combining the message with the derived key at an offset, obfuscating the
content from opponents.
- FNV-1a checksumming: computes a short hash of the encrypted payload to
detect tampering or corruption during transmission.
- Custom nibble-encoding: converts binary data into safe text-only messages
using a 16-character lookup table, ensuring compatibility with the game's
line-based, text-only protocol.

This design balances simplicity, speed, and the severe constraints of
the game's communication system (small, plain-text messages with no binary).

Key Derivation
--------------

We derive a team-wide symmetric key from the team name using PBKDF2:

.. code-block:: python

   from hashlib import pbkdf2_hmac

   def derive_team_key(team_name: str) -> bytes:
       return pbkdf2_hmac(
           'sha256', team_name.encode(), b'3301', 10000, 2048
       )

- Salt: `b"3301"`
- Iterations: 10,000
- Output length: 2048 bytes

✅ Pros:
- Each team automatically gets a unique key from its name.
- Harder for opponents to brute-force without knowing the team name.

✅ Cons:
- All team members share the same key (no per-player secrecy).

Encryption Process
-------------------

1. Derive key from team name.
2. Compute `rotation = counter % len(key)`.
3. XOR the plaintext content with the key, offset by rotation.
4. Compute FNV-1a checksum over the encrypted content.
5. Pack a header:

::

   bot_id | counter | rotation | timestamp | checksum | length

6. Concatenate header + encrypted payload.
7. Nibble-encode the result:

::

   LUT = "AIOU aiou qpdb QPDB"

✅ Pros:
- Very fast encryption/decryption (suitable for real-time bot).
- Obfuscates content enough to confuse naive interceptors.
- Includes integrity check (checksum) to detect tampering.

✅ Cons:
- Relatively weak cryptography if opponent knows the team name.
- No perfect secrecy if rotation/counter reused.

Decoding Process
-----------------

1. Nibble-decode the string.
2. Unpack the header.
3. Validate the checksum.
4. XOR-decrypt using rotation.
5. Parse the content as ASCII.

Header Format
--------------

.. code-block:: python

   def build_header(bot_id, counter, rotation, timestamp, checksum, length):
       return struct.pack(">HBBI2sH", bot_id, counter, rotation, timestamp, checksum, length)

- `bot_id`: Unique player ID.
- `counter`: Monotonic message counter.
- `rotation`: For XOR offset.
- `timestamp`: Unix time at message creation.
- `checksum`: FNV-1a checksum over ciphertext.
- `length`: Encrypted payload length.

Nibble Encoding
---------------

We use a custom 16-character LUT to map binary data into text:

::

   AIOU aiou qpdb QPDB

- Each byte is split into two nibbles (4 bits).
- Each nibble maps to a single LUT character.
- Guarantees safe transmission through text-only channels.

✅ Pros:
- Avoids forbidden binary characters in communication.
- Lightweight and easy to decode.

✅ Cons:
- Doubles message length.

Code Example
------------

.. literalinclude:: ../path/to/your/code.py
   :language: python

Alternative Models Considered
-----------------------------

During design, we evaluated other encoding/encryption models:

**1. Full Symmetric Encryption (AES, ChaCha20)**
- Pros:
  - Strong cryptographic security.
  - Widely available, well-studied algorithms.
- Cons:
  - Requires binary-safe channels (the game only accepts plain text).
  - Much slower on low-powered bots.
  - Not feasible within the text-based, line-delimited protocol.

**2. Per-Player Individual Keys**
- Pros:
  - Better secrecy (messages can be addressed to a single bot).
  - Prevents compromised teammate from leaking all keys.
- Cons:
  - Requires key-exchange in-game (forbidden by project spec).
  - Team-based communication makes single-player keys impractical.
  - Harder to coordinate multiple bots for incantation.

**3. Asymmetric Cryptography (RSA, ECC)**
- Pros:
  - Strongest security.
  - Supports signatures for message authenticity.
- Cons:
  - Far too slow for real-time usage on many bots.
  - Requires large message sizes, incompatible with nibble encoding.
  - Overkill given the adversary model (mostly meant to confuse, not
  for military-grade secrecy).

**4. Simple Substitution or Caesar Cipher**
- Pros:
  - Very easy to implement.
  - Minimal overhead.
- Cons:
  - Breakable by any opponent listening to multiple messages.
  - Not robust even against random guessing.
  - Fails to meet our minimum obfuscation requirement.

Security Improvements Considered
--------------------------------

We also thought about ways to make our existing scheme safer:

**A. Self-validation Using Previous Messages**
- Idea:
  - Include hashes of previous messages in the new one to form a chain.
- Pros:
  - Detects replay attacks.
  - Ensures message sequence integrity.
- Cons:
  - Longer messages, less room for content.
  - Complicated state-tracking across many independent bots.
  - Difficult to resynchronize after disconnects.

**B. Individual Counters Per Bot**
- Idea:
  - Each bot tracks its own counter for rotation.
- Pros:
  - Reduces risk of key rotation reuse across bots.
  - Slightly harder to guess.
- Cons:
  - Requires reliable synchronization of counters.
  - Risk of drift if bots miss messages or reconnect.
  - Added complexity for small security gain.

**C. Full Key Cipher Instead of XOR**
- Idea:
  - Encrypt entire payload with AES or ChaCha20.
- Pros:
  - High security.
- Cons:
  - Computationally expensive.
  - Large binary outputs unsuitable for nibble encoding.
  - Exceeds game's communication limits (plain-text line-based protocol).

Why We Didn't Use Them
-----------------------

We chose our design for being:

- Lightweight enough for many AI clients to run simultaneously.
- Compatible with the game's text-based, line-delimited communication protocol.
- Sufficiently obfuscating to confuse other teams without over-engineering.
- Fast to encode/decode in Python even on limited hardware.

Ultimately, given the game's constraints (no binary, limited bandwidth,
no out-of-band key exchange), we accepted the trade-off of using a shared
team key with modest obfuscation over unbreakable cryptography.

