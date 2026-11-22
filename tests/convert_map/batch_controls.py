"""Harmless host controls: run the real Perl wrapper against a fake zone only."""
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

WRAPPER = Path(__file__).resolve().parents[2] / "utils/scripts/convert_maps_to_mmfs.pl"
FAKE_ZONE = '''#!/usr/bin/env python3
import json, os, signal, sys
with open("calls.jsonl", "a") as out:
    out.write(json.dumps(sys.argv[1:]) + "\\n")
name = sys.argv[2]
if name == "signal.map":
    os.kill(os.getpid(), signal.SIGTERM)
if name == "invalid.map":
    print("fixture rejected", file=sys.stderr)
    sys.exit(7)
print("fixture converted")
'''


class BatchControls(unittest.TestCase):
    def run_batch(self, names, binary=True):
        with tempfile.TemporaryDirectory(prefix="convert-map-batch-") as directory:
            root = Path(directory)
            (root / "maps").mkdir()
            for name in names:
                (root / "maps" / name).touch()
            if binary:
                (root / "zone").write_text(FAKE_ZONE)
                (root / "zone").chmod(0o700)
            result = subprocess.run(
                ["perl", str(WRAPPER)], cwd=root, text=True,
                capture_output=True, timeout=15,
            )
            calls = root / "calls.jsonl"
            arguments = [json.loads(line) for line in calls.read_text().splitlines()] if calls.exists() else []
            self.assertFalse((root / "INJECTED.map").exists())
            return result, arguments

    def test_success_and_literal_arguments(self):
        names = ["plain.map", "space name.map", "semi;touch INJECTED.map", "$(touch INJECTED.map).map", "quote'\"&*.map"]
        result, calls = self.run_batch(names + ["skip_lit.map", "skip.MAP", "skip.txt"])
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertCountEqual(calls, [["convert_map", name] for name in names])
        self.assertIn("Batch processing complete", result.stdout)

    def test_failure_still_processes_other_maps(self):
        result, calls = self.run_batch(["invalid.map", "valid.map"])
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(len(calls), 2)
        self.assertIn("fixture rejected", result.stderr)
        self.assertIn("exit status 7", result.stderr)
        self.assertIn("Batch processing failed", result.stdout)

    def test_missing_binary(self):
        result, calls = self.run_batch(["valid.map"], binary=False)
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(calls, [])
        self.assertIn("Can't execute ./zone", result.stderr)

    @unittest.skipUnless(os.name == "posix", "POSIX wait status control")
    def test_signal(self):
        result, calls = self.run_batch(["signal.map", "valid.map"])
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(len(calls), 2)
        self.assertIn("terminated by signal 15", result.stderr)

    def test_empty_batch(self):
        result, calls = self.run_batch([])
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(calls, [])


if __name__ == "__main__":
    unittest.main()
