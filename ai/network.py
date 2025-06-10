import logging
import socket
import time

logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)
formatter = logging.Formatter(
    "%(asctime)s - %(name)-11s - %(levelname)-8s - %(message)s",
    datefmt="%H:%M:%S",
)

stream_handler = logging.StreamHandler()
stream_handler.setFormatter(formatter)

logger.addHandler(stream_handler)


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
                    f"Connection failed: {e}. Retrying in {self.delay} seconds..."
                )
                time.sleep(self.delay)
        raise Exception(
            "Could not connect to the server after multiple attempts"
        )

    def send_message(self, message: str):
        self.sock.sendall(f"{message}\n".encode("ascii"))

    def receive_message(self) -> str:
        return self.sock.recv(self.buffer_size).decode("ascii").strip()

    def close(self):
        if self.sock:
            self.sock.close()
