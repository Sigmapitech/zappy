#!/usr/bin/env python3
##
## EPITECH PROJECT, 2025
## zappy
## File description:
## look
##

import sys
import random
from enum import Enum


class Ressource(Enum):
    FOOD = 0
    LINEMATE = 1
    DERAUMERE = 2
    SIBUR = 3
    MENDIANE = 4
    PHIRAS = 5
    THYSTAME = 6

    def __str__(self):
        return self.name.lower()


class Direction(Enum):
    UP = 0
    RIGHT = 1
    DOWN = 2
    LEFT = 3

    def __str__(self):
        return self.name.lower()


class Map:
    def __init__(self, width, height):
        self.width: int = width
        self.height: int = height
        self.grid: list[list[list[Ressource]]] = [
            [[] for _ in range(width)] for _ in range(height)
        ]
        for i, density in enumerate([0.5, 0.3, 0.15, 0.1, 0.1, 0.08, 0.05]):
            ressource_count = int(width * height * density)
            for _ in range(ressource_count):
                x = random.randint(0, width - 1)
                y = random.randint(0, height - 1)
                self.grid[y][x].append(Ressource(i))


def rotate(dx, dy, direction):
    if direction == Direction.UP:
        return dx, -dy
    elif direction == Direction.DOWN:
        return -dx, dy
    elif direction == Direction.LEFT:
        return -dy, -dx
    elif direction == Direction.RIGHT:
        return dy, dx
    # Default case to avoid returning None (can't happen)
    # but python needs a return statement
    return dx, dy

def flood_cone(game_map, posx, posy, direction, level):
    width = game_map.width
    height = game_map.height
    result = []

    for l in range(level + 1):
        for i in range(-l, l + 1):
            dx, dy = rotate(i, l, direction)
            x = (posx + dx) % width
            y = (posy + dy) % height
            result.append((x, y, game_map.grid[y][x]))
    return result


if __name__ == "__main__":
    if len(sys.argv) != 6:
        print("Usage: python look.py <width> <height> <posx> <posy> <level>")
        sys.exit(1)

    width, height = map(int, sys.argv[1:3])
    posx, posy = map(int, sys.argv[3:5])
    level = int(sys.argv[5])
    game_map = Map(width, height)

    for y in range(height):
        for x in range(width):
            ressource_names = [str(Ressource(i)) for i in game_map.grid[y][x]]
            print(
                f"Cell ({x}, {y}): {', '.join(ressource_names) if ressource_names else 'empty'}"
            )

    for dir in Direction:
        print(
            f"Looking in direction {dir} from position ({posx}, {posy}) at level {level}:"
        )
        results = flood_cone(game_map, posx, posy, dir, level)
        for x, y, resources in results:
            ressource_names = [str(res) for res in resources]
            print(
                f"  Cell ({x}, {y}): {', '.join(ressource_names) if ressource_names else 'empty'}"
            )
