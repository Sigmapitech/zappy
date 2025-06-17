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
            msg = decode(msg, self.__secret)

            if msg is None:
                return

            logger.debug("<< %s", msg)
            return await func(direction, msg)

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

        current_dir = pathlib.Path(__file__).parent
        with open(current_dir / "elevation.json") as f:
            self.elevation_requirements = json.load(f)

    def handle_look_response(self, look_response: str) -> List[List[str]]:
        tiles = look_response.strip("[]").split(",")
        return [tile.strip().split() for tile in tiles]

    def parse_look_response(self, look_response: str) -> List[str]:
        look_response = look_response.replace("Received: ", "")
        response = look_response.strip("[]").split(", ")
        player_tile = response[0].replace("player,", "").strip()
        items = player_tile.split()
        return items

    async def take_all_on_tile(self, tile_data: List[str]):
        if tile_data:
            for item in tile_data:
                await self.take(item)

    async def handle_tile_actions(self, tiles: List[List[str]]):
        for tile in tiles:
            for obj in [
                "food",
                "linemate",
                "deraumere",
                "sibur",
                "mendiane",
                "phiras",
                "thystame",
            ]:
                if obj in tile:
                    await self.take(obj)
                    if obj == "food":
                        self.food_stock += 1
                    else:
                        self.resources[obj] += 1

    @property
    def can_reproduce(self) -> bool:
        return self.food_stock >= 1

    async def reproduce(self):
        if self.can_reproduce:
            logger.debug("Reproducing")
            await self.fork()

    def can_evolve(self) -> bool:
        if str(self.level + 1) not in self.elevation_requirements:
            return False
        requirements = self.elevation_requirements[str(self.level + 1)]
        for resource, amount in requirements.items():
            if resource == "players":
                continue  # Ignore player count for now
            if self.resources.get(resource, 0) < amount:
                return False
        return True

    async def evolve(self):
        if not self.can_evolve():
            return logger.debug("Cannot evolve yet. Insufficient resources.")

        logger.debug("Starting incantation")
        evolution_result = await self.start_incantation()

        if "Elevation underway" not in evolution_result:
            return logger.debug("Elevation did not start")

        logger.debug("Elevation underway")
        final_result = await self._sock._next_non_message()

        if "Current level" not in final_result:
            return logger.debug("Elevation failed")

        self.level += 1
        requirements = self.elevation_requirements[str(self.level)]
        for resource, amount in requirements.items():
            if (
                resource != "players"
            ):  # We don't subtract players, just resources
                self.resources[resource] -= amount
        logger.debug(f"Evolved to level {self.level}")

    async def broadcast_message(self, message: str):
        await self.broadcast(message)

    @Client.broadcast_receiver
    async def handle_incoming_messages(
        self, direction: int, message: DecodedMessage
    ):
        if message.content.startswith("level "):
            other_level = int(message.content.split(" ")[1])
            if other_level == self.level:
                pass

    async def look_for_food(self):
        look_response = await self.look()
        tiles = self.parse_look_response(look_response)
        if tiles != [] and tiles[0] == "food":
            tiles.pop(0)
        logger.debug(f"Look response: %s", tiles)
        return tiles

    def calculate_direction(self, food_tile_index):
        if food_tile_index == 1:
            return "right"
        elif food_tile_index == 2:
            return "left"
        return "up"

    async def move_to_food(self, food_tile_index):
        direction = self.calculate_direction(food_tile_index)
        if direction == "left":
            await self.turn_left()
        elif direction == "right":
            await self.turn_right()
        await self.move_up()
        new_tile = (await self.look())[0]
        await self.handle_tile_actions([new_tile])
        logger.debug(
            f"Food found in tile %s, moving %s", food_tile_index, direction
        )

    async def random_movement(self):
        if random.choice([True, False]):
            await self.turn_left()
        else:
            await self.turn_right()
        await self.move_up()
        logger.debug("No food found, turning randomly and moving up")

    async def search_food(self):
        tiles = await self.look_for_food()
        await self.take_all_on_tile(tiles)
        food_tile_index = next(
            (i for i, tile in enumerate(tiles) if "food" in tile), None
        )
        if food_tile_index is not None:
            if food_tile_index == 0:
                await self.take("food")
                self.food_stock += 1
                logger.debug("Food found and taken")
                await self.reproduce()
            else:
                await self.move_to_food(food_tile_index)
        else:
            await self.random_movement()

    async def get_inventory(self):
        inventory_response = await self.inventory()
        inventory_response = inventory_response.strip("[]").split(", ")
        inventory = {}
        for item in inventory_response:
            key, value = item.split()
            inventory[key] = int(value)
        self.food_stock = inventory.get("food", 0)
        self.resources = {
            resource: inventory.get(resource, 0)
            for resource in self.resources.keys()
        }
        logger.debug(f"Updated inventory: %s", self.resources)

    async def run_until_death(self):
        while True:
            await self.get_inventory()

            if self.can_evolve():
                await self.evolve()
            else:
                await self.search_food()

            await self.broadcast(f"level {self.level}")
