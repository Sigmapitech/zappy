import subprocess

from .network import Network


class Commands:
    def __init__(self, network: Network, team_name: str):
        self.network = network
        self.team_name = team_name

    def send_command(self, command: str) -> str:
        print(f"Sending: {command}")
        self.network.send_message(command)
        response = self.network.receive_message()
        print(f"Sent: {command}, Received: {response}")
        return response

    def look(self) -> str:
        return self.send_command("Look")

    def take(self, obj: str) -> str:
        return self.send_command(f"Take {obj}")

    def set(self, obj: str) -> str:
        return self.send_command(f"Set {obj}")

    def move_up(self) -> str:
        return self.send_command("Forward")

    def turn_right(self) -> str:
        return self.send_command("Right")

    def turn_left(self) -> str:
        return self.send_command("Left")

    def start_incantation(self) -> str:
        return self.send_command("Incantation")

    def broadcast(self, message: str) -> str:
        self.network.send_message(f"Broadcast {message}")
        return self.network.receive_message()

    def fork(self) -> str:
        # Lancer un sous-processus
        subprocess.Popen(
            [
                "python3",
                "-m",
                "ia.zappy_ia",
                "-p",
                str(self.network.server_address[1]),
                "-n",
                self.team_name,
                "-h",
                self.network.server_address[0],
            ]
        )
        return self.send_command("Fork")

    def connect_nbr(self) -> str:
        return self.send_command("Connect_nbr")

    def inventory(self) -> str:
        return self.send_command("Inventory")

    def eject(self) -> str:
        return self.send_command("Eject")
