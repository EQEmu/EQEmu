"""Worker-only acceptance of the actual zone entry point. Never run on the host."""
import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--zone", type=Path, required=True)
    parser.add_argument("--mode", choices=["rust", "cpp-no-mmf", "cpp-mmf"], required=True)
    parser.add_argument("--fixture", type=Path, help="Valid stock map, required for cpp-mmf")
    args = parser.parse_args()
    zone = args.zone.resolve(strict=True)
    if args.mode == "cpp-mmf" and not args.fixture:
        parser.error("cpp-mmf requires --fixture")
    fixture = args.fixture.resolve(strict=True) if args.fixture else None
    env = {key: value for key, value in os.environ.items() if not key.startswith("EQEMU_INSERVER_TEST")}
    with tempfile.TemporaryDirectory(prefix="convert-map-native-") as directory:
        root = Path(directory)

        def run(arguments):
            result = subprocess.run([str(zone), *arguments], cwd=root, env=env,
                                    text=True, capture_output=True, timeout=15)
            print(json.dumps({"args": arguments, "status": result.returncode,
                              "stdout": result.stdout, "stderr": result.stderr}))
            return result

        # No config: malformed invocations must stop at usage in every build.
        for arguments in (["convert_map"], ["convert_map", "a.map", "extra"], ["convert_map", ""]):
            result = run(arguments)
            assert result.returncode == 1, result
            assert result.stderr == "Usage: zone convert_map <filename.map>\n", result
            assert result.stdout == "", result

        # Negative dispatch controls: ordinary and sidecar arguments still follow
        # startup and fail for absent config, rather than convert_map usage/refusal.
        # Worker must have no config in the temporary directory's parent either.
        for arguments in ([], ["qeynos2", "launcher", "0"], ["sidecar:serve-http"]):
            result = run(arguments)
            assert result.returncode == 1, result
            assert "convert_map" not in result.stderr + result.stdout, result
            if not arguments or arguments[0] == "qeynos2":
                assert "Loading server configuration" in result.stdout + result.stderr, result

        if args.mode != "cpp-mmf":
            expected = "stock .map files load directly" if args.mode == "rust" else "no native MMF support"
            for command in ("convert_map", "CoNvErT_mAp"):
                result = run([command, "file.map"])
                assert result.returncode == 1, result
                assert expected in result.stderr, result
                assert result.stdout == "", result
        else:
            (root / "maps").mkdir()
            (root / "eqemu_config.json").write_text(json.dumps({"server": {"directories": {"maps": "maps/"}}}))
            shutil.copyfile(fixture, root / "maps/valid.map")
            result = run(["convert_map", "valid.map"])
            assert result.returncode == 0, result
            assert "conversion succeeded" in result.stdout, result
            cache = root / "maps/valid.mmf"
            assert cache.is_file() and cache.stat().st_size > 16, list((root / "maps").iterdir())
            # Force overwrite must replace an existing invalid cache.
            cache.write_bytes(b"invalid cache")
            result = run(["CoNvErT_mAp", "valid.map"])
            assert result.returncode == 0 and cache.stat().st_size > 16, result
            (root / "maps/invalid.map").write_bytes(b"bad!")
            for name in ("invalid.map", "missing.map"):
                result = run(["convert_map", name])
                assert result.returncode == 1, result
                assert "conversion failed" in result.stdout, result
                assert not (root / "maps" / name.replace(".map", ".mmf")).exists()
    print("native convert_map controls passed")


if __name__ == "__main__":
    main()
