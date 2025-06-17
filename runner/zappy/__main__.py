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
        .add_arg("--use-ref-gui", action="store_true")
        .add_arg("--use-ref-server", action="store_true")
        .add_arg("--nom", action="store_true")
        .add_arg("--use-respective-branches", action="store_true")
        .add_arg("--gui-branch", type=str)
        .add_arg("--server-branch", type=str)
        .add_arg("--ai-branch", type=str)
        .parse_args()
    )


def main():
    args = parse_args()

    build_settings = 0
    for c, flag in enumerate(
        (
            args.use_ref_gui,
            args.use_ref_server,
            args.nom,
            args.use_respective_branches,
        )
    ):
        build_settings |= int(flag) << c

    branches = (args.server_branch, args.gui_branch, args.ai_branch)
    bins = create_bin_pool(Settings(build_settings), branches)
    run_zappy(bins, args)

    return EXIT_KO


if __name__ == "__main__":
    sys.exit(main())
