import asyncio
import logging
import socket

logger = logging.getLogger(__name__)


class AsyncSocketClient:

    def __init__(self, host, port, broadcast_callback):
        """
        Initializes the AsyncClient instance.

        Args:
            host (str): The hostname or IP address of the server to connect to.
            port (int): The port number of the server to connect to.
            broadcast_callback (Callable): A callback function to handle broadcast messages.

        Attributes:
            _host (str): The server hostname or IP address.
            _port (int): The server port number.
            _sock (socket.socket): The non-blocking socket used for communication.
            _loop (asyncio.AbstractEventLoop): The event loop for asynchronous operations.
            _recv_task (Optional[asyncio.Task]): The task handling incoming data reception.
            _incoming_queue (asyncio.Queue): Queue for storing incoming messages.
            command_lock (asyncio.Lock): Lock to ensure command execution order.
            _connected (bool): Connection status flag.
            _broadcast_callback (Callable): Callback for handling broadcast messages.
        """
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
        """
        Asynchronously establishes a socket connection to the specified host and port.

        This method connects the internal socket to the target address using the event loop,
        sets the connection status to True, and starts the background receiver loop as an asyncio task.

        Raises:
            OSError: If the connection attempt fails.
        """
        await self._loop.sock_connect(self._sock, (self._host, self._port))
        self._connected = True
        self._recv_task = asyncio.create_task(self._receiver_loop())

    async def send_command(self, command: str) -> str:
        """
        Sends a command asynchronously to the server and returns the response.

        Args:
            command (str): The command string to send.

        Returns:
            str: The response from the server.

        Raises:
            ConnectionError: If the client is not connected.

        Notes:
            This method acquires a lock to ensure that commands are sent sequentially.
        """
        async with self.command_lock:
            if not self._connected:
                raise ConnectionError("Not connected")
            logger.debug("-> %s", command)
            await self._send_line(command)
            return await self._next_non_message()

    async def _next_non_message(self) -> str:
        """
        Asynchronously retrieves the next non-message item from the incoming queue.

        This coroutine continuously processes messages from the internal queue. If a message starts with "message ", it parses the direction and content, validates the direction, and invokes the broadcast callback. If the message is "dead", it closes the client and raises a ConnectionError. For any other message, it logs and returns the message.

        Returns:
            str: The next non-message string from the incoming queue.

        Raises:
            ConnectionError: If the client receives a "dead" message and needs to close.
        """
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

            logger.debug("<- %s", msg)
            return msg

    async def _wait_for_exact(self, expected: str):
        """
        Waits asynchronously for a specific expected message.

        Continuously retrieves the next non-message from the message stream until the received message matches the expected value.
        Raises:
            ValueError: If a message different from the expected one is received.
        Args:
            expected (str): The message to wait for.
        """
        while True:
            msg = await self._next_non_message()
            if msg == expected:
                return
            raise ValueError(f"Unexpected message: {msg}")

    async def _send_line(self, line: str):
        """
        Asynchronously sends a line of text over a socket, appending a newline character.

        Args:
            line (str): The line of text to send.

        Raises:
            Any exceptions raised by the underlying socket send operation.
        """
        await self._loop.sock_sendall(self._sock, (line + "\n").encode())

    async def _receiver_loop(self):
        """
        Asynchronously receives data from a socket, processes incoming messages line by line, and puts them into the incoming queue.

        Continuously reads data from the socket while the connection is active. Accumulates data in a buffer until a newline character is found, then decodes and strips each complete message before placing it into the incoming queue. If the connection is closed or an error occurs, marks the connection as disconnected and puts a "dead" message into the queue to signal termination.

        Raises:
            Any exceptions raised by socket operations or queue operations.
        """
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
        """
        Asynchronously closes the client connection.

        This method sets the connection state to disconnected, cancels any ongoing
        receive task if present, and closes the underlying socket.
        """
        self._connected = False
        if self._recv_task:
            self._recv_task.cancel()

        self._sock.close()
