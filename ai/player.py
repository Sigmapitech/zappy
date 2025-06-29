import json
import logging
import pathlib
import random
from typing import List, Final

from .cipher import DecodedMessage, decode, derive_team_key, encode
from .crow import Client

CRITICAL_FOOD_THOLD: Final[int] = 3
MAX_FLOCK_PLAYERS: Final[int] = 15
RAVEN_LVL_THOLD: Final[int] = 3

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

    async def broadcast(self, message: str, to_encrypt: bool) -> str:
        self.counter += 1
        self.counter %= 255

        if not to_encrypt:
            return await super().broadcast(message)
        
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
            "linemate": 0, "deraumere": 0, "sibur": 0,
            "mendiane": 0, "phiras": 0, "thystame": 0
        }

        self.last_broadcast_direction = None
        self.last_broadcast_message = None
        self.known_players: set[tuple[int, str]] = set() # { (id: job_name), ... }
        self.role = "flocker"

        current_dir = pathlib.Path(__file__).parent
        with open(current_dir / "elevation.json") as f:
            self.elevation_requirements = json.load(f)

    def elect_leader(self):
        if any(role == "leader" for _, role in self.known_players):
            self.role = "flocker"
        if not self.known_players or self.id_ < min(self.known_players):
            self.role = "leader"

    def handle_look_response(self, look_response: str) -> List[List[str]]:
        tiles = look_response.strip("[]").split(",")
        return [tile.strip().split() for tile in tiles]

    def parse_look_response(self, look_response: str) -> List[str]:
        look_response = look_response.replace("Received: ", "")
        response = look_response.strip("[]").split(", ")
        player_tile = response[0].replace("player,", "").strip()
        return player_tile.split()

    async def take_all_on_tile(self, tile_data: List[str]):
        for item in tile_data:
            await self.take(item)

    async def handle_tile_actions(self, tiles: List[List[str]]):
        for tile in tiles:
            for obj in self.resources.keys() | {"food"}:
                if obj in tile:
                    await self.take(obj)
                    if obj == "food":
                        self.food_stock += 1
                    else:
                        self.resources[obj] += 1

    @property
    def can_reproduce(self) -> bool:
        return self.food_stock >= 5

    async def reproduce(self):
        if self.can_reproduce:
            await self.fork()

    async def get_current_tile_amount(self, key: str) -> int:
        look_response = await self.look()
        tiles = self.handle_look_response(look_response)
        return tiles[0].count(key)

    async def can_evolve(self) -> bool:
        level_str = str(self.level + 1)
        if level_str not in self.elevation_requirements:
            return False

        reqs = self.elevation_requirements[level_str]
        look_response = await self.look()
        tiles = look_response.strip("[]").split(",")
        player_count = tiles[0].count("player")

        for resource, amount in reqs.items():
            if (resource == "players" and player_count < amount) or (self.resources.get(resource, 0) < amount):
                return False
        return True

    async def evolve(self):
        if not await self.can_evolve(): return

        result = await self.start_incantation()
        if "Elevation underway" not in result: return

        final = await self._sock._next_non_message()
        if "Current level" not in final: return

        self.level += 1
        reqs = self.elevation_requirements[str(self.level)]

        for r, amt in reqs.items():
            if r != "players":
                self.resources[r] -= amt

    def tiles_count(self, level: int) -> int:
        return (level + 1)**2

    def tile_index_to_direction(self, idx: int) -> str:
        level = self.level
        i = 0
        for d in range(1, level + 1):
            width = 2 * d + 1
            for x in range(width):
                if i == idx:
                    rel_x = x - d
                    if rel_x == 0: return "forward"
                    elif rel_x < 0: return "left"
                    else: return "right"
                i += 1
        return None

    async def move_toward_direction(self, direction: str):
        if direction == "left":
            await self.turn_left()
            await self.move_forward()
        elif direction == "right":
            await self.turn_right()
            await self.move_forward()
        elif direction == "forward":
            await self.move_forward()

    async def search_food(self):
        tiles = self.handle_look_response(await self.look())
        for idx, tile in enumerate(tiles):
            if "food" in tile:
                if idx == 0:
                    await self.take("food")
                    self.food_stock += 1
                    await self.reproduce()
                else:
                    direction = self.tile_index_to_direction(idx)
                    await self.move_toward_direction(direction)
                return
        await self.random_movement()

    async def random_movement(self):
        if random.choice([True,False]):
            await self.turn_left()
        else:
            await self.turn_right()
        await self.move_forward()

    @Client.broadcast_receiver
    async def handle_incoming_messages(self, direction: int, message: DecodedMessage):
        self.known_players.add(message.bot_id)

        if len(self.known_players) > MAX_FLOCK_PLAYERS and self.level < RAVEN_LVL_THOLD:
            screech = random.choice([
                "\ufeff", "\x00", "\xff",
                "\x7f", "\x1b", "\x08",   # uncrypted garbage characters
                "�", "♠", "‼", "¿"
            ])
            await self.broadcast(screech, to_encrypt=False)
            await self.random_movement()
            return
        
        if message.content.startswith("flock"):
            self.last_broadcast_direction = direction
            self.last_broadcast_message = message.content
            parts = message.content.split(":")
            sender_id = message.bot_id
            sender_role = parts[2] if len(parts) > 2 else "flocker"
            self.known_players.add((sender_id, sender_role))

    async def broadcast_position(self):
        await self.broadcast(f"flock:{self.id_}", to_encrypt=True)

    async def get_inventory(self):
        inv = (await self.inventory()).strip("[]").split(", ")
        parsed = dict(item.split() for item in inv if item)
        self.food_stock = int(parsed.get("food", 0))
        for k in self.resources:
            self.resources[k] = int(parsed.get(k, 0))

    async def flock_move(self):
        if self.last_broadcast_direction is None:
            await self.random_movement()
            return

        direction_map = {
            0: None,
            1: "forward",
            2: "forward-right",
            3: "right",
            4: "back-right",
            5: "back",
            6: "back-left",
            7: "left",
            8: "forward-left"
        }

        move_dir = direction_map.get(self.last_broadcast_direction)
        if move_dir == "forward":
            await self.move_forward()
        elif move_dir == "right":
            await self.turn_right()
            await self.move_forward()
        elif move_dir == "left":
            await self.turn_left()
            await self.move_forward()
        elif move_dir == "back":
            await self.turn_right()
            await self.turn_right()
            await self.move_forward()
        else:
            await self.random_movement()

    async def raven_move(self):
        screech = random.choice(
            ["\xfeff", "\x00\x00", "\xff", "\x7f", "\x1b", "\x08", "¿", "�", "♠"]
        ) # unencrypted noise
        await self.broadcast(screech, to_encrypt=False)

        if self.last_broadcast_direction is not None:
            opposite_direction = (self.last_broadcast_direction + 4) % 8 or 8
            direction_map = {
                1: "forward", 2: "forward-right", 3: "right", 4: "back-right",
                5: "back", 6: "back-left", 7: "left", 8: "forward-left"
            }
            move_dir = direction_map.get(opposite_direction)
            if move_dir == "forward": await self.move_forward()
            elif move_dir == "right":
                await self.turn_right()
                await self.move_forward()
            elif move_dir == "left":
                await self.turn_left()
                await self.move_forward()
            elif move_dir == "back":
                await self.turn_right()
                await self.turn_right()
                await self.move_forward()
            else:
                await self.random_movement()
        else:
            await self.random_movement()

    async def run_until_death(self):
        self.elect_leader()

        while True:
            await self.get_inventory()
            if self.food_stock < CRITICAL_FOOD_THOLD:
                await self.search_food()
                continue

            if self.role == "garbler":
                await self.garble_move()
            elif self.role == "leader":
                await self.broadcast_position()
            else:
                await self.flock_move()

            await self.evolve()
            await self.reproduce()
