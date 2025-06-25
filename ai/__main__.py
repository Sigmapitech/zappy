import argparse
import asyncio
import logging
import os
import time
from functools import wraps
from typing import Any, NoReturn, Sequence

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
    "--version": dict(action="version", version=f"{__package__}, v{__version__}"),
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Player for the game", add_help=False)

    for flags, settings in CLI_ARGS.items():
        if isinstance(flags, str):
            flags = (flags,)

        parser.add_argument(*flags, **settings)
    return parser.parse_args()


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


def make_async(func):
    def wrapper_signature() -> NoReturn: ...

    @wraps(wrapper_signature)
    def wrapped():
        asyncio.run(func())

    return wrapped


@make_async
async def main():
    setup_logger()

    args = parse_args()
    client = Player(args.host, args.port)

    await client.connect(team=args.name)
    try:
        await client.run_until_death()
    except KeyboardInterrupt:
        logging.info("Player interrupted by user.")
    except ConnectionError:
        logging.debug("Waiting for all child processes to exit...")
        while client.childs:
            for pid in client.childs[:]:  # Iterate over a copy of the list
                finished_pid, status = os.waitpid(pid, os.WNOHANG)
                if finished_pid != 0:
                    print(f"Child {finished_pid} exited with status {status}")
                    client.childs.remove(pid)
            time.sleep(0.1)  # Avoid tight CPU loop


if __name__ == "__main__":
    main()
