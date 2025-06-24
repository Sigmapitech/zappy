import os
import sys

from .async_client import AsyncSocketClient


class CommandManager:

    def __init__(self, sock: AsyncSocketClient):
        self._sock = sock

    async def look(self) -> str:
        return await self._sock.send_command("Look")

    async def take(self, obj: str) -> str:
        return await self._sock.send_command(f"Take {obj}")

    async def set(self, obj: str) -> str:
        return await self._sock.send_command(f"Set {obj}")

    async def move_forward(self) -> str:
        return await self._sock.send_command("Forward")

    async def turn_right(self) -> str:
        return await self._sock.send_command("Right")

    async def turn_left(self) -> str:
        return await self._sock.send_command("Left")

    async def start_incantation(self) -> str:
        return await self._sock.send_command("Incantation")

    async def broadcast(self, message: str):
        return await self._sock.send_command(f'Broadcast "{message}"')

    async def fork(self):
        res = await self._sock.send_command("Fork")

        if res == "ko":
            return res

        pid = os.fork()
        if pid == 0:
            os.execv(sys.executable, [sys.executable] + sys.argv)

        return res

    async def connect_nbr(self) -> str:
        return await self._sock.send_command("Connect_nbr")

    async def inventory(self) -> str:
        return await self._sock.send_command("Inventory")

    async def eject(self) -> str:
        return await self._sock.send_command("Eject")
