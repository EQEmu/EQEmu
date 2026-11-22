"""Worker-only acceptance of the actual zone entry point. Never run on the host."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--zone", type=Path, required=True)
    args = parser.parse_args()
    zone = args.zone.resolve(strict=True)
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

        for command in ("convert_map", "CoNvErT_mAp"):
            result = run([command, "file.map"])
            assert result.returncode == 1, result
            assert result.stderr == "convert_map is retired: stock .map files load directly; no conversion is needed.\n", result
            assert result.stdout == "", result
    print("native convert_map controls passed")


if __name__ == "__main__":
    main()
