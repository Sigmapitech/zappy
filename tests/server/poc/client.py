import socket
import select
import sys
import signal
import os

HOST = "127.0.0.1"
PORT = 4242

def main():
    def handle_sigint(signum, frame):
        print("\nDisconnected from server.")
        sock.close()
        sys.exit(0)

    signal.signal(signal.SIGINT, handle_sigint)

    # Create non-blocking socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setblocking(False)
    try:
        sock.connect((HOST, PORT))
    except BlockingIOError:
        pass  # Expected for non-blocking connect

    # Set stdin to non-blocking
    fd_stdin = sys.stdin.fileno()
    os.set_blocking(fd_stdin, False)

    poller = select.poll()
    poller.register(sock, select.POLLIN)
    poller.register(fd_stdin, select.POLLIN)

    print(f"Connected to {HOST}:{PORT}. Press Ctrl+C to quit.")
    print("> ", end="", flush=True)  # Initial prompt

    while True:
        events = poller.poll(100)

        for fd, event in events:
            if fd == sock.fileno():
                if event & select.POLLIN:
                    try:
                        data = sock.recv(4096)
                        if not data:
                            print("\nServer closed the connection.")
                            return
                        # Clear line and print received data
                        print("\rReceived:\n", data.decode(), end="")
                        print("\n> ", end="", flush=True)  # Reprint prompt
                    except Exception as e:
                        print("\nRead error:", e)

            elif fd == fd_stdin:
                try:
                    user_input = os.read(fd_stdin, 4096)
                    if user_input:
                        sock.sendall(user_input)
                        print("> ", end="", flush=True)  # Redraw prompt
                except Exception as e:
                    print("\nSend error:", e)

if __name__ == "__main__":
    main()
