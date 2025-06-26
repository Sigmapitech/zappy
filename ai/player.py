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
        self.last_fork = False  # Changed from timestamp to bool
        current_dir = pathlib.Path(__file__).parent
        with open(current_dir / "elevation.json") as f:
            self.elevation_requirements = json.load(f)

    async def on_broadcast(self, direction, msg: DecodedMessage):
        # Only react if not evolving and message is for our level
        if self.evolving or self.level < 2:
            return
        # Listen for "Evolving to level X" messages and move toward the sender
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
            # If at max level, only gather food, ignore other resources
            next_level = str(self.level + 1)
            if next_level not in self.elevation_requirements:
                look_response = await self.look()
                tiles = self.handle_look_response(look_response)
                if tiles is None:
                    logger.error("Look command failed, retrying...")
                    continue
                if "food" in tiles[0]:
                    response = await self.take("food")
                    if response != "ko\n":
                        self.food_stock += 1
                await asyncio.sleep(0.5)
                continue

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
            next_level = str(self.level + 1)
            if next_level not in self.elevation_requirements:
                await asyncio.sleep(2.0)
                continue  # Already max level

            req = self.elevation_requirements[next_level]
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
                    await asyncio.sleep(1.0)
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
                    await self.drop_resources()
                    success = await self.incantation_and_wait()
                    if success:
                        print(f"[DEBUG] Evolved to level {self.level}")
                        await self.fork()  # Fork immediately after ascending
                    else:
                        print("[DEBUG] Incantation failed or timed out.")
                else:
                    # Broadcast intent to evolve and wait for others
                    await self.prepare_evolution(req)
                    self.level += 1
                    print(f"[DEBUG] Evolved to level {self.level}")
                    await self.fork()  # Fork immediately after ascending

            await asyncio.sleep(
                2.0
            )  # Still keep a small delay to avoid tight loop

    async def prepare_evolution(self, req):
        self.evolving = True
        await self.drop_resources()
        while True:
            # Broadcast intent to evolve so others can join
            await self.broadcast(f"Evolving to level {self.level + 1}")
            look_response = await self.look()
            tiles = self.handle_look_response(look_response)
            if tiles is None:
                logger.error("Look command failed, retrying...")
                continue
            if self.check_for_others(tiles, req["players"]):
                success = await self.incantation_and_wait()
                if success:
                    print(f"[DEBUG] Evolved to level {self.level}")
                else:
                    print("[DEBUG] Incantation failed or timed out.")
                break
            await asyncio.sleep(1)
        self.evolving = False

    async def drop_resources(self):
        next_level = str(self.level + 1)
        if next_level not in self.elevation_requirements:
            return
        req = self.elevation_requirements[next_level]
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

    async def incantation_and_wait(self):
        """
        Handles the incantation process:
        - Waits for 'Elevation underway'
        - Waits for 'Current level: k' or 'ko'
        Returns True if level up succeeded, False otherwise.
        """
        resp = await self.start_incantation()
        if resp is None or "ko" in resp:
            print("[DEBUG] Incantation failed immediately.")
            return False
        if "Elevation underway" not in resp:
            print(f"[DEBUG] Unexpected incantation response: {resp}")
            return False

        # Wait for the final result
        for _ in range(40):  # Wait up to 4 seconds (adjust as needed)
            await asyncio.sleep(0.1)
            # You may need to fetch messages from a queue or call a method to get the next message
            # Here we assume self._sock._next_non_message() or similar is available
            try:
                msg = await self._sock._next_non_message()
            except Exception as e:
                print(f"[DEBUG] Error waiting for incantation result: {e}")
                return False
            if "ko" in msg:
                print("[DEBUG] Incantation failed after underway.")
                return False
            if "Current level:" in msg:
                try:
                    k = int(msg.split(":")[1].strip())
                    if k > self.level:
                        self.level = k
                        print(f"[DEBUG] Evolved to level {self.level}")
                        return True
                except Exception as e:
                    print(f"[DEBUG] Failed to parse level from: {msg} ({e})")
                    return False
        print("[DEBUG] Incantation timed out waiting for result.")
        return False

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
        Expects: '[food X, linemate X, deraumere X, sibur X, mendiane X, phiras X, thystame X]'
        Returns a dictionary with resource names as keys and their counts as values.
        """
        response = response.strip("[] \n")
        if response == "ko":
            return None
        items = [trim.strip() for trim in response.split(",")]
        inventory = {}

        expected = {
            "food",
            "linemate",
            "deraumere",
            "sibur",
            "mendiane",
            "phiras",
            "thystame",
        }
        for item in items:
            parts = item.split()
            if len(parts) != 2:
                logger.error(f"Malformed inventory item: '{item}'")
                continue
            name, count = parts
            if name not in expected:
                logger.error(f"Unexpected inventory item: '{name}'")
                continue
            try:
                inventory[name] = int(count)
            except ValueError:
                logger.error(
                    f"Inventory count is not an int for item: '{item}'"
                )
                continue

        for resource in self.resources:
            if resource in inventory:
                self.resources[resource] = inventory[resource]
        self.food_stock = inventory.get("food", 0)
        return inventory

    async def run_until_death(self):  # type: ignore
        # Register broadcast handler
        self._sock._broadcast_callback = self.on_broadcast
        await asyncio.gather(self.gather_resources(), self.check_evolution())
