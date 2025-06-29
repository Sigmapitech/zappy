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
        Asynchronously connects the client to the server using the specified team name.

        This method performs the following steps:
            1. Establishes a connection to the server socket.
            2. Waits for a "WELCOME" message from the server.
            3. Sends the team name to the server.
            4. Receives and stores the client ID assigned by the server.
            5. Receives and prints the map size from the server.

        Args:
            team (str): The name of the team to join.

        Returns:
            Tuple[str, str]: A tuple containing the client ID message and the map size message received from the server.
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

        This decorator ensures that only one broadcast receiver callback can be set
        per class. If a callback is already registered, it raises a ValueError.

        Args:
            func (Callable): The function to be registered as the broadcast receiver.

        Returns:
            Callable: The original function, registered as the broadcast receiver.

        Raises:
            ValueError: If a broadcast receiver callback has already been set.
        """
        if cls._broadcast_receiver_callback is not None:
            raise ValueError("Cannot have 2 broadcast receive method")

        cls._broadcast_receiver_callback = func
        return func

    async def run_until_death(self):
        """
        Continuously performs resource handling in an infinite loop until externally stopped.

        This asynchronous method repeatedly:
            1. Calls `look()` to observe or gather current resources.
            2. Passes the observed resources to `handle_resources()` for processing.
            3. Waits for 1 second before repeating the cycle.

        Intended to run as a long-lived coroutine, typically until the program or task is cancelled.
        """
        while True:
            resources = await self.look()
            await self.handle_resources(resources)
            await asyncio.sleep(1)

    async def handle_resources(self, resources):
        """
        Asynchronously handles resource gathering and evolution logic.

        Iterates through the provided resources, collecting those that are needed.
        If the conditions to evolve are met, drops all resources, broadcasts an evolution message,
        and gathers the group for evolution.

        Args:
            resources (Iterable): A collection of available resources to process.

        Returns:
            None

        Raises:
            Any exceptions raised by the called asynchronous methods.
        """
        # Logic to gather resources and food
        for resource in resources:
            if resource in self.resources_needed():
                await self.take(resource)

        if self.can_evolve():
            await self.drop_resources()
            await self.broadcast("Evolving to the next stage!")
            await self.gather_around()

    async def drop_resources(self):
        """
        Asynchronously drops the required resources at the current location.

        Iterates over all resources needed and performs the drop action for each by calling the `set` method.

        Returns:
            None
        """
        # Logic to drop the required resources at the current location
        for resource in self.resources_needed():
            await self.set(resource)

    async def gather_around(self):
        """
        Asynchronously gathers other AIs around the current location until the required number for an incantation is met.

        Continuously checks the number of nearby AIs using `connect_nbr()`. When the number of nearby AIs is greater than or equal to the required amount (as determined by `required_AIs_for_incantation()`), initiates the incantation process by calling `start_incantation()`.

        This coroutine runs until the incantation is started.
        """
        # Logic to gather other AIs around the current location
        while True:
            nearby_AIs = await self.connect_nbr()
            if nearby_AIs >= self.required_AIs_for_incantation():
                await self.start_incantation()
                break

    def resources_needed(self):
        """
        Returns a list of resources required for evolution.

        Returns:
            list: A list containing the names of resources needed for evolution.
        """
        # Return the resources needed for evolution
        return ["linemate", "deraumere", "sibur"]

    def can_evolve(self):
        """
        Determine if the AI can evolve based on current resources and food stock.

        Returns:
            bool: True if the AI has at least the required amount of each resource
            (as specified by `self.resources_needed()`) and has more than 0 food in
            `self.food_stock`; otherwise, False.
        """
        # Check if the AI has enough resources and food to evolve
        return (
            all(
                self.resources[res] >= self.resources_needed()[res]
                for res in self.resources_needed()
            )
            and self.food_stock > 0
        )

    async def start_incantation(self):
        """
        Asynchronously initiates the incantation process by calling the parent class's
        start_incantation method and prints a confirmation message upon starting.

        Overrides:
            The parent class's start_incantation method.

        Raises:
            Any exceptions raised by the parent class's start_incantation method.

        Returns:
            None
        """
        await super().start_incantation()  # Call the incantation command
        print("Incantation started!")
