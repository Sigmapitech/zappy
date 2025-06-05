import random
import pathlib
import json
import sys
import select
import subprocess
from .commands import Commands
from .network import Network
from typing import List, Dict

class Player:
    def __init__(self, server_address: tuple, team_name: str):
        self.server_address = server_address
        self.team_name = team_name
        self.network = Network(server_address)
        self.commands = Commands(self.network, team_name)
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
            print(f"Received: {welcome_message}")
            if "WELCOME" in welcome_message:
                self.network.send_message(self.team_name)
                client_num = self.network.receive_message()
                print(f"Received: {client_num}")
        except Exception as e:
            print(f"Failed to connect: {e}")

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
                self.commands.take(item)

    def handle_tile_actions(self, tiles: List[List[str]]):
        for tile in tiles:
            for obj in ["food", "linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"]:
                if obj in tile:
                    self.commands.take(obj)
                    if obj == "food":
                        self.food_stock += 1
                    else:
                        self.resources[obj] += 1

    def can_reproduce(self) -> bool:
        return self.food_stock >= 1

    def reproduce(self):
        if self.can_reproduce():
            self.commands.fork()
            print("Reproducing")
            # Lancer un nouveau processus pour une nouvelle instance du joueur
            subprocess.Popen([sys.executable, __file__, *sys.argv[1:]])

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
        if self.can_evolve():
            print("Starting incantation")
            self.commands.start_incantation()
            evolution_result = self.network.receive_message()
            if "Elevation underway" in evolution_result:
                print("Elevation underway")
                final_result = self.network.receive_message()
                if "Current level" in final_result:
                    self.level += 1
                    requirements = self.elevation_requirements[str(self.level)]
                    for resource, amount in requirements.items():
                        if resource != "players":  # We don't subtract players, just resources
                            self.resources[resource] -= amount
                    print(f"Evolved to level {self.level}")
                else:
                    print("Elevation failed")
            else:
                print("Elevation did not start")
        else:
            print("Cannot evolve yet. Insufficient resources.")

    def broadcast_message(self, message: str):
        self.commands.broadcast(message)

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
        look_response = self.commands.look()
        tiles = self.parse_look_response(look_response)
        # if tiles[0] == "food":
        tiles.pop(0)
        print(f"Look response: {tiles}")
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
            self.commands.turn_left()
        elif direction == "right":
            self.commands.turn_right()
        self.commands.move_up()
        new_tile = self.commands.look()[0]
        self.handle_tile_actions([new_tile])
        print(f"Food found in tile {food_tile_index}, moving {direction}")

    def random_movement(self):
        if random.choice([True, False]):
            self.commands.turn_left()
        else:
            self.commands.turn_right()
        self.commands.move_up()
        print("No food found, turning randomly and moving up")

    def search_food(self):
        tiles = self.look_for_food()
        self.take_all_on_tile(tiles)
        food_tile_index = next(
            (i for i, tile in enumerate(tiles) if "food" in tile), None
        )
        if food_tile_index is not None:
            if food_tile_index == 0:
                self.commands.take("food")
                self.food_stock += 1
                print("Food found and taken")
                self.reproduce()
            else:
                self.move_to_food(food_tile_index)
        else:
            self.random_movement()

    def get_inventory(self):
        inventory_response = self.commands.inventory()
        inventory_response = inventory_response.strip("[]").split(", ")
        inventory = {}
        for item in inventory_response:
            key, value = item.split()
            inventory[key] = int(value)
        self.food_stock = inventory.get("food", 0)
        self.resources = {resource: inventory.get(resource, 0) for resource in self.resources.keys()}
        print(f"Updated inventory: {self.resources}")

    def main_loop(self):
        print("Player main loop")
        while True:
            print("je passe")
            self.get_inventory()

            if self.can_evolve():
                self.evolve()
            else:
                self.search_food()

            ready_to_read, _, _ = select.select([self.network.sock], [], [], 0)
            if ready_to_read:
                incoming_message = self.network.receive_message()
                if "dead" in incoming_message:
                    print("Received 'dead', exiting.")
                    self.close()
                    sys.exit(0)
                self.handle_incoming_messages(incoming_message)

            self.broadcast_message(f"level {self.level}")

    def close(self):
        self.network.close()
