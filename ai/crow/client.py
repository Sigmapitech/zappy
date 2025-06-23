from functools import partial

from ai.crow.async_client import AsyncSocketClient

from .command_manager import CommandManager


class Client(CommandManager):
    _broadcast_receiver_callback = None

    def __init__(self, host: str, port: int):
        """
        Initializes the client with the specified host and port.

        Args:
            host (str): The hostname or IP address to connect to.
            port (int): The port number to connect to.

        The constructor sets up an asynchronous socket client using the provided host and port.
        If a broadcast receiver callback is defined, it is used; otherwise, a default no-op callback is provided.
        """
        __callback = None

        if self._broadcast_receiver_callback is not None:
            __callback = self._broadcast_receiver_callback

        socket = AsyncSocketClient(host, port, __callback or (lambda *_: _))
        super().__init__(sock=socket)

    async def connect(self, team: str):
        """
        Asynchronously connects to the server using the specified team name.

        This method performs the following steps:
        1. Establishes a connection to the server socket.
        2. Waits for a "WELCOME" message from the server.
        3. Sends the team name to the server.
        4. Receives and stores the client ID assigned by the server.
        5. Receives and prints the map size information.

        Args:
            team (str): The name of the team to connect as.

        Returns:
            tuple: A tuple containing the client ID message and the map size message, both as strings.
        """
        await self._sock.connect()

        await self._sock._wait_for_exact("WELCOME")
        await self._sock._send_line(team)

        id_msg = await self._sock._next_non_message()
        self.client_id = id_msg.strip()

        print(f"Connected with ID: {self.client_id}")

        map_size = await self._sock._next_non_message()
        print(f"map size: {map_size}")
        return id_msg, map_size

    @classmethod
    def broadcast_receiver(cls, func):
        """
        Decorator to register a method as the broadcast receiver callback.

        This method ensures that only one broadcast receiver callback can be registered
        per class. If a callback is already set, it raises a ValueError.

        Args:
            func (callable): The function to be registered as the broadcast receiver.

        Returns:
            callable: The original function, unmodified.

        Raises:
            ValueError: If a broadcast receiver callback has already been registered.
        """
        if cls._broadcast_receiver_callback is not None:
            raise ValueError("Cannot have 2 broadcast receive method")

        cls._broadcast_receiver_callback = func
        return func

    async def run_until_death(self):
        """
        Continuously executes broadcast and look operations in an infinite loop.

        This asynchronous method repeatedly calls the `broadcast` method with the message "plop"
        and the `look` method, printing their results to the standard output. The loop runs indefinitely
        until externally interrupted or the process is terminated.

        Note:
            - The method is intended to run until the instance is "dead" or stopped.
            - There is a TODO to wait for all child tasks to complete after the loop ends.
        """
        while True:
            print(await self.broadcast("plop"))
            print(await self.look())

        # todo: wait for all child to complete
