import random
import socket
import select
import sys
import signal
import os

HOST = "127.0.0.1"
PORT = 4242

OBJECTS = (
    "",
    "egg",
    "player",
    "food",
    "linemate",
    "deraumere",
    "sibur",
    "mendiane",
    "phiras",
    "thystame",
)

COMMANDS_WITH_WEIGHTS = (
    ("Forward", 10),
    ("Left", 8),
    ("Right", 8),
    ("Look", 5),
    ("Broadcast \"hello\"", 1),
    ("Connect_nbr", 2),
    ("Fork",1),
    ("Eject",4),
    ("Take obj",5),
    ("Set obj",5),
    ("Incantation",3),
    ("Plop",3),
)

COMMANDS_SUBLISTS = (
    [cmd for _ in range(weight)]
    for cmd, weight in COMMANDS_WITH_WEIGHTS
)

COMMANDS = tuple(x for y in COMMANDS_SUBLISTS for x in y)


def main():
    def handle_sigint(signum, frame):
        print("\nDisconnected from server.")
        sock.close()
        sys.exit(0)

    signal.signal(signal.SIGINT, handle_sigint)

    # Create non-blocking socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock.connect((HOST, PORT))
    except BlockingIOError:
        pass  # Expected for non-blocking connect

    poller = select.poll()
    poller.register(sock, select.POLLIN)

    sock.sendall(sys.argv[1].encode() +  b"\n")
    if sock.recv(4096).startswith(b"ko"):
        return

    while True:
        events = poller.poll(100)

        for fd, event in events:
            if fd == sock.fileno():
                if event & select.POLLIN:
                    try:
                        data = sock.recv(4096)
                        if not data:
                            print("Server closed the connection.\n")
                            return
                        print("<-", data.decode().removesuffix("\n"))
                    except Exception as e:
                        print("Read error:", e)

                random_cmd = random.choice(COMMANDS).replace(
                    "obj", random.choice(OBJECTS)).encode()
                print("->", random_cmd)
                sock.sendall(random_cmd + b"\n")

                if random_cmd.startswith(b"Fork"):
                    pid = os.fork()
                    if pid == 0:
                        os.execv(sys.executable, [sys.executable] + sys.argv)


if __name__ == "__main__":
    main()
