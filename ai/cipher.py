import logging
import struct
import time
from dataclasses import dataclass
from hashlib import pbkdf2_hmac

logger = logging.getLogger(__name__)


@dataclass
class DecodedMessage:
    bot_id: int
    counter: int
    timestamp: int
    content: str


def derive_team_key(team_name: str) -> bytes:
    return pbkdf2_hmac(
        hash_name="sha256",
        password=team_name.encode(),
        salt=b"3301",
        iterations=10000,
        dklen=2048,
    )


def xor_cipher(data: bytes, key: bytes, offset: int) -> bytes:
    return bytes(b ^ key[(i + offset) % len(key)] for i, b in enumerate(data))


def fnv1a_16(data: bytes) -> bytes:
    FNV_PRIME = 0x01000193
    h = 0x811C9DC5

    for byte in data:
        h ^= byte
        h = (h * FNV_PRIME) & 0xFFFFFFFF
    return h.to_bytes(4, "big")[-2:]


def build_header(
    bot_id, counter, rotation, timestamp, checksum, length
) -> bytes:
    return struct.pack(
        ">HBBI2sH", bot_id, counter, rotation, timestamp, checksum, length
    )


LUT = "AIOU aiou qpdb QPDB".replace(" ", "")
LUT_REV = {c: i for i, c in enumerate(LUT)}


def nibble_encode(data: bytes) -> str:
    encoded = ""
    for byte in data:
        hi, lo = byte >> 4, byte & 0x0F
        encoded += LUT[hi] + LUT[lo]
    return encoded


def nibble_decode(text: str) -> bytes:
    if len(text) & 1:
        raise ValueError("Nibble-encoded string must have even length.")

    data = []
    for i in range(0, len(text), 2):
        hi = LUT_REV[text[i]]
        lo = LUT_REV[text[i + 1]]
        data.append((hi << 4) | lo)
    return bytes(data)


def encode(bot_id: int, counter: int, content: bytes, team_key: bytes) -> str:
    timestamp = int(time.time())
    rotation = counter % len(team_key)
    encrypted = xor_cipher(content, team_key, rotation)
    chksum = fnv1a_16(encrypted)
    length = len(encrypted)

    header = build_header(bot_id, counter, rotation, timestamp, chksum, length)
    message_bytes = header + encrypted
    return nibble_encode(message_bytes)


def decode(msg_str: str, team_key: bytes) -> DecodedMessage | None:
    try:
        raw = nibble_decode(msg_str)
        header = raw[:12]
        bot_id, counter, rotation, timestamp, chksum, length = struct.unpack(
            ">HBBI2sH", header
        )

        encrypted = raw[12 : 12 + length]
        if fnv1a_16(encrypted) != chksum:
            return None

        decrypted = xor_cipher(encrypted, team_key, rotation)
        return DecodedMessage(
            bot_id,
            counter,
            timestamp,
            decrypted.decode("ascii"),
        )

    except Exception as e:
        logger.debug("Failed to decipher %s: %s", msg_str, e)
        return None
