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


@dataclass
class Source:
    branch: str
    deriv: str

    @property
    def flakepath(self) -> str:
        return f"github:Sigmapitech/zappy/{self.branch}#{self.deriv}"

    def __hash__(self) -> int:
        return hash(self.flakepath)


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


def create_bin_pool(
    build: Settings, branches: tuple[str, str, str]
) -> ZappyPool:
    for branch, name in zip(branches, ("server", "gui", "ai")):
        if branch is not None:
            SOURCES[name].branch = branch

    gui = "ref-gui" if build & Settings.USE_REF_GUI else "gui"
    srv = "ref-server" if build & Settings.USE_REF_SERVER else "server"

    if build & Settings.USE_RESPECTIVE_BRANCHES:
        pool = {SOURCES["ai"], SOURCES[gui], SOURCES[srv]}

    else:
        pool = {SOURCES["dev"]}

    deriv_paths = {}
    for target in pool:
        out = Path(build_package(target, bool(build & Settings.USE_NOM)))

        for entry in (out / "bin").iterdir():
            deriv_paths[entry.name.removeprefix("zappy_")] = entry

    return ZappyPool(
        server=deriv_paths[srv], gui=deriv_paths[gui], ai=deriv_paths["ai"]
    )
