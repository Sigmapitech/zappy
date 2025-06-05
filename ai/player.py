import json
import pathlib
import random
import select
import subprocess
import sys
from typing import Dict, List

from .network import Network


class Player:
    def __init__(self, server_address: tuple, team_name: str):
        self.server_address = server_address
        self.team_name = team_name
        self.network = Network(server_address)
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
