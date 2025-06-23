import argparse
import asyncio
import logging
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
    "--version": dict(
        action="version", version=f"{__package__}, v{__version__}"
    ),
}


def parse_args() -> argparse.Namespace:
    """
    Parses command-line arguments for the player application.

    This function creates an ArgumentParser, adds arguments defined in the CLI_ARGS dictionary,
    and returns the parsed arguments as a Namespace object.

    Returns:
        argparse.Namespace: The parsed command-line arguments.
    """
    parser = argparse.ArgumentParser(
        description="Player for the game", add_help=False
    )

    for flags, settings in CLI_ARGS.items():
        if isinstance(flags, str):
            flags = (flags,)

        parser.add_argument(*flags, **settings)
    return parser.parse_args()


def setup_logger():
    """
    Configures and returns a logger for the current package.

    The logger is set to the DEBUG level and uses a custom formatter that includes
    the timestamp, logger name, line number, log level, and message. Logging output
    is directed to the standard output stream.

    Returns:
        logging.Logger: Configured logger instance for the current package.
    """
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
    """
    Decorator that converts a synchronous function into an asynchronous one by running it inside an asyncio event loop.

    Args:
        func (Callable): The synchronous function to be wrapped and executed asynchronously.

    Returns:
        Callable: A wrapper function that, when called, runs the original function using asyncio.run().

    Note:
        The wrapped function does not accept arguments and assumes the original function takes no parameters.
    """

    def wrapper_signature() -> NoReturn: ...

    @wraps(wrapper_signature)  # make pyhon happy
    def wrapped():
        asyncio.run(func())

    return wrapped


@make_async
async def main():
    """
    Asynchronous entry point for the AI client.

    This function sets up logging, parses command-line arguments, initializes a Player client,
    connects to the specified server with the provided team name, and runs the client until termination.

    Steps:
        1. Initializes the logger.
        2. Parses command-line arguments for host, port, and team name.
        3. Creates a Player client instance.
        4. Connects the client to the server using the specified team name.
        5. Runs the client until it is terminated.

    Raises:
        Any exceptions raised by the Player client during connection or execution.
    """
    setup_logger()

    args = parse_args()
    client = Player(args.host, args.port)

    await client.connect(team=args.name)
    await client.run_until_death()


if __name__ == "__main__":
    main()
