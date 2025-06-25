#!/usr/bin/env python3
import argparse
import sys
from typing import Self

from .binpool import Settings, create_bin_pool
from .run import run_zappy

EXIT_KO = 84


class ArgBuilder(argparse.ArgumentParser):

    def add_arg(self, *args, **kwargs) -> Self:
        self.add_argument(*args, **kwargs)
        return self


def parse_args() -> argparse.Namespace:
    return (
        ArgBuilder()
        .add_arg("--map-width", type=int, default=10)
        .add_arg("--map-height", type=int, default=10)
        .add_arg("--team-count", type=int, default=3)
        .add_arg("--team-init-count", type=int, default=2)
        .add_arg("--team-init-cap", type=int, default=200)
        .add_arg("--freq", type=int, default=100)
        .add_arg("--port", type=int, default=4242)
        .add_arg("--ref-gui", action="store_true")
        .add_arg("--ref-server", action="store_true")
        .add_arg("--nom", action="store_true")
        .add_arg("--respective-branches", action="store_true")
        .add_arg("--branch-gui", type=str)
        .add_arg("--branch-server", type=str)
        .add_arg("--branch-ai", type=str)
        .add_arg("--local-ai", action="store_true")
        .add_arg("--local-gui", action="store_true")
        .add_arg("--local-server", action="store_true")
        .add_arg("--debug-server", action="store_true")
        .add_arg("--basic-team-names", action="store_true")
        .add_arg("--no-server", action="store_true")
        .add_arg("--no-gui", action="store_true")
        .add_arg("--split-logs", action="store_true")
        .add_arg("--host", type=str, default="0.0.0.0")
        .parse_args()
    )


def main():
    args = parse_args()

    if args.host != "0.0.0.0":
        args.no_server = True
    if args.ref_server:
        args.debug_server = False

    build_settings = 0
    for c, flag in enumerate(
        (
            args.ref_gui,
            args.ref_server,
            args.nom,
            args.respective_branches,
            args.local_server,
            args.local_gui,
            args.local_ai,
            args.debug_server,
        )
    ):
        build_settings |= int(flag) << c

    branches = (args.branch_server, args.branch_gui, args.branch_ai)
    bins = create_bin_pool(Settings(build_settings), branches)
    run_zappy(bins, args)

    return EXIT_KO


if __name__ == "__main__":
    sys.exit(main())
