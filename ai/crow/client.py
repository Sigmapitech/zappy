from functools import partial

from ai.crow.async_client import AsyncSocketClient

from .command_manager import CommandManager


class Client(CommandManager):
    _broadcast_receiver_callback = None

    def __init__(self, host: str, port: int):
        __callback = None

        if self._broadcast_receiver_callback is not None:
            __callback = self._broadcast_receiver_callback

        socket = AsyncSocketClient(host, port, __callback or (lambda *_: _))
        super().__init__(sock=socket)

    async def connect(self, team: str):
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
        if cls._broadcast_receiver_callback is not None:
            raise ValueError("Cannot have 2 broadcast receive method")

        cls._broadcast_receiver_callback = func
        return func

    async def run_until_death(self):
        while True:
            resources = await self.look()
            await self.handle_resources(resources)
            await asyncio.sleep(1)

    async def handle_resources(self, resources):
        # Logic to gather resources and food
        for resource in resources:
            if resource in self.resources_needed():
                await self.take(resource)

        if self.can_evolve():
            await self.drop_resources()
            await self.broadcast("Evolving to the next stage!")
            await self.gather_around()

    async def drop_resources(self):
        # Logic to drop the required resources at the current location
        for resource in self.resources_needed():
            await self.set(resource)

    async def gather_around(self):
        # Logic to gather other AIs around the current location
        while True:
            nearby_AIs = await self.connect_nbr()
            if nearby_AIs >= self.required_AIs_for_incantation():
                await self.start_incantation()
                break

    def resources_needed(self):
        # Return the resources needed for evolution
        return ["linemate", "deraumere", "sibur"]

    def can_evolve(self):
        # Check if the AI has enough resources and food to evolve
        return (
            all(
                self.resources[res] >= self.resources_needed()[res]
                for res in self.resources_needed()
            )
            and self.food_stock > 0
        )

    async def start_incantation(self):
        await self.start_incantation()  # Call the incantation command
        print("Incantation started!")
