import random

CONSONANT_STARTS = [
    "Z",
    "P",
    "F",
    "M",
    "T",
    "K",
    "L",
    "G",
    "B",
    "D",
    "Zr",
    "Pr",
    "Tr",
    "Fr",
    "Kl",
    "Gr",
    "Bl",
    "Dr",
    "Pl",
    "Kr",
]

VOWELS = ["a", "e", "i", "o", "u", "y", "ae", "ia", "io", "ea", "ou"]

CONSONANT_MIDDLES = [
    "r",
    "l",
    "n",
    "k",
    "g",
    "z",
    "t",
    "m",
    "ck",
    "th",
    "sh",
    "v",
    "d",
    "b",
]

SUFFIXES = [
    "ia",
    "or",
    "is",
    "el",
    "ar",
    "on",
    "us",
    "ir",
    "en",
    "al",
    "er",
    "az",
    "ek",
    "il",
    "il",
]


def generate_name() -> str:
    start = random.choice(CONSONANT_STARTS)
    vowel1 = random.choice(VOWELS)
    middle = random.choice(CONSONANT_MIDDLES)
    vowel2 = random.choice(VOWELS)
    suffix = random.choice(SUFFIXES)

    name_parts = [start, vowel1]

    if random.random() > 0.3:
        name_parts.append(middle)

    if random.random() > 0.5:
        name_parts.append(vowel2)

    name_parts.append(suffix)

    name = "".join(name_parts)

    return name.capitalize()
