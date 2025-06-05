import os
from pathlib import Path

ROOT_DIR = "."
HEADER_FORMAT = (
    "/*",
    "** EPITECH PROJECT, ",
    "**",
    "** File description:",
    "**",
    "*/",
)


def should_remove_source_headers(lines: list[str]) -> bool:
    if len(lines) < 5:
        return False

    return all(
        line.strip().startswith(header_format)
        for line, header_format in zip(lines, HEADER_FORMAT)
    )


def fix_header(file: Path) -> None:
    with open(file) as f:
        lines = f.readlines()

    if not should_remove_source_headers(lines):
        return

    skip = len(HEADER_FORMAT)
    for line in lines[skip:]:
        if len(line.strip()) != 0:
            break
        skip += 1

    print(file, skip)
    with open(file, "w") as f:
        f.writelines(lines[skip:])


def main():
    collected_files = (
        os.path.join(root, fname)
        for root, _, files in os.walk(ROOT_DIR)
        for fname in files
        if fname.endswith((".h", ".hpp", ".c", ".cpp"))
    )

    for file in collected_files:
        fix_header(Path(file))


if __name__ == "__main__":
    main()
