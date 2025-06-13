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

    @classmethod
    def broadcast_receiver(cls, func):
        if cls._broadcast_receiver_callback is not None:
            raise ValueError("Cannot have 2 broadcast receive method")

        cls._broadcast_receiver_callback = func
        return func

    async def run_until_death(self):
        while True:
            print(await self.broadcast("plop"))
            print(await self.look())

        # todo: wait for all child to complete
