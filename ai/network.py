import logging
import socket
import time

logger = logging.getLogger(__name__)


class Network:
    def __init__(
        self, server_address: tuple, retries: int = 3, delay: int = 5
    ):
        self.server_address = server_address
        self.sock = None
        self.buffer_size = 4096
        self.retries = retries
        self.delay = delay

    def connect(self):
        for _ in range(self.retries):
            try:
                self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.sock.connect(self.server_address)
                return
            except socket.error as e:
                logger.warning(
                    f"Connection failed: %s. Retrying in %s seconds...",
                    e,
                    self.delay,
                )
                time.sleep(self.delay)
        raise Exception(
            "Could not connect to the server after multiple attempts"
        )

    def send_message(self, message: str):
        logger.debug("-> %s", message)
        self.sock.sendall(f"{message}\n".encode("ascii"))

    def receive_message(self) -> str:
        received = self.sock.recv(self.buffer_size).decode("ascii").strip()
        logger.debug("<- %s", received)
        return received

    def close(self):
        if self.sock:
            self.sock.close()
