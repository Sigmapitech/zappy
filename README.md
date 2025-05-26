# ZAPPY

<!-- Badges -->
![C](https://img.shields.io/badge/language-C-blue.svg?logo=c&logoColor=white)
![C++](https://img.shields.io/badge/language-C%2B%2B-purple.svg?logo=c%2B%2B&logoColor=white)
![Python](https://img.shields.io/badge/language-Python-yellow.svg?logo=python&logoColor=white)
![Makefile](https://img.shields.io/badge/build-Makefile-brightgreen?logo=gnu&logoColor=white)
![Linux x86_64](https://img.shields.io/badge/platform-Linux%20x86__64-lightgrey?logo=linux)
![Nix](https://img.shields.io/badge/env-Nix-5277C3?logo=nixos&logoColor=white)
[![Coverage](https://codecov.io/gh/Sigmapitech/zappy/branch/dev/graph/badge.svg?token=5fssEEelNi)](https://codecov.io/gh/Sigmapitech/zappy?branch=dev)
![Build](https://img.shields.io/github/actions/workflow/status/Sigmapitech/zappy/ci.yml?branch=main&logo=github)
![Last Commit](https://img.shields.io/github/last-commit/Sigmapitech/zappy?logo=git)
![Issues](https://img.shields.io/github/issues/Sigmapitech/zappy?logo=github)
![Pull Requests](https://img.shields.io/github/issues-pr/Sigmapitech/zappy?logo=github)
![Code style](https://img.shields.io/badge/code%20style-clangformat-blue?logo=clang)

## Overview

ZAPPY is a network game about managing a world and its inhabitants, where several teams
confront each other on a tile map containing resources. The winning team is the first one
where at least 6 players reach the maximum elevation. To do so, a team of player AIs is
launched on a board representing their planet, communicating –and sabotaging other teams–
together to achieve victory.

## Features

- ...

## Requirements

- **gcc**
- **python3**
- **make**
- **Nix** (optional, for reproducible builds via `flake.nix`)

## Project Structure

```sh
.
├── server/
│   └── ...
├── ai/
│   └── ...
├── gui/
│   └── ...
├── scripts/
│   ├── align_columns.py
│   ├── check_commit_message.py
│   ├── discard_headers.py
│   └── insert_headers.py
├── .clang-format
├── .clang-tidy
├── .gitignore
├── Makefile
├── flake.nix
├── flake.lock
├── assignment.pdf
├── zappy_ref-v3.0.0.tgz
├── .github/
│   └── workflows/
│       └── ci.yml
└── (bonus/)
```

## Installation

1. **Clone the repository:**

   ```sh
   git clone <repository-url>
   cd <repository-directory>
   ```

2. **Build the project:**

   ```sh
   make
   ```

   - Use `make re` to force a rebuild.
   - Use `make clean` to remove object files.
   - Use `make fclean` to remove all build artifacts and binaries.

3. **(Optional) Using Nix:**

   ```sh
   nix develop
   make
   ```

## Usage

### Server

Run the server shell with:

```sh
./zappy_server -p port -x width -y height -n name1 name2 ... -c clientsNb -f freq
```

- `-p port`: Port number.
- `-x width`: Width of the world.
- `-y height`: Height of the world.
- `-n name1 name2 ...`: Name(s) of the allowed team(s).
- `-c clientsNb`: Number of authorized clients per team.
- `-f freq`: Reciprocal of time unit for execution of actions.

> [!NOTE]
> The team name `GRAPHIC` is reserved for the GUI to authenticate itself as such to
the server.

### GUI

Run the server shell with:

```sh
./zappy_gui -p port -h machine
```

- `-p port`: Port number.
- `-h machine`: Hostname of the server.

### AI

Run the server shell with:

```sh
./zappy_ai -p port -n name -h machine
```

- `-p port`: Port number.
- `-n name`: Name of the team.
- `-h machine`: Name of the machine; localhost by default.

## Testing

- **Coverage:**
  Run:

  ```sh
  make cov
  ```

  *(To be completed: Add test instructions and coverage details.)*

## Contributors

| Yohann B. | Gabriel H. | Julien B. | Valentin R. | Hugo H. |
|---|---|---|---|---|
| <img src="https://github.com/Sigmanificient.png" width="300em"/> | <img src="https://github.com/Ciznia.png" width="300em"/> | <img src="https://github.com/Fenriir42.png" width="300em"/> | <img src="https://github.com/Valentin22r.png" width="300em"/> | <img src="https://github.com/hop3calypse.png" width="300em"/> |

For more details, see [assignment.pdf](assignment.pdf).
