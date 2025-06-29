import logging
import struct
import time
from dataclasses import dataclass
from hashlib import pbkdf2_hmac

logger = logging.getLogger(__name__)


@dataclass
class DecodedMessage:
    """
    Represents a decoded message with metadata.

    Attributes:
        bot_id (int): Identifier of the bot that sent the message.
        counter (int): Sequence number or counter for the message.
        timestamp (int): Timestamp indicating when the message was sent or received.
        content (str): The actual content of the decoded message.
    """

    bot_id: int
    counter: int
    timestamp: int
    content: str


def derive_team_key(team_name: str) -> bytes:
    """
    Derives a cryptographic key from a team name using PBKDF2-HMAC-SHA256.

    Args:
        team_name (str): The name of the team to derive the key from.

    Returns:
        bytes: The derived key as a byte string.

    Notes:
        - Uses a fixed salt (b"3301"), 10,000 iterations, and a key length of 2048 bytes.
        - The derived key is suitable for cryptographic operations requiring a strong key.
    """
    return pbkdf2_hmac(
        hash_name="sha256",
        password=team_name.encode(),
        salt=b"3301",
        iterations=10000,
        dklen=2048,
    )


def xor_cipher(data: bytes, key: bytes, offset: int) -> bytes:
    """
    Encrypts or decrypts data using a repeating-key XOR cipher with an optional offset.

    Args:
        data (bytes): The input data to be encrypted or decrypted.
        key (bytes): The key used for the XOR operation.
        offset (int): The starting offset to apply to the key for the XOR operation.

    Returns:
        bytes: The result of XOR-ing the input data with the key, starting at the given offset.

    Example:
        >>> xor_cipher(b'hello', b'key', 0)
        b'\r\x00\x15\x07\x0a'
    """
    return bytes(b ^ key[(i + offset) % len(key)] for i, b in enumerate(data))


def fnv1a_16(data: bytes) -> bytes:
    """
    Computes the 16-bit FNV-1a hash of the given data.

    Args:
        data (bytes): The input data to hash.

    Returns:
        bytes: The 2-byte (16-bit) FNV-1a hash of the input data, in big-endian order.
    """
    FNV_PRIME = 0x01000193
    h = 0x811C9DC5

    for byte in data:
        h ^= byte
        h = (h * FNV_PRIME) & 0xFFFFFFFF
    return h.to_bytes(4, "big")[-2:]


def build_header(
    bot_id, counter, rotation, timestamp, checksum, length
) -> bytes:
    """
    Builds a binary header for a message using the specified parameters.

    Args:
        bot_id (int): The unique identifier for the bot (unsigned short, 2 bytes).
        counter (int): A counter value (unsigned char, 1 byte).
        rotation (int): Rotation value (unsigned char, 1 byte).
        timestamp (int): Timestamp value (unsigned int, 4 bytes).
        checksum (bytes): Checksum value (2 bytes).
        length (int): Length of the message (unsigned short, 2 bytes).

    Returns:
        bytes: The packed header as a bytes object in big-endian order.
    """
    return struct.pack(
        ">HBBI2sH", bot_id, counter, rotation, timestamp, checksum, length
    )


LUT = "AIOU aiou qpdb QPDB".replace(" ", "")
LUT_REV = {c: i for i, c in enumerate(LUT)}


def nibble_encode(data: bytes) -> str:
    """
    Encodes a bytes object into a string using nibble (4-bit) encoding.

    Each byte is split into its high and low 4-bit nibbles, which are then mapped
    to characters using the LUT lookup table and concatenated to form the encoded string.

    Args:
        data (bytes): The input bytes to encode.

    Returns:
        str: The nibble-encoded string representation of the input bytes.

    Raises:
        NameError: If LUT is not defined in the scope.
    """
    encoded = ""
    for byte in data:
        hi, lo = divmod(byte, 16)
        encoded += LUT[hi] + LUT[lo]
    return encoded


def nibble_decode(text: str) -> bytes:
    """
    Decodes a nibble-encoded string into its original bytes.

    A nibble-encoded string represents each byte as two characters, where each character
    corresponds to a 4-bit nibble. The function uses a lookup table `LUT_REV` to map
    characters back to their 4-bit values.

    Args:
        text (str): The nibble-encoded string to decode. Must have even length.

    Returns:
        bytes: The decoded bytes.

    Raises:
        ValueError: If the input string has an odd length.
        KeyError: If a character in the input string is not present in `LUT_REV`.
    """
    if len(text) & 1:
        raise ValueError("Nibble-encoded string must have even length.")

    data = []
    for i in range(0, len(text), 2):
        hi = LUT_REV[text[i]]
        lo = LUT_REV[text[i + 1]]
        data.append((hi << 4) | lo)
    return bytes(data)


def encode(bot_id: int, counter: int, content: bytes, team_key: bytes) -> str:
    """
    Encodes a message with encryption and metadata for secure transmission.

    Args:
        bot_id (int): Identifier for the bot sending the message.
        counter (int): Message counter, used for rotation and replay protection.
        content (bytes): The raw message content to be encrypted and sent.
        team_key (bytes): The secret key used for encryption.

    Returns:
        str: The encoded message as a nibble-encoded string, including header and encrypted content.
    """
    timestamp = int(time.time())
    rotation = counter % len(team_key)
    encrypted = xor_cipher(content, team_key, rotation)
    chksum = fnv1a_16(encrypted)
    length = len(encrypted)

    header = build_header(bot_id, counter, rotation, timestamp, chksum, length)
    message_bytes = header + encrypted
    return nibble_encode(message_bytes)


def decode(msg_str: str, team_key: bytes) -> DecodedMessage | None:
    """
    Decodes an encoded message string using the provided team key.

    Args:
        msg_str (str): The encoded message string to decode.
        team_key (bytes): The secret key used for decryption.

    Returns:
        DecodedMessage | None: Returns a DecodedMessage object if decoding is successful,
        or None if decoding fails due to checksum mismatch or other errors.

    Raises:
        None: All exceptions are caught and logged internally.
    """
    try:
        raw = nibble_decode(msg_str)
        header = raw[:12]
        bot_id, counter, rotation, timestamp, chksum, length = struct.unpack(
            ">HBBI2sH", header
        )

        encrypted = raw[12 : 12 + length]
        if fnv1a_16(encrypted) != chksum:
            logger.error("Checksum mismatch during decoding.")
            return None

        decrypted = xor_cipher(encrypted, team_key, rotation)
        return DecodedMessage(bot_id, counter, timestamp, decrypted.decode())
    except Exception as e:
        logger.error("Decoding failed: %s", e)
        return None
