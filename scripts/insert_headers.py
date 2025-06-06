import os
import pathlib
from datetime import datetime

PROJECT_NAME = "plazza"

_HEADER_TEMPLATE = (
    "{start}\n"
    "{middle} EPITECH PROJECT, {year}\n"
    "{middle} {project_name}\n"
    "{middle} File description:\n"
    "{middle} {filename}\n"
    "{end}\n"
)

HEADER_CHARACTERS = {
    "h": ("/*", "**", "*/"),
    "c": ("/*", "**", "*/"),
    "hpp": ("/*", "**", "*/"),
    "cpp": ("/*", "**", "*/"),
    "mk": ("##", "##", "##"),
    "Makefile": ("##", "##", "##"),
}


def insert_header_in_file(dirs: str, filename: str):
    filepath = os.path.join(dirs, filename)
    type = filename.split(".")[-1] if filename != "Makefile" else "Makefile"
    file_content = pathlib.Path(filepath).read_text()

    start, middle, end = HEADER_CHARACTERS[type]
    header = _HEADER_TEMPLATE.format(
        start=start,
        middle=middle,
        end=end,
        year=datetime.now().year,
        project_name=PROJECT_NAME,
        filename=filename,
    )

    if file_content.startswith(header):
        return

    if file_content.startswith(start):
        i = 0
        for i, line in enumerate(file_content.split("\n")):
            if not (
                line.startswith(end)
                or line.startswith(middle)
                or line.startswith(start)
            ):
                break
        file_content = "\n".join(file_content.split("\n")[i + 1 :])

    with open(filepath, "w+") as f:
        f.write(f"{header}\n{file_content}")


def main():
    for dirs, _, files in os.walk("."):
        for filename in files:
            if not any(
                filename.endswith(ext)
                for ext in {".c", ".h", ".cpp", ".hpp", ".mk", "Makefile"}
            ):
                continue

            insert_header_in_file(dirs, filename)


if __name__ == "__main__":
    main()
