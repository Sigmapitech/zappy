#!/usr/bin/env python3
import os
import pathlib
import sys


def has_correct_message(msg: str) -> bool:
    print(f"Commit message:\n{msg}")
    title = msg.splitlines()[0]

    if msg == "squash\n":
        return True

    verb, *words = title.split()
    report = {
        "  " in title: "double space!",
        title.startswith(" "): "leading space",
        title.endswith(" "): "trailling space",
        verb[0].islower(): "verb is not capitalized",
        any(map(verb.endswith, {"ed", "ing", "s"})): "use imperative tense",
        title.endswith("."): "message should not end with a period",
        len(words) < 2: "include at least 3 words",
    }.get(True)

    if report is None:
        return True

    print(report)
    return False


def main():
    if len(sys.argv) != 2:
        return os.EX_USAGE

    message = pathlib.Path(sys.argv[1]).read_text()
    return not has_correct_message(message)


if __name__ == "__main__":
    sys.exit(main())
