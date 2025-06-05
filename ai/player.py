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
    def main_loop(self):
        print("Player main loop")
        while True:

            ready_to_read, _, _ = select.select([self.network.sock], [], [], 0)
            if ready_to_read:
                incoming_message = self.network.receive_message()
                if "dead" in incoming_message:
                    print("Received 'dead', exiting.")
                    self.close()
                    sys.exit(0)

    def close(self):
        self.network.close()
