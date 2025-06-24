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

    async def move_up(self) -> str:
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

    async def drop(self, obj: str) -> str:
        return await self._sock.send_command(
            f"Set {obj} 0"
        )  # Drop the object by setting its quantity to 0

    async def gather_resources(self):
        # Logic to gather resources from the current tile
        look_response = await self.look()
        tile_data = self.parse_look_response(look_response)
        await self.take_all_on_tile(tile_data)

    async def prepare_for_evolution(self):
        # Logic to check if the player can evolve and drop resources
        if self.can_evolve() and self.has_sufficient_food():
            await self.drop_required_resources()
            await self.broadcast("Evolving to the next stage. Gather around!")

    async def drop_required_resources(self):
        # Logic to drop the required resources for evolution
        for resource, amount in self.elevation_requirements[
            self.level
        ].items():
            if self.resources[resource] >= amount:
                await self.drop(resource)
                self.resources[resource] -= amount

    async def incantation(self):
        # Logic to initiate the incantation when enough AIs are gathered
        await self.start_incantation()
