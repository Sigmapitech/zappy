import os
import subprocess
import sys

from .network import Network


class Commands:
    def __init__(self, network: Network, team_name: str):
        self.network = network
        self.team_name = team_name

    def __send_command(self, command: str) -> str:
        print(f"Sending: {command}")
        self.network.send_message(command)
        response = self.network.receive_message()
        print(f"Sent: {command}, Received: {response}")
        return response

    def look(self) -> str:
        return self.__send_command("Look")

    def take(self, obj: str) -> str:
        return self.__send_command(f"Take {obj}")

    def set(self, obj: str) -> str:
        return self.__send_command(f"Set {obj}")

    def move_up(self) -> str:
        return self.__send_command("Forward")

    def turn_right(self) -> str:
        return self.__send_command("Right")

    def turn_left(self) -> str:
        return self.__send_command("Left")

    def start_incantation(self) -> str:
        return self.__send_command("Incantation")

    def broadcast(self, message: str) -> str:
        self.network.send_message(f'"Broadcast {message}"')
        return self.network.receive_message()

    def fork(self):
        res = self.__send_command("Fork")

        if res == "ko":
            return res

        pid = os.fork()
        if pid == 0:
            os.execv(sys.executable, [sys.executable] + sys.argv)

        return res

    def connect_nbr(self) -> str:
        return self.__send_command("Connect_nbr")

    def inventory(self) -> str:
        return self.__send_command("Inventory")

    def eject(self) -> str:
        return self.__send_command("Eject")
