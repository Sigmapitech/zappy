import subprocess
import sys
from dataclasses import dataclass
from enum import IntFlag
from pathlib import Path

EXIT_KO = 84


class Settings(IntFlag):
    NONE = 0

    USE_REF_GUI = 1 << 0
    USE_REF_SERVER = 1 << 1

    USE_NOM = 1 << 2
    USE_RESPECTIVE_BRANCHES = 1 << 3

    USE_LOCAL_SERVER = 1 << 4
    USE_LOCAL_GUI = 1 << 5
    USE_LOCAL_AI = 1 << 6

    USE_SERVER_DEBUG = 1 << 7


@dataclass
class Source:
    branch: str
    deriv: str

    @property
    def flakepath(self) -> str:
        if self.branch == "":
            return f".#{self.deriv}"
        return f"github:Sigmapitech/zappy/{self.branch}#{self.deriv}"

    def __eq__(self, other) -> bool:
        if not isinstance(other, Source):
            raise ValueError
        return self.deriv == other.deriv

    def __hash__(self) -> int:
        return hash(self.deriv)


DEV = "zappy-runner"
SOURCES = {
    "dev": Source(DEV, "default"),
    "ai": Source("ia", "ai"),
    "server": Source("server", "server"),
    "gui": Source("gui", "gui"),
    "ref-gui": Source(DEV, "ref-gui"),
    "ref-server": Source(DEV, "ref-server"),
}


def build_package(source: Source, use_nom: bool) -> str:
    print("Building", source.flakepath)
    cmd = (
        "nom" if use_nom else "nix",
        "build",
        "--refresh",
        source.flakepath,
        "--print-build-logs",
        "--no-link",
        "--print-out-paths",
    )

    try:
        out = subprocess.check_output(cmd, text=True).strip()
        return out

    except subprocess.CalledProcessError as e:
        print(f"Failed to build {source}: {e}")
        sys.exit(EXIT_KO)


@dataclass
class ZappyPool:
    server: str
    gui: str
    ai: str


def make_deriv_paths(pool: set[Source], build: Settings) -> dict[str, str]:
    deriv_paths = {}

    for target in pool:
        out = Path(build_package(target, bool(build & Settings.USE_NOM)))

        for entry in (out / "bin").iterdir():
            deriv_paths[entry.name.removeprefix("zappy_")] = entry

    return deriv_paths


def create_bin_pool(
    build: Settings, branches: tuple[str, str, str]
) -> ZappyPool:
    custom_branches = set()

    for branch, name in zip(branches, ("server", "gui", "ai")):
        if branch is not None:
            SOURCES[name].branch = branch
            custom_branches.add(SOURCES[name])

    mk_local = lambda d: Source(branch="", deriv=d)

    if build & Settings.USE_LOCAL_SERVER:
        custom_branches.add(mk_local("server"))

    if build & Settings.USE_LOCAL_GUI:
        custom_branches.add(mk_local("gui"))

    if build & Settings.USE_LOCAL_AI:
        custom_branches.add(mk_local("ai"))

    gui = "ref-gui" if build & Settings.USE_REF_GUI else "gui"
    srv = "ref-server" if build & Settings.USE_REF_SERVER else "server"

    if build & Settings.USE_RESPECTIVE_BRANCHES:
        pool = {SOURCES["ai"], SOURCES[gui], SOURCES[srv]}

    else:
        pool = {SOURCES["dev"]}

    final_pool = custom_branches | pool

    if build & Settings.USE_SERVER_DEBUG:
        for target in final_pool:
            if target.deriv == "server":
                target.deriv += "-debug"
                srv = "debug_" + srv

    # ensure custom_branches take priority when comparing
    deriv_paths = make_deriv_paths(final_pool, build)

    return ZappyPool(
        server=deriv_paths[srv], gui=deriv_paths[gui], ai=deriv_paths["ai"]
    )
