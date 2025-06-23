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
        """
        Wraps an asynchronous message receiver function to decode incoming messages and log them.

        Args:
            func (Callable[[Any, Any], Awaitable[Any]]): The asynchronous function to wrap, which takes a direction and a decoded message.

        Returns:
            Callable[[Any, Any], Awaitable[Any]]: The wrapped asynchronous function that decodes the message, logs it, and calls the original function.
        """

        async def wrapped(direction, msg):
            msg = decode(msg, self.__secret)

            if msg is None:
                return

            logger.debug("<< %s", msg)
            return await func(direction, msg)

        return wrapped

    async def connect(self, team: str):
        """
        Asynchronously connects the player to the server with the specified team name.

        If a broadcast callback is set on the socket, wraps it with a custom receiver.
        Derives and stores a secret key based on the team name.
        Calls the parent class's connect method to establish the connection and retrieve the player ID and map size.
        Stores the player ID as an integer.

        Args:
            team (str): The name of the team to connect as.

        Returns:
            Tuple[Any, Any]: A tuple containing the player ID and the map size.

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

        Increments an internal counter (wrapping at 255), encodes the message using the player's ID, the updated counter, and a secret key, then sends the encoded message using the superclass's broadcast method.

        Args:
            message (str): The message to be broadcasted.

        Returns:
            str: The result of the superclass's broadcast method after sending the encoded message.
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
        Initializes the player instance with default attributes.

        Args:
            *args: Variable length argument list for the superclass initializer.
            **kwargs: Arbitrary keyword arguments for the superclass initializer.

        Attributes:
            level (int): The current level of the player, initialized to 1.
            food_stock (int): The amount of food the player has, initialized to 0.
            resources (dict): A dictionary containing the quantities of various resources.
            memory (list): A list to store memory or history for the player.
            elevation_requirements (dict): Requirements for elevation loaded from 'elevation.json'.

        Raises:
            FileNotFoundError: If 'elevation.json' does not exist in the current directory.
            json.JSONDecodeError: If 'elevation.json' contains invalid JSON.
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

        current_dir = pathlib.Path(__file__).parent
        with open(current_dir / "elevation.json") as f:
            self.elevation_requirements = json.load(f)

    def handle_look_response(self, look_response: str) -> List[List[str]]:
        """
        Parses the response from a 'look' command and converts it into a 2D list of tile contents.

        Args:
            look_response (str): The raw string response from the 'look' command, typically formatted as a comma-separated list of tiles within square brackets.

        Returns:
            List[List[str]]: A list of lists, where each inner list contains the items present on a tile.
        """
        tiles = look_response.strip("[]").split(",")
        return [tile.strip().split() for tile in tiles]

    def parse_look_response(self, look_response: str) -> List[str]:
        """
        Parses the response string from a 'look' command and extracts the items present on the player's current tile.

        Args:
            look_response (str): The raw response string received from the 'look' command, expected to be in the format "[player, item1, item2, ...]".

        Returns:
            List[str]: A list of item names (as strings) found on the player's current tile, excluding the 'player' keyword.
        """
        look_response = look_response.replace("Received: ", "")
        response = look_response.strip("[]").split(", ")
        player_tile = response[0].replace("player,", "").strip()
        items = player_tile.split()
        return items

    async def take_all_on_tile(self, tile_data: List[str]):
        """
        Asynchronously takes all items present on the current tile.

        Args:
            tile_data (List[str]): A list of item names present on the tile.

        Returns:
            None

        Notes:
            For each item in tile_data, this method calls the asynchronous `take` method to pick up the item.
        """
        if tile_data:
            for item in tile_data:
                await self.take(item)

    async def handle_tile_actions(self, tiles: List[List[str]]):
        """
        Asynchronously processes a list of tiles, collecting resources found on each tile.

        For each tile in the provided list, checks for the presence of specific objects
        ("food", "linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame").
        If an object is found, attempts to take it by calling `self.take(obj)`.
        Updates the player's inventory: increments `self.food_stock` if the object is "food",
        or increments the corresponding count in `self.resources` for other objects.

        Args:
            tiles (List[List[str]]): A list of tiles, where each tile is a list of object names (strings) present on that tile.

        Returns:
            None
        """
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
        """
        Determines if the player has enough food stock to reproduce.

        Returns:
            bool: True if the player's food stock is greater than or equal to 1, False otherwise.
        """
        return self.food_stock >= 1

    async def reproduce(self):
        """
        Asynchronously attempts to reproduce the player if the conditions allow.

        If the player is able to reproduce (`can_reproduce` is True), logs the action and initiates the forking process.

        Returns:
            None
        """
        if self.can_reproduce:
            logger.debug("Reproducing")
            await self.fork()

    def can_evolve(self) -> bool:
        """
        Determine if the player has sufficient resources to evolve to the next level.

        Checks if the requirements for the next level exist and verifies that the player
        has at least the required amount of each resource (excluding player count).
        Returns True if all resource requirements are met, otherwise False.

        Returns:
            bool: True if the player can evolve to the next level, False otherwise.
        """
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
        """
        Attempts to evolve the player to the next level by performing an incantation.

        This asynchronous method checks if the player has sufficient resources to evolve.
        If so, it initiates the incantation process and waits for confirmation that the elevation
        is underway. Upon successful elevation, the player's level is incremented and the required
        resources are deducted from the player's inventory.

        Returns:
            None

        Logs:
            - Debug messages at each step of the evolution process, including failures and successes.
        """
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
        """
        Asynchronously broadcasts a message to all connected clients.

        Args:
            message (str): The message to be broadcasted.
        """
        await self.broadcast(message)

    @Client.broadcast_receiver
    async def handle_incoming_messages(
        self, direction: int, message: DecodedMessage
    ):
        """
        Handles incoming messages from a specified direction and processes them based on their content.

        Args:
            direction (int): The direction from which the message was received.
            message (DecodedMessage): The decoded message object containing the message content.

        Behavior:
            - If the message content starts with "level ", extracts the level value.
            - If the extracted level matches the player's current level, performs no action.
        """
        if message.content.startswith("level "):
            other_level = int(message.content.split(" ")[1])
            if other_level == self.level:
                pass

    async def look_for_food(self):
        """
        Asynchronously scans the surrounding area for food and returns a list of items found.

        This method performs a look action, parses the response to identify items on the tiles,
        and removes the first item if it is "food". The resulting list of items is then returned.

        Returns:
            list: A list of items found on the tiles after removing the first "food" item, if present.
        """
        look_response = await self.look()
        tiles = self.parse_look_response(look_response)
        if tiles != [] and tiles[0] == "food":
            tiles.pop(0)
        logger.debug(f"Look response: %s", tiles)
        return tiles

    def calculate_direction(self, food_tile_index):
        """
        Determines the direction to move based on the provided food tile index.

        Args:
            food_tile_index (int): The index representing the position of the food tile.

        Returns:
            str: The direction to move ("right", "left", or "up") based on the food tile index.
        """
        if food_tile_index == 1:
            return "right"
        elif food_tile_index == 2:
            return "left"
        return "up"

    async def move_to_food(self, food_tile_index):
        """
        Asynchronously moves the player towards the specified food tile.

        Determines the direction to turn based on the food tile index, performs the turn,
        moves the player forward, inspects the new tile, and handles any actions required
        on that tile. Logs the movement and direction taken.

        Args:
            food_tile_index (int): The index of the tile containing food.

        Returns:
            None
        """
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
        """
        Performs a random movement by turning either left or right, then moving up.

        This asynchronous method randomly chooses to turn the player left or right,
        executes the turn, then moves the player up. It logs a debug message indicating
        that no food was found and a random movement was performed.
        """
        if random.choice([True, False]):
            await self.turn_left()
        else:
            await self.turn_right()
        await self.move_up()
        logger.debug("No food found, turning randomly and moving up")

    async def search_food(self):
        """
        Asynchronously searches for food in the surrounding tiles and takes appropriate actions.

        This method performs the following steps:
        1. Looks for food in the nearby tiles.
        2. Attempts to take all items on the found tiles.
        3. If food is found:
            - If food is on the current tile, takes the food, increments food stock, logs the action, and attempts to reproduce.
            - If food is on another tile, moves towards the food.
        4. If no food is found, performs a random movement.

        Returns:
            None
        """
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
        """
        Asynchronously retrieves and parses the player's inventory from the server.

        The method calls the `inventory` coroutine to obtain the inventory response,
        parses the response string to extract item names and their quantities, and updates
        the player's `food_stock` and `resources` attributes accordingly.

        Updates:
            - self.food_stock: Sets to the amount of "food" in the inventory, or 0 if not present.
            - self.resources: Updates each resource's quantity based on the parsed inventory.

        Logs the updated resources at the debug level.
        """
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
        """
        Continuously executes player actions in a loop until termination.

        The method performs the following steps in each iteration:
            1. Retrieves the player's current inventory asynchronously.
            2. Checks if the player can evolve:
                - If evolution is possible, performs the evolution action.
                - Otherwise, searches for food.
            3. Broadcasts the player's current level.

        This loop is intended to run indefinitely, simulating the player's lifecycle.
        """
        while True:
            await self.get_inventory()

            if self.can_evolve():
                await self.evolve()
            else:
                await self.search_food()

            await self.broadcast(f"level {self.level}")
