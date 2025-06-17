import asyncio
import logging
import socket

logger = logging.getLogger(__name__)


class AsyncSocketClient:

    def __init__(self, host, port, broadcast_callback):
        self._host = host
        self._port = port
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._sock.setblocking(False)
        self._loop = asyncio.get_event_loop()

        self._recv_task = None
        self._incoming_queue = asyncio.Queue()
        self.command_lock = asyncio.Lock()

        self._connected = False
        self._broadcast_callback = broadcast_callback

    async def connect(self):
        await self._loop.sock_connect(self._sock, (self._host, self._port))
        self._connected = True
        self._recv_task = asyncio.create_task(self._receiver_loop())

    async def send_command(self, command: str) -> str:
        async with self.command_lock:
            if not self._connected:
                raise ConnectionError("Not connected")
            logger.debug("-> %s", command)
            await self._send_line(command)
            return await self._next_non_message()

    async def _next_non_message(self) -> str:
        while True:
            msg = await self._incoming_queue.get()

            if msg.startswith("message "):
                _, direction, content = msg.split(" ")

                direction = direction.removesuffix(",")
                if not direction.isdigit():
                    continue

                if content[0] == '"' and content[-1] == '"':
                    content = content[1:-1]

                await self._broadcast_callback(int(direction), content)
                continue

            if msg == "dead":
                await self.close()
                raise ConnectionError("Client died.")

            return msg

    async def _wait_for_exact(self, expected: str):
        while True:
            msg = await self._next_non_message()
            if msg == expected:
                return
            raise ValueError(f"Unexpected message: {msg}")

    async def _send_line(self, line: str):
        await self._loop.sock_sendall(self._sock, (line + "\n").encode())

    async def _receiver_loop(self):
        buffer = b""
        try:
            while self._connected:
                data = await self._loop.sock_recv(self._sock, 4096)
                if not data:
                    break
                buffer += data
                while b"\n" in buffer:
                    line, buffer = buffer.split(b"\n", 1)
                    message = line.decode().strip()
                    await self._incoming_queue.put(message)
        finally:
            self._connected = False
            await self._incoming_queue.put("dead")

    async def close(self):
        self._connected = False
        if self._recv_task:
            self._recv_task.cancel()

        self._sock.close()
