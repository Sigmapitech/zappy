import asyncio
import json
import logging
import pathlib
import random
import string
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
        self._bullshit_task = None

    async def on_broadcast(self, direction, msg):
        # If we can't decode, sometimes rebroadcast the raw message (unencrypted)
        if not isinstance(msg, DecodedMessage):
            if random.random() < 0.3:  # 30% chance to rebroadcast
                await self.broadcast(msg)
            return
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
            if tiles is None:
                logger.error("Look command failed, retrying...")
                continue
            await self.handle_tile_actions(tiles)
            await asyncio.sleep(0.5)

    async def handle_tile_actions(self, tiles: List[List[str]]):
        # Prioritize taking resources needed for next level
        next_level = str(self.level + 1)
        needed = set()
        if next_level in self.elevation_requirements:
            req = self.elevation_requirements[next_level]
            needed = {
                res
                for res, amount in req.items()
                if res != "players" and self.resources.get(res, 0) < amount
            }

        # Take needed resources first
        for item in tiles[0]:
            if item in needed:
                response = await self.take(item)
                if response == "ko\n":
                    continue
                self.resources[item] += 1
                return  # Only take one per tick for realism

        # Then take food if present
        if "food" in tiles[0]:
            response = await self.take("food")
            if response == "ko\n":
                return
            self.food_stock += 1
            return

        # Then take any other resource
        for item in tiles[0]:
            if item in self.resources:
                response = await self.take(item)
                if response == "ko\n":
                    continue
                self.resources[item] += 1
                return

        # If nothing to take, move randomly
        await random.choice([self.move_up, self.turn_left, self.turn_right])()

    async def take_all_on_tile(self, tile_data: List[str]):
        for item in tile_data:
            if item in self.resources:
                response = await self.take(item)
                if response == "ko\n":
                    continue
                self.resources[item] += 1
            elif item == "food":
                response = await self.take(item)
                if response == "ko\n":
                    continue
                self.food_stock += 1

    async def check_evolution(self):
        while True:
            await asyncio.sleep(0.1)  # Try to evolve more often
            # Fork every 30 seconds if possible
            if asyncio.get_event_loop().time() - self.last_fork > 30:
                await self.fork()
                self.last_fork = asyncio.get_event_loop().time()
            # Check if ready to evolve
            next_level = str(self.level + 1)
            if next_level not in self.elevation_requirements:
                continue  # Already max level
            req = self.elevation_requirements[next_level]
            # Only check for resources actually required for next level
            has_all = all(
                self.resources.get(res, 0) >= amount
                for res, amount in req.items()
                if res != "players"
            )
            if has_all:
                # Sync stored inventory with the server
                if (
                    self.handle_inventory_response(await self.inventory())
                    is None
                ):
                    logger.error("Inventory command failed, retrying...")
                    continue

            has_all = all(
                self.resources.get(res, 0) >= amount
                for res, amount in req.items()
                if res != "players"
            )
            if self.food_stock >= 4 and has_all:
                print(
                    f"[DEBUG] Ready to evolve to level {next_level} (current: {self.level})"
                )
                print(
                    f"[DEBUG] Inventory: {self.resources}, Food: {self.food_stock}"
                )
                if req["players"] == 1:
                    # Level 1->2: evolve immediately, no broadcast
                    await self.drop_resources(req)
                    await self.incantation()
                    self.level += 1
                    print(f"[DEBUG] Evolved to level {self.level}")
                else:
                    # Level >=2: broadcast and wait for others
                    await self.prepare_evolution(req)
                    self.level += 1
                    print(f"[DEBUG] Evolved to level {self.level}")

    async def prepare_evolution(self, req):
        self.evolving = True
        await self.drop_resources(req)
        while True:
            await self.broadcast(f"Evolving to level {self.level + 1}")
            look_response = await self.look()
            tiles = self.handle_look_response(look_response)
            if tiles is None:
                logger.error("Look command failed, retrying...")
                continue
            if self.check_for_others(tiles, req["players"]):
                await self.incantation()
                break
            await asyncio.sleep(1)
        self.evolving = False

    async def drop_resources(self, req):
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

    def handle_look_response(self, response: str) -> List[List[str]] | None:
        """
        Parses the response from the Look command.
        Returns a list of lists: each sublist contains the objects on a tile.
        Example input: '[player, player deraumere,,food]'
        Output: [['player'], ['player', 'deraumere'], [], ['food']]
        """
        # Remove brackets and whitespace
        response = response.strip("[] \n")
        if response == "ko":
            return None
        # Split by comma (with or without following space)
        tiles = [tile.strip() for tile in response.split(",")]
        # Split each tile by spaces to get objects
        return [tile.split() if tile else [] for tile in tiles]

    def handle_inventory_response(
        self, response: str
    ) -> dict[str, int] | None:
        """
        Parses the response from the Inventory command.
        Returns a dictionary with resource names as keys and their counts as values.
        Example input: '[linemate 1, deraumere 2, food 3]'
        Output: {'linemate': 1, 'deraumere': 2, 'food': 3}
        """
        response = response.strip("[] \n")
        if response == "ko":
            return None
        items = [trim.strip() for trim in response.split(",")]
        inventory = {}

        for item in items:
            name, count = item.split(" ")
            inventory[name] = int(count)

        for resource in self.resources:
            if resource in inventory:
                self.resources[resource] = inventory[resource]
        self.food_stock = inventory.get("food", 0)
        return inventory

    async def _bullshit_broadcast_loop(self):
        while True:
            # Send random/bullshit message every 8-20 seconds
            await asyncio.sleep(random.uniform(8, 20))
            msg = self._generate_bullshit_message()
            await self.broadcast(msg)

    def _generate_bullshit_message(self):
        # You can make this more elaborate if you want
        options = [
            "Help! Need linemate!",
            "Anyone got food?",
            "Incantation at tile 3!",
            "Looking for deraumere.",
            "Random: "
            + "".join(
                random.choices(string.ascii_letters + string.digits, k=8)
            ),
            "Meet at spawn.",
            "Evolving soon!",
            "Watch out for enemy!",
            "Food at tile 5.",
            "Bullshit: " + "".join(random.choices(string.ascii_letters, k=12)),
        ]
        return random.choice(options)

    async def run_until_death(self):  # type: ignore
        # Register broadcast handler
        self._sock._broadcast_callback = self.on_broadcast
        # Start bullshit broadcast task
        self._bullshit_task = asyncio.create_task(
            self._bullshit_broadcast_loop()
        )
        await asyncio.gather(self.gather_resources(), self.check_evolution())
