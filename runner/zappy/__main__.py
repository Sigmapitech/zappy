#!/usr/bin/env python3
"""
Usage:
  --map-width=<width>, default: 10
  --map-height=<height>, default: 10
  --team-count=<num>, default: 3
  --team-init-count=<num>, default: 2
  --team-init-cap=<num>, default: 200
  --freq=<frequency>, default: 100
  --port=<port>, default: find a port >= 1024
  --use-ref-gui, default: false
  --use-ref-server, default: false
  --nom, default: false; Use nix output monitor
  --use-respective-branches   (default: use dev branch)
"""

import sys

EXIT_KO = 84


def main():
    return EXIT_KO


if __name__ == "__main__":
    sys.exit(main())
