import argparse
import os
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


def run_proc(cmd, logger, proc_list):
    proc = subprocess.Popen(cmd, stdout=logger, stderr=subprocess.STDOUT)
    proc_list.append(proc)


def run_zappy(bins: ZappyPool, args: argparse.Namespace):
    if not args.basic_team_names:
        teams = [generate_name() for _ in range(args.team_count)]
    else:
        teams = [f"team{i}" for i in range(1, args.team_count + 1)]

    processes = []
    log_files = []

    if args.split_logs and not os.path.exists("logs"):
        os.makedirs("logs")

    def make_log(name: str):
        if args.split_logs:
            if not os.path.exists("logs/.gitginore"):
                with open("logs/.gitignore", "w+") as f:
                    f.write("*")

            log_path = f"logs/{name}.log"
            f = open(log_path, "w")
            log_files.append(f)
            return f
        return None

    if not args.no_server:
        run_proc(
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
            ),
            make_log("server"),
            processes,
        )

    time.sleep(1)
    if args.pause_before_connections:
        input("Press enter to continue...")

    if not args.no_ai:
        for i, team in (
            (i, team) for i in range(args.team_init_count) for team in teams
        ):
            run_proc(
                (bins.ai, "-h", args.host, "-p", str(args.port), "-n", team),
                make_log(f"ai_{i}_{team}"),
                processes,
            )

    if not args.no_gui:
        run_proc(
            (bins.gui, "-h", args.host, "-p", str(args.port)),
            make_log("gui"),
            processes,
        )

    try:
        if args.split_logs:
            cmd = "tail -f logs/server.log"

            if os.path.exists("./bleach"):
                cmd += " | tee /dev/stderr | ./bleach > logs/server-bleach.log"

            os.system(cmd)
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        shutdown(processes)
    finally:
        for f in log_files:
            f.close()
