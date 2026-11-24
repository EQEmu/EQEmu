"""Compare two EQMAP001 captures and locate the first difference."""

import argparse
import os
import struct

MAGIC = b"EQMAP001"
HEADER = struct.Struct("<8sIII64s")
INVALID, EMPTY, GEOMETRY = range(3)


def read_header(stream):
    header = stream.read(HEADER.size)
    if len(header) != HEADER.size:
        raise ValueError(f"{stream.name}: truncated header")
    magic, verdict, vertices, indices, identity = HEADER.unpack(header)
    if magic != MAGIC or verdict not in (INVALID, EMPTY, GEOMETRY):
        raise ValueError(f"{stream.name}: unsupported format or verdict")
    if any(char not in b"0123456789abcdef" for char in identity):
        raise ValueError(f"{stream.name}: invalid input SHA-256")
    if (
        vertices > 8_000_000
        or indices > 12_000_000
        or indices % 3
        or (verdict == INVALID and vertices)
        or (verdict == GEOMETRY) != bool(indices)
        or (indices and not vertices)
        or os.fstat(stream.fileno()).st_size != HEADER.size + vertices * 12 + indices * 4
    ):
        raise ValueError(f"{stream.name}: inconsistent verdict, counts or length")
    return identity, verdict, vertices, indices


def compare_words(reference, candidate, words, vertex_words):
    # Keep equal files fast and memory bounded even for stock large maps.
    for offset in range(0, words, 16384):
        size = 4 * min(16384, words - offset)
        expected = reference.read(size)
        actual = candidate.read(size)
        if len(expected) != size or len(actual) != size:
            raise ValueError("capture truncated during comparison")
        if expected == actual:
            continue
        for word, ((left,), (right,)) in enumerate(
            zip(struct.iter_unpack("<I", expected), struct.iter_unpack("<I", actual)), offset
        ):
            if left == right:
                continue
            if word < vertex_words:
                location = f"vertex {word // 3} axis {'xyz'[word % 3]}"
            else:
                index = word - vertex_words
                location = f"triangle {index // 3} corner {index % 3}"
            raise ValueError(f"{candidate.name}: {location}: 0x{left:08x} != 0x{right:08x}")


def compare(reference, candidate):
    with open(reference, "rb") as left, open(candidate, "rb") as right:
        expected = read_header(left)
        actual = read_header(right)
        for label, left_value, right_value in zip(
            ("input SHA-256", "verdict", "vertex count", "index count"), expected, actual
        ):
            if left_value != right_value:
                raise ValueError(f"{candidate}: {label} mismatch: {left_value} != {right_value}")
        vertex_words = expected[2] * 3
        compare_words(left, right, vertex_words + expected[3], vertex_words)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("reference")
    parser.add_argument("candidate")
    args = parser.parse_args()
    try:
        compare(args.reference, args.candidate)
    except (OSError, ValueError) as error:
        parser.exit(1, f"{error}\n")
    print("Exact decoder match")


if __name__ == "__main__":
    main()
