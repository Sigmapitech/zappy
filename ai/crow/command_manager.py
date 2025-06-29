import os
import sys

from .async_client import AsyncSocketClient


class CommandManager:

    def __init__(self, sock: AsyncSocketClient):
        self._sock = sock
        self.childs = []

    async def look(self) -> str:
        """
        Sends the "Look" command asynchronously to the server via the socket.

        Returns:
            str: The server's response to the "Look" command.
        """
        return await self._sock.send_command("Look")

    async def take(self, obj: str) -> str:
        """
        Asynchronously sends a command to take the specified object.

        Args:
            obj (str): The name of the object to take.

        Returns:
            str: The response from the server after attempting to take the object.
        """
        return await self._sock.send_command(f"Take {obj}")

    async def set(self, obj: str) -> str:
        """
        Asynchronously sends a 'Set' command with the specified object to the server.

        Args:
            obj (str): The object to be set.

        Returns:
            str: The server's response to the 'Set' command.
        """
        return await self._sock.send_command(f"Set {obj}")

    async def move_up(self) -> str:
        """
        Sends a command to move the entity forward.

        Returns:
            str: The response from the server after executing the "Forward" command.
        """
        return await self._sock.send_command("Forward")

    async def turn_right(self) -> str:
        """
        Asynchronously sends the "Right" command to the server to turn the entity to the right.

        Returns:
            str: The server's response to the "Right" command.
        """
        return await self._sock.send_command("Right")

    async def turn_left(self) -> str:
        """
        Asynchronously sends the "Left" command to the server to turn the entity left.

        Returns:
            str: The server's response to the "Left" command.
        """
        return await self._sock.send_command("Left")

    async def start_incantation(self) -> str:
        """
        Initiates an incantation by sending the "Incantation" command asynchronously.

        Returns:
            str: The response from the server after sending the incantation command.
        """
        return await self._sock.send_command("Incantation")

    async def broadcast(self, message: str):
        """
        Sends a broadcast message to all connected clients asynchronously.

        Args:
            message (str): The message to broadcast.

        Returns:
            The result of the asynchronous send_command operation.

        Raises:
            Any exceptions raised by the underlying send_command method.
        """
        return await self._sock.send_command(f'Broadcast "{message}"')

    async def fork(self):
        """
        Asynchronously sends a "Fork" command over the socket and, if successful, forks the current process.

        If the command response is "ko", returns the response immediately.
        Otherwise, forks the current process using os.fork():
            - In the child process (pid == 0), replaces the current process image with a new one using os.execv.
            - In the parent process, appends the child's PID to the 'childs' list.

        Returns:
            str: The response received from the socket command ("ko" or success indicator).
        """
        res = await self._sock.send_command("Fork")

        if res == "ko":
            return res

        pid = os.fork()
        if pid == 0:
            os.execv(sys.argv[0], sys.argv)
        else:
            self.childs.append(pid)
        return res

    async def connect_nbr(self) -> str:
        """
        Asynchronously sends the "Connect_nbr" command to the server via the socket.

        Returns:
            str: The server's response to the "Connect_nbr" command.
        """
        return await self._sock.send_command("Connect_nbr")

    async def inventory(self) -> str:
        return await self._sock.send_command("Inventory")

    async def eject(self) -> str:
        return await self._sock.send_command("Eject")

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
                await self.set(resource)
                self.resources[resource] -= amount

    async def incantation(self):
        # Logic to initiate the incantation when enough AIs are gathered
        await self.start_incantation()
