import argparse
import subprocess
import sys
import time

from .binpool import ZappyPool
from .team_names import generate_name


def shutdown(processes):
    print("\nReceived interrupt, terminating subprocesses...")
    for p in processes:
        p.terminate()
    for p in processes:
        try:
            p.wait(timeout=5)
        except subprocess.TimeoutExpired:
            print(f"Process {p.pid} did not exit in time, killing it.")
            p.kill()
    sys.exit(0)


def run_zappy(bins: ZappyPool, args: argparse.Namespace):
    teams = [generate_name() for _ in range(args.team_count)]

    srv = subprocess.Popen(
        (
            bins.server,
            "-p",
            str(args.port),
            "-x",
            str(args.map_width),
            "-y",
            str(args.map_height),
            "-n",
            *teams,
            "-c",
            str(args.team_init_cap),
            "-f",
            str(args.freq),
        )
    )

    time.sleep(1)

    ais = [
        subprocess.Popen(
            (bins.ai, "-h", "0.0.0.0", "-p", str(args.port), "-n", team)
        )
        for _ in range(args.team_init_count)
        for team in teams
    ]

    gui = subprocess.Popen((bins.gui, "-h", "0.0.0.0", "-p", str(args.port)))
    processes = [srv, gui] + ais

    try:
        while True:
            pass
    except KeyboardInterrupt:
        shutdown(processes)
