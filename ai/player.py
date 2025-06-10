import json
import logging
import pathlib
import random
import select
import subprocess
import sys
from typing import Dict, List

from .commands import Commands
from .network import Network


class Player(Commands):
    def __init__(self, server_address: tuple, team_name: str):
        self.server_address = server_address
        self.team_name = team_name
        self.network = Network(server_address)
        Commands().__init__(self.network, team_name)
        self.buffer_size = 4096
        self.food_stock = 0
        self.level = 1
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

    def connect(self):
        try:
            self.network.connect()
            welcome_message = self.network.receive_message()
            logging.debug(f"Received: {welcome_message}")
            if "WELCOME" in welcome_message:
                self.network.send_message(self.team_name)
                client_num = self.network.receive_message()
                logging.debug(f"Received: {client_num}")
        except Exception as e:
            logging.warning(f"Failed to connect: {e}")

    def handle_look_response(self, look_response: str) -> List[List[str]]:
        tiles = look_response.strip("[]").split(",")
        return [tile.strip().split() for tile in tiles]

    def parse_look_response(self, look_response: str) -> List[str]:
        look_response = look_response.replace("Received: ", "")
        response = look_response.strip("[]").split(", ")
        player_tile = response[0].replace("player,", "").strip()
        items = player_tile.split()
        return items

    def take_all_on_tile(self, tile_data: List[str]):
        if tile_data:
            for item in tile_data:
                self.take(item)

    def handle_tile_actions(self, tiles: List[List[str]]):
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
                    self.take(obj)
                    if obj == "food":
                        self.food_stock += 1
                    else:
                        self.resources[obj] += 1

    def can_reproduce(self) -> bool:
        return self.food_stock >= 1

    def reproduce(self):
        if self.can_reproduce():
            logging.debug("Reproducing")
            self.fork()

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

    def evolve(self):
        if not self.can_evolve():
            logging.debug("Cannot evolve yet. Insufficient resources.")

        logging.debug("Starting incantation")
        evolution_result = self.start_incantation()

        if "Elevation underway" not in evolution_result:
            logging.debug("Elevation did not start")

        logging.debug("Elevation underway")
        final_result = self.network.receive_message()

        if "Current level" not in final_result:
            logging.debug("Elevation failed")

        self.level += 1
        requirements = self.elevation_requirements[str(self.level)]
        for resource, amount in requirements.items():
            if (
                resource != "players"
            ):  # We don't subtract players, just resources
                self.resources[resource] -= amount
        logging.debug(f"Evolved to level {self.level}")

    def broadcast_message(self, message: str):
        self.broadcast(message)

    def handle_incoming_messages(self, message: str):
        if "message" in message:
            _, direction, text = message.split(", ")
            direction = int(direction)
            if text.startswith("level "):
                other_level = int(text.split(" ")[1])
                if other_level == self.level:
                    pass
            # Handle other types of incoming messages as needed

    def look_for_food(self):
        look_response = self.look()
        tiles = self.parse_look_response(look_response)
        # if tiles[0] == "food":
        tiles.pop(0)
        logging.debug(f"Look response: {tiles}")
        return tiles

    def calculate_direction(self, food_tile_index):
        if food_tile_index == 1:
            return "right"
        elif food_tile_index == 2:
            return "left"
        return "up"

    def move_to_food(self, food_tile_index):
        direction = self.calculate_direction(food_tile_index)
        if direction == "left":
            self.turn_left()
        elif direction == "right":
            self.turn_right()
        self.move_up()
        new_tile = self.look()[0]
        self.handle_tile_actions([new_tile])
        logging.debug(
            f"Food found in tile {food_tile_index}, moving {direction}"
        )

    def random_movement(self):
        if random.choice([True, False]):
            self.turn_left()
        else:
            self.turn_right()
        self.move_up()
        logging.debug("No food found, turning randomly and moving up")

    def search_food(self):
        tiles = self.look_for_food()
        self.take_all_on_tile(tiles)
        food_tile_index = next(
            (i for i, tile in enumerate(tiles) if "food" in tile), None
        )
        if food_tile_index is not None:
            if food_tile_index == 0:
                self.take("food")
                self.food_stock += 1
                logging.debug("Food found and taken")
                self.reproduce()
            else:
                self.move_to_food(food_tile_index)
        else:
            self.random_movement()

    def get_inventory(self):
        inventory_response = self.inventory()
        inventory_response = inventory_response.strip("[]").split(", ")
        inventory = {}
        for item in inventory_response:
            try:
                key, value = item.split()
            except ValueError:
                logging.warning("Error reading item: ", item)
                continue
            inventory[key] = int(value)
        self.food_stock = inventory.get("food", 0)
        self.resources = {
            resource: inventory.get(resource, 0)
            for resource in self.resources.keys()
        }
        logging.debug(f"Updated inventory: {self.resources}")

    def main_loop(self):
        logging.debug("Player main loop")
        while True:
            self.get_inventory()

            if self.can_evolve():
                self.evolve()
            else:
                self.search_food()

            ready_to_read, _, _ = select.select([self.network.sock], [], [], 0)
            if ready_to_read:
                incoming_message = self.network.receive_message()
                if "dead" in incoming_message:
                    logging.debug("Received 'dead', exiting.")
                    self.close()
                    sys.exit(0)
                self.handle_incoming_messages(incoming_message)

            self.broadcast_message(f"level {self.level}")

    def close(self):
        self.network.close()
