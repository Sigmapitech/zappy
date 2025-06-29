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
        """
        Wraps a receiver function to automatically decode incoming messages before passing them to the original function.

        Args:
            func (Callable[[Any, Any], Awaitable]): The asynchronous function to wrap, which takes a direction and a decoded message.

        Returns:
            Callable[[Any, Any], Awaitable]: An asynchronous wrapper function that decodes the message using the instance's secret before invoking the original function. If decoding fails, the original function is not called.
        """

        async def wrapped(direction, msg):
            decoded_msg = decode(msg, self.__secret)
            if decoded_msg:
                await func(direction, decoded_msg)

        return wrapped

    async def connect(self, team: str):
        """
        Asynchronously connects the player to the server with the specified team name.

        If a broadcast callback is set on the socket, wraps it with a custom receiver.
        Derives a secret key for the team and stores it.
        Calls the parent class's connect method to establish the connection, retrieves the player ID and map size,
        and stores the player ID as an integer.

        Args:
            team (str): The name of the team to connect as.

        Returns:
            Tuple[int, Any]: A tuple containing the player ID and the map size.

        Raises:
            Any exceptions raised by the parent class's connect method.
        """
        if self._sock._broadcast_callback is not None:
            self._sock._broadcast_callback = self.__wrap_receiver(
                self._sock._broadcast_callback
            )

        self.__secret = derive_team_key(team)
        id_, map_size = await super().connect(team)
        self.id_ = int(id_)
        return id_, map_size

    async def broadcast(self, message: str) -> str:
        """
        Broadcasts a message after encoding it with a cipher.

        Increments an internal counter, encodes the message using the player's ID,
        the updated counter, and a secret key, then broadcasts the encoded message
        using the superclass's broadcast method.

        Args:
            message (str): The message to be broadcasted.

        Returns:
            str: The result of the superclass's broadcast method with the encoded message.
        """
        self.counter += 1
        self.counter %= 255
        ciphered = encode(
            self.id_, self.counter, message.encode("ascii"), self.__secret
        )
        return await super().broadcast(ciphered)


class Player(SecretivePlayer):
    def __init__(self, *args, **kwargs):
        """
        Initializes a new player instance with default attributes.

        Args:
            *args: Variable length argument list for the superclass.
            **kwargs: Arbitrary keyword arguments for the superclass.

        Attributes:
            level (int): The current level of the player, initialized to 1.
            food_stock (int): The amount of food the player has, initialized to 0.
            resources (dict): A dictionary tracking the quantity of each resource type.
            memory (list): A list used for storing memory or history.
            broadcasting (bool): Indicates if the player is currently broadcasting.
            evolving (bool): Indicates if the player is currently evolving.
            last_fork (int): Timestamp or counter for the last fork action.
            elevation_requirements (dict): Requirements for elevation loaded from 'elevation.json'.
            _bullshit_task: Placeholder for an internal task or coroutine.
        """
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
        """
        Handle incoming broadcast messages.

        If the message is not a DecodedMessage, there is a 30% chance to rebroadcast it as-is.
        If the player is currently evolving or below level 2, the message is ignored.
        If the message indicates an evolution to the next level, the player moves towards the broadcast direction.

        Args:
            direction (int): The direction from which the broadcast was received.
            msg (Any): The broadcast message, which may or may not be a DecodedMessage.

        Returns:
            None
        """
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
        """
        Asynchronously moves the player towards a specified direction.

        Args:
            direction (int): The direction to move towards. If 0, the player is already on the target tile and no movement occurs.

        Returns:
            None

        Notes:
            - If the direction is not 0, the player will move up as a simple movement logic.
            - This method can be extended to include more complex movement or turning logic.
        """
        # Simple logic: turn and move toward the direction
        if direction == 0:
            return  # Already on the tile
        # For simplicity, just move forward or turn randomly
        await self.move_up()

    async def gather_resources(self):
        """
        Continuously gathers resources by repeatedly scanning the environment and acting on visible tiles.

        This asynchronous method performs the following steps in a loop:
            1. Calls the `look` method to observe the surroundings.
            2. Processes the response using `handle_look_response`.
            3. If the look command fails, logs an error and retries.
            4. Executes actions on the observed tiles using `handle_tile_actions`.
            5. Waits for 0.5 seconds before repeating the process.

        This method is intended to run indefinitely as part of the player's resource-gathering behavior.
        """
        while True:
            look_response = await self.look()
            tiles = self.handle_look_response(look_response)
            if tiles is None:
                logger.error("Look command failed, retrying...")
                continue
            await self.handle_tile_actions(tiles)
            await asyncio.sleep(0.5)

    async def handle_tile_actions(self, tiles: List[List[str]]):
        """
        Handles actions to perform on the current tile based on its contents.

        The method prioritizes actions in the following order:
        1. Takes resources required for the next elevation level if available and needed.
        2. Takes food if present on the tile.
        3. With a 20% chance, ejects other players if there are multiple players on the tile.
        4. Takes any other available resource present on the tile.
        5. If no action is taken, performs a random movement action.

        Args:
            tiles (List[List[str]]): A list of lists representing the items on the visible tiles,
                                    where tiles[0] corresponds to the current tile.

        Returns:
            None
        """
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

        # If there are other players on the tile, sometimes eject them
        if tiles[0].count("player") > 1 and random.random() < 0.2:
            await self.eject()  # 20% chance to eject if not alone

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
        """
        Asynchronously attempts to take all items present on the current tile.

        For each item in the provided tile_data list:
            - If the item is a resource tracked in self.resources, attempts to take it and increments the resource count if successful.
            - If the item is "food", attempts to take it and increments the food stock if successful.
            - If the take action fails (response is "ko\n"), the item is skipped.

        Args:
            tile_data (List[str]): A list of item names present on the tile.

        Returns:
            None
        """
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
        """
        Periodically checks and manages the player's evolution process.

        This asynchronous method performs the following tasks in a loop:
        - Sleeps briefly between iterations to allow frequent evolution checks.
        - Attempts to fork a new process every 30 seconds if possible.
        - Determines if the player has met the requirements to evolve to the next level.
        - Synchronizes the player's inventory with the server before evolving.
        - If requirements are met and sufficient food is available:
            - For level 1 to 2, evolves immediately without broadcasting.
            - For higher levels, prepares for evolution and broadcasts to other players.
        - Increments the player's level upon successful evolution.

        The method relies on the player's current resources, food stock, and evolution requirements.
        """
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
        """
        Prepares the player for evolution (level up) by dropping required resources and coordinating with other players.

        This asynchronous method sets the player's evolving state to True, drops the necessary resources for evolution,
        and repeatedly broadcasts the intention to evolve. It checks the surrounding tiles for the presence of the required
        number of players. If the required players are present, it initiates the incantation process to evolve. The method
        continues to retry if the look command fails or if the required players are not present, waiting 1 second between retries.
        Once the evolution process is complete or aborted, the evolving state is set to False.

        Args:
            req (dict): A dictionary containing the requirements for evolution, including the number of players needed.

        Returns:
            None
        """
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
        """
        Asynchronously drops specified resources from the player's inventory.

        Iterates over the provided resource requirements and, for each resource (excluding "players"),
        removes the specified amount from the player's resources by calling the `set` method and
        decrementing the resource count.

        Args:
            req (dict): A dictionary mapping resource names (str) to the amount (int) to drop.
                        The key "players" is ignored.

        Returns:
            None

        Note:
            This method assumes that `self.resources` is a dictionary containing the player's current
            resources and that `self.set(resource)` is an asynchronous method responsible for handling
            the resource drop logic.
        """
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
        """
        Checks if there are at least a specified number of players on the current tile.

        Args:
            tiles (List[List[str]]): A list of tiles, where each tile is represented as a list of strings.
                                    The first tile (tiles[0]) is considered the current tile.
            needed_players (int): The minimum number of players required on the current tile.

        Returns:
            bool: True if the number of players on the current tile is greater than or equal to needed_players, False otherwise.
        """
        # Count players on current tile (tiles[0])
        return tiles[0].count("player") >= needed_players

    async def incantation(self):
        """
        Initiates the incantation process asynchronously.

        This method starts the incantation by awaiting the `start_incantation` coroutine.
        """
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
        """
        Periodically broadcasts a malicious message at random intervals between 10 and 30 seconds.

        This asynchronous loop is intended to send out messages generated by
        `_generate_malicious_message()` using the `broadcast` method. The random
        interval between broadcasts helps to reduce predictability and increase
        survivability of the broadcasting entity.

        Returns:
            None
        """
        while True:
            # Broadcast every 10-30 seconds, less frequent for survivability
            await asyncio.sleep(random.uniform(10, 30))
            msg = self._generate_malicious_message()
            await self.broadcast(msg)

    def _generate_malicious_message(self):
        """
        Generates and returns a randomly selected malicious or misleading message.

        The generated message may impersonate other teams, admins, or provide false information
        intended to deceive other players. The message content is randomly chosen from a set of
        predefined misleading statements, including fake alerts, impersonations, and random noise.

        Returns:
            str: A randomly selected malicious or misleading message.
        """
        # Add misleading, fake, or impersonating messages
        fake_teams = ["RedTeam", "BlueTeam", "GRAPHIC", "Admin"]
        options = [
            "Incantation at tile 5! Hurry!",
            "Food at tile 2, come quick!",
            "Enemy spotted at tile 7!",
            "RedTeam: Need help at spawn!",
            "GRAPHIC: Server will restart soon.",
            "BlueTeam: Evolving at tile 3.",
            "Admin: Please disconnect for update.",
            "Random: "
            + "".join(
                random.choices(string.ascii_letters + string.digits, k=8)
            ),
            "Bullshit: " + "".join(random.choices(string.ascii_letters, k=12)),
            f"{random.choice(fake_teams)}: All gather at tile {random.randint(1, 8)}!",
        ]
        return random.choice(options)

    async def run_until_death(self):  # type: ignore
        """
        Asynchronously runs the main player loop until termination.

        This method registers the broadcast handler, starts a background task for broadcasting,
        and concurrently gathers resources and checks for evolution conditions. The method
        completes when either resource gathering or evolution checking finishes.

        Returns:
            None
        """
        # Register broadcast handler
        self._sock._broadcast_callback = self.on_broadcast
        # Start bullshit broadcast task
        self._bullshit_task = asyncio.create_task(
            self._bullshit_broadcast_loop()
        )
        await asyncio.gather(self.gather_resources(), self.check_evolution())
