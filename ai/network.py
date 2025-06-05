import socket
import time


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
                print(
                    f"Connection failed: {e}. Retrying in {self.delay} seconds..."
                )
                time.sleep(self.delay)
        raise Exception(
            "Could not connect to the server after multiple attempts"
        )

    def send_message(self, message: str):
        self.sock.sendall(f"{message}\n".encode("utf-8"))

    def receive_message(self) -> str:
        return self.sock.recv(self.buffer_size).decode("utf-8").strip()

    def close(self):
        if self.sock:
            self.sock.close()
