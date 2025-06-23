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
        timestamp (int): Unix timestamp indicating when the message was sent.
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
    Encrypts or decrypts data using an XOR cipher with a repeating key and an optional offset.

    Args:
        data (bytes): The input data to be encrypted or decrypted.
        key (bytes): The key used for the XOR operation. It is repeated as necessary.
        offset (int): The starting offset to apply to the key for the XOR operation.

    Returns:
        bytes: The result of XOR-ing the input data with the key, starting at the given offset.

    Note:
        XOR cipher is symmetric; applying the function twice with the same key and offset restores the original data.
    """
    return bytes(b ^ key[(i + offset) % len(key)] for i, b in enumerate(data))


def fnv1a_16(data: bytes) -> bytes:
    """
    Computes the 16-bit FNV-1a hash of the given data.

    Args:
        data (bytes): Input data to hash.

    Returns:
        bytes: The last 2 bytes (16 bits) of the 32-bit FNV-1a hash, in big-endian order.
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
        rotation (int): The rotation value (unsigned char, 1 byte).
        timestamp (int): The timestamp (unsigned int, 4 bytes).
        checksum (bytes): The checksum (2 bytes).
        length (int): The length of the message (unsigned short, 2 bytes).

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
    Encodes a bytes object into a string using a nibble-based lookup table.

    Each byte in the input is split into its high and low 4-bit nibbles,
    which are then mapped to characters using the LUT lookup table.

    Args:
        data (bytes): The input bytes to encode.

    Returns:
        str: The encoded string representation of the input bytes.

    Raises:
        NameError: If LUT is not defined in the scope.
    """
    encoded = ""
    for byte in data:
        hi, lo = byte >> 4, byte & 0x0F
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
    Encodes a message with encryption, header, and nibble encoding.

    Args:
        bot_id (int): Identifier for the bot sending the message.
        counter (int): Message counter, used for rotation and tracking.
        content (bytes): The message content to be encoded.
        team_key (bytes): The secret key used for encryption.

    Returns:
        str: The encoded message as a nibble-encoded string.

    Raises:
        ValueError: If input parameters are invalid or encoding fails.

    Process:
        - Gets the current timestamp.
        - Calculates rotation based on the counter and key length.
        - Encrypts the content using an XOR cipher with the team key and rotation.
        - Computes a 16-bit FNV-1a checksum of the encrypted content.
        - Builds a header containing metadata.
        - Concatenates the header and encrypted content.
        - Encodes the result using nibble encoding.
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
        DecodedMessage | None: A DecodedMessage object containing the decoded information if successful,
        or None if decoding fails (e.g., due to checksum mismatch or decryption error).

    Raises:
        None: All exceptions are caught and logged; function returns None on failure.

    Notes:
        - The message is expected to be nibble-decoded, then unpacked according to the specified header format.
        - The function verifies the checksum before attempting decryption.
        - Decryption uses an XOR cipher with the provided team key and rotation value.
    """
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
