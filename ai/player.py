import asyncio
import json
import logging
import pathlib
import random
from typing import List

from .cipher import DecodedMessage, decode, derive_team_key, encode
from .crow import Client

logger = logging.getLogger(__name__)


class SecretivePlayer(Client):
    counter = 0

    def __wrap_receiver(self, func):
        async def wrapped(direction, msg):
            decoded_msg = decode(msg, self.__secret)
            if decoded_msg:
                await func(direction, decoded_msg)

        return wrapped

    async def connect(self, team: str):
        if self._sock._broadcast_callback is not None:
            self._sock._broadcast_callback = self.__wrap_receiver(
                self._sock._broadcast_callback
            )

        self.__secret = derive_team_key(team)
        id_, map_size = await super().connect(team)
        self.id_ = int(id_)
        return id_, map_size

    async def broadcast(self, message: str) -> str:
        self.counter += 1
        self.counter %= 255
        ciphered = encode(
            self.id_, self.counter, message.encode("ascii"), self.__secret
        )
        return await super().broadcast(ciphered)


class Player(SecretivePlayer):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.level = 1
        self.food_stock = 0
        self.resources = {
            "linemate": 0,
            "deraumere": 0,
            "sibur": 0,
            "mendiane": 0,
            "phiras": 0,
            "thystame": 0,
        }
        self.memory = []
        self.broadcasting = False
        self.evolving = False
        self.last_fork = 0
        current_dir = pathlib.Path(__file__).parent
        with open(current_dir / "elevation.json") as f:
            self.elevation_requirements = json.load(f)

    async def on_broadcast(self, direction, msg: DecodedMessage):
        # Only react if not evolving and message is for our level
        if self.evolving or self.level < 2:
            return
        if f"Evolving to level {self.level + 1}" in msg.content:
            await self.move_towards(direction)

    async def move_towards(self, direction: int):
        # Simple logic: turn and move toward the direction
        if direction == 0:
            return  # Already on the tile
        # For simplicity, just move forward or turn randomly
        await self.move_up()

    async def gather_resources(self):
        while True:
            look_response = await self.look()
            tiles = self.handle_look_response(look_response)
            await self.handle_tile_actions(tiles)
            await asyncio.sleep(0.5)

    async def handle_tile_actions(self, tiles: List[List[str]]):
        # Take everything on current tile (tiles[0])
        await self.take_all_on_tile(tiles[0])
        # If nothing to take, move randomly
        if not any(
            item in self.resources or item == "food" for item in tiles[0]
        ):
            await random.choice(
                [self.move_up, self.turn_left, self.turn_right]
            )()

    async def take_all_on_tile(self, tile_data: List[str]):
        for item in tile_data:
            if item in self.resources:
                await self.take(item)
                self.resources[item] += 1
            elif item == "food":
                await self.take(item)
                self.food_stock += 1

    async def check_evolution(self):
        while True:
            await asyncio.sleep(0.5)
            # Fork every 30 seconds if possible
            if asyncio.get_event_loop().time() - self.last_fork > 30:
                await self.fork()
                self.last_fork = asyncio.get_event_loop().time()
            # Check if ready to evolve
            req = self.elevation_requirements[str(self.level)]
            if self.food_stock >= 10 and all(
                self.resources.get(res, 0) >= req.get(res, 0)
                for res in self.resources
            ):
                if self.level == 1:
                    # Level 1->2: evolve immediately, no broadcast
                    await self.drop_resources()
                    await self.incantation()
                    self.level += 1
                else:
                    # Level >=2: broadcast and wait for others
                    await self.prepare_evolution(req)
                    self.level += 1

    async def prepare_evolution(self, req):
        self.evolving = True
        await self.drop_resources()
        # Only broadcast if level >= 2 (i.e., after first ascension)
        while True:
            await self.broadcast(f"Evolving to level {self.level + 1}")
            look_response = await self.look()
            tiles = self.handle_look_response(look_response)
            if self.check_for_others(tiles, req["players"]):
                await self.incantation()
                break
            await asyncio.sleep(1)
        self.evolving = False

    async def drop_resources(self):
        req = self.elevation_requirements[str(self.level)]
        for resource, amount in req.items():
            if resource == "players":
                continue
            for _ in range(amount):
                if self.resources[resource] > 0:
                    await self.set(resource)
                    self.resources[resource] -= 1

    def check_for_others(
        self, tiles: List[List[str]], needed_players: int
    ) -> bool:
        # Count players on current tile (tiles[0])
        return tiles[0].count("player") >= needed_players

    async def incantation(self):
        await self.start_incantation()

    def handle_look_response(self, response: str) -> List[List[str]]:
        # Parse look response: [player, food, ...]
        response = response.strip("[] \n")
        tiles = [
            tile.strip().split() if tile.strip() else []
            for tile in response.split(",")
        ]
        return tiles

    async def run_until_death(self):  # type: ignore
        # Register broadcast handler
        self._sock._broadcast_callback = self.on_broadcast
        await asyncio.gather(self.gather_resources(), self.check_evolution())
