import os

TARGET_COLUMN = 78
ROOT_DIR = "."
TAB_SIZE = 4


def align_slash(line: str) -> str:
    line = line.rstrip()

    if not line.endswith("\\"):
        return line + "\n"

    content = line.rstrip("\\ \t")[:]
    spaces_needed = TARGET_COLUMN - len(content.replace("\t", " " * TAB_SIZE))

    if spaces_needed > 0:
        return content + (" " * spaces_needed) + "\\\n"

    return line + "\n"


def align_slash_in_file(file_path):
    with open(file_path, encoding="utf-8") as f:
        lines = [line for line in f]

    lines_out = [align_slash(line) for line in lines]

    if lines != lines_out:
        with open(file_path, "w", encoding="utf-8") as f:
            f.writelines(lines_out)

        print(f"Aligned slashes in: {file_path}")


def main():
    submodules = []

    if os.path.exists(".gitmodules"):
        with open(".gitmodules") as f:
            submodules = [
                line.split("=", 1)[1].strip()
                for line in f
                if line.strip().startswith("path = ")
            ]
    collected_files = (
        os.path.join(root, fname)
        for root, _, files in os.walk(ROOT_DIR)
        for fname in files
        if fname.endswith((".h", ".hpp", ".c", ".cpp", "Makefile", "Doxyfile"))
        and not any(sub in root for sub in submodules)
    )

    for path in collected_files:
        align_slash_in_file(path)


if __name__ == "__main__":
    main()
