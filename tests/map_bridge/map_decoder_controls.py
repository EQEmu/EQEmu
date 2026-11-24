"""Generate small stock-format maps and verify the Release oracle and comparator."""

import argparse
import hashlib
import json
import struct
import subprocess
import sys
import tempfile
import zlib
from pathlib import Path

from compare_map_decoder import EMPTY, GEOMETRY, HEADER, INVALID, MAGIC

V1 = 0x01000000
V2 = 0x02000000


def v1_map(*faces):
    header = struct.pack("<IIHI", V1, len(faces), 0, 0)
    return header + b"".join(struct.pack("<13f", *face, 0, 0, 1, 0) for face in faces)


def v2_map(payload):
    compressed = zlib.compress(payload)
    return struct.pack("<III", V2, len(compressed), len(payload)) + compressed


def fixtures():
    face_a = (-0.0, 1, 2, 4, 5, 6, 7, 8, 9)
    face_b = (7, 8, 9, 4, 5, 6, 10, 11, 12)
    v1_ordered = v1_map(face_a, face_b)
    yield "v1-ordered", v1_ordered, GEOMETRY, face_a + face_b, tuple(range(6))
    yield "v1-empty", v1_map(), EMPTY, (), ()
    yield "v1-truncated-face", v1_ordered[:-1], INVALID, (), ()
    yield "v1-nonfinite", v1_map((float("inf"), *face_a[1:])), INVALID, (), ()

    empty_header = struct.pack("<9If", *([0] * 9), 1)
    yield "v2-empty", v2_map(empty_header), EMPTY, (), ()
    vertex_only = struct.pack("<9If3f", 1, *([0] * 8), 1, 1, 2, 3)
    yield "v2-vertex-only", v2_map(vertex_only), EMPTY, (1, 2, 3), ()

    header = struct.pack("<9If", 4, 6, 3, 3, 1, 1, 1, 2, 1, 2)
    base_vertices = (1, -0.0, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12)
    base_indices = (2, 0, 1, 3, 0, 2)
    base = struct.pack("<12f6I", *base_vertices, *base_indices)
    noncollidable = struct.pack("<9f3I", 90, 91, 92, 90, 93, 92, 94, 91, 92, 0, 1, 2)
    model = b"m\0" + struct.pack("<II9f", 3, 2, 1, 2, 3, 4, 5, 6, 7, 8, 9)
    model += struct.pack("<3IB3IB", 2, 0, 1, 1, 1, 0, 2, 0)
    placement = b"m\0" + struct.pack("<9f", 10, 20, 30, 0, 0, 0, 2, 3, 4)
    group = struct.pack("<12fI", 1, 2, 3, 0, 0, 0, 1, 1, 1, 10, 20, 30, 1)
    group += b"m\0" + struct.pack("<9f", 2, 3, 4, 0, 0, 0, 1, 1, 1)
    flat_tile = struct.pack("<B3f", 1, 20, 10, 3)
    sloped_tile = struct.pack("<B2fB4f", 0, 30, 40, 0, 4, 5, 6, 7)
    expansion = noncollidable + model + placement + group + flat_tile + sloped_tile

    # Expected expansion in file order, including the placement x/y swap,
    # skipped hidden polygon and both terrain windings.
    vertices = base_vertices + (
        44, 24, 66, 26, 12, 42, 35, 18, 54,
        33, 20, 46, 27, 14, 40, 30, 17, 43,
        20, 10, 3, 22, 10, 3, 22, 12, 3, 20, 12, 3,
        30, 40, 4, 32, 40, 6, 32, 42, 7, 30, 42, 5,
    )
    indices = base_indices + (4, 5, 6, 7, 8, 9, 13, 11, 12, 13, 10, 11, 17, 15, 16, 17, 14, 15)
    yield "v2-expanded", v2_map(header + base + expansion), GEOMETRY, vertices, indices
    bad_base = base[:-4] + struct.pack("<I", 4)
    yield "v2-bad-index", v2_map(header + bad_base + expansion), INVALID, (), ()
    yield "v2-bad-zlib", struct.pack("<III", V2, 4, len(empty_header)) + b"\0" * 4, INVALID, (), ()
    yield "unknown-version", struct.pack("<I", 0x03000000), INVALID, (), ()
    yield "short-version", b"\0\0", INVALID, (), ()


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def require_mismatch(comparator, reference, candidate, data, diagnostic):
    candidate.write_bytes(data)
    result = subprocess.run([*comparator, reference, candidate], capture_output=True, text=True)
    if result.returncode != 1 or diagnostic not in result.stderr:
        raise ValueError(f"{candidate.name}: expected comparator exit 1 and {diagnostic!r}, got {result}")
    print(result.stderr.strip())


def run_controls(oracle, output):
    comparator = [sys.executable, "-B", str(Path(__file__).with_name("compare_map_decoder.py"))]
    manifest = {}
    for name, data, verdict, vertices, indices in fixtures():
        path = output / f"{name}.map"
        path.write_bytes(data)
        identity = sha256(data)
        expected = (
            HEADER.pack(MAGIC, verdict, len(vertices) // 3, len(indices), identity.encode("ascii"))
            + struct.pack(f"<{len(vertices)}f", *vertices)
            + struct.pack(f"<{len(indices)}I", *indices)
        )

        first, second = (output / f"{name}.{repeat}.oracle" for repeat in (1, 2))
        for capture in (first, second):
            subprocess.run([oracle, path, capture, identity], check=True)
        subprocess.run([*comparator, first, second], check=True, stdout=subprocess.DEVNULL)
        if first.read_bytes() != expected:
            raise ValueError(f"{name}: capture differs from fixture's expected ordered bytes")

        manifest[name] = {
            "input_sha256": identity,
            "output_sha256": sha256(expected),
            "verdict": verdict,
            "vertices": len(vertices) // 3,
            "indices": len(indices),
        }
        print(f"{name}: both captures match expected ordered bytes")

    reference = output / "v2-expanded.1.oracle"
    original = reference.read_bytes()
    vertex_count = manifest["v2-expanded"]["vertices"]
    controls = {
        "coordinate": (HEADER.size + 4, 0, "vertex 0 axis y: 0x80000000 != 0x00000000"),
        "index": (HEADER.size + 12 * vertex_count, 1, "triangle 0 corner 0: 0x00000002 != 0x00000001"),
    }
    for name, (offset, word, diagnostic) in controls.items():
        changed = bytearray(original)
        struct.pack_into("<I", changed, offset, word)
        require_mismatch(comparator, reference, output / f"changed-{name}.oracle", changed, diagnostic)
    require_mismatch(
        comparator, reference, output / "truncated.oracle", original[:-1],
        "inconsistent verdict, counts or length",
    )

    # Keep the same input identity when testing a changed verdict.
    reference = output / "v1-empty.1.oracle"
    changed = bytearray(reference.read_bytes())
    struct.pack_into("<I", changed, len(MAGIC), INVALID)
    require_mismatch(comparator, reference, output / "changed-verdict.oracle", changed, "verdict mismatch: 1 != 0")
    require_mismatch(
        comparator, reference, output / "changed-identity.oracle",
        (output / "v2-empty.1.oracle").read_bytes(), "input SHA-256 mismatch",
    )
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("oracle", type=Path)
    parser.add_argument("--output", type=Path, help="retain artifacts in a new directory")
    args = parser.parse_args()
    oracle = args.oracle.resolve()
    if args.output:
        args.output.mkdir()
        run_controls(oracle, args.output)
    else:
        with tempfile.TemporaryDirectory(prefix="map-decoder-controls-") as directory:
            run_controls(oracle, Path(directory))


if __name__ == "__main__":
    main()
