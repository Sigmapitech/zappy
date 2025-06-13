import argparse
import logging
from contextlib import contextmanager
from typing import Any, Sequence

from .player import Player

__version__ = "0.0.1"

CLI_ARGS: dict[Sequence[str], dict[str, Any]] = {
    ("-p", "--port"): dict(
        type=int,
        required=True,
        help="The port of the server (e.g., 4242)",
    ),
    ("-n", "--name"): dict(
        type=str,
        required=True,
        help='The name of the team (e.g., "toto")',
    ),
    ("-h", "--host"): dict(
        type=str,
        required=True,
        help='The address of the server (e.g., "localhost")',
    ),
    ("-a", "--help"): dict(
        action="help",
        default=argparse.SUPPRESS,
        help="Show this help message and exit",
    ),
    "--version": dict(
        action="version", version=f"{__package__}, v{__version__}"
    ),
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Player for the game", add_help=False
    )

    for flags, settings in CLI_ARGS.items():
        if isinstance(flags, str):
            flags = (flags,)

        parser.add_argument(*flags, **settings)
    return parser.parse_args()


@contextmanager
def game(player):
    player.connect()
    print("Player connected")
    yield player
    player.close()


def setup_logger():
    logger = logging.getLogger(__package__)

    logger.setLevel(logging.DEBUG)
    formatter = logging.Formatter(
        "%(asctime)s %(name)-11s:%(lineno)-4d %(levelname)-8s %(message)s",
        datefmt="%H:%M:%S",
    )

    stream_handler = logging.StreamHandler()
    stream_handler.setFormatter(formatter)

    logger.addHandler(stream_handler)


def main():
    setup_logger()

    # signal.signal(signal.SIGINT, lambda *_: sys.exit(0))
    args = parse_args()

    print("args:", args)
    server_address = (args.host, args.port)
    team_name = args.name

    with game(Player(server_address, team_name)) as player:
        player.main_loop()


if __name__ == "__main__":
    main()
