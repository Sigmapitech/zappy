#!/usr/bin/env python3
##
## EPITECH PROJECT, 2025
## zappy
## File description:
## sound
##

import math
import sys
from enum import Enum


class Direction(Enum):
    UP = 0
    RIGHT = 1
    DOWN = 2
    LEFT = 3

    def __str__(self):
        return self.name.lower()


class RelativeSoundDirection(Enum):
    UP = 0
    UP_LEFT = 1
    LEFT = 2
    DOWN_LEFT = 3
    DOWN = 4
    DOWN_RIGHT = 5
    RIGHT = 6
    UP_RIGHT = 7
    ON = 8

    def __str__(self):
        return self.name.lower()


def torus_delta(x1, y1, x2, y2, width, height):
    dx = (x1 - x2) % width
    if dx > width // 2:
        dx -= width
    dy = (y1 - y2) % height
    if dy > height // 2:
        dy -= height
    return dx, dy


def angle_to_direction_index(
    dx, dy, facing_dir: Direction
) -> RelativeSoundDirection:
    if dx == 0 and dy == 0:
        return RelativeSoundDirection.ON

    angle_to_source = math.atan2(-dy, dx)
    facing_angle = facing_dir.value * (math.pi / 2)

    relative_angle = (facing_angle - angle_to_source + 2 * math.pi) % (
        2 * math.pi
    )
    index = round(relative_angle / (math.pi / 4)) % 8

    return RelativeSoundDirection(index)


if __name__ == "__main__":
    if len(sys.argv) != 7:
        print(
            "Usage: python sound.py <width> <height> <posx1> <posy1> <posx2> <posy2>"
        )
        sys.exit(1)

    width, height = map(int, sys.argv[1:3])
    posx1, posy1 = map(int, sys.argv[3:5])
    posx2, posy2 = map(int, sys.argv[5:7])

    dx, dy = torus_delta(posx2, posy2, posx1, posy1, width, height)
    print(f"dx, dy = {dx}, {dy} from p2 to p1")

    for dir2 in Direction:
        rel_dir = angle_to_direction_index(dx, dy, dir2)
        print(f"Facing {dir2}: rel_dir = {rel_dir.value} ({rel_dir})")
