import argparse
import signal
import sys
from contextlib import contextmanager
from .player import Player

def parse_arguments():
    parser = argparse.ArgumentParser(
        description="Player for the game", add_help=False
    )
    parser.add_argument(
        "-p",
        "--port",
        type=int,
        required=True,
        help="The port of the server (e.g., 4242)",
    )
    parser.add_argument(
        "-n",
        "--name",
        type=str,
        required=True,
        help='The name of the team (e.g., "toto")',
    )
    parser.add_argument(
        "-h",
        "--host",
        type=str,
        required=True,
        help='The address of the server (e.g., "localhost")',
    )
    parser.add_argument(
        "-a",
        "--help",
        action="help",
        default=argparse.SUPPRESS,
        help="Show this help message and exit",
    )
    return parser.parse_args()

@contextmanager
def game(player):
    player.connect()
    print("Player connected")
    yield player
    player.close()

def main():
    # signal.signal(signal.SIGINT, lambda *_: sys.exit(0))
    args = parse_arguments()
    print("args:", args)
    server_address = (args.host, args.port)
    team_name = args.name
    with game(player := Player(server_address, team_name)) as player:
        player.main_loop()

if __name__ == "__main__":
    main()
