"""Run only inside the approved disposable worker, against its compiled control."""
import os
import subprocess
import sys

binary = sys.argv[1]
backend = sys.argv[2]
for value in (None, "wrong-fixture", "server-collision-npc-2011"):
    env = dict(os.environ)
    env.pop("EQEMU_COLLISION_OBSERVE", None)
    if value is not None:
        env["EQEMU_COLLISION_OBSERVE"] = value
    lines = subprocess.check_output([binary], env=env, text=True).splitlines()
    if value != "server-collision-npc-2011":
        assert not lines, lines[:3]
        continue
    assert all(f"backend={backend}" in line for line in lines)
    assert len(lines) == 4096, len(lines)
    assert "caller=outer entity=123" in lines[0] and "kind=first" in lines[0]
    assert "caller=outer entity=123" in lines[1] and "kind=restored" in lines[1]
    assert "caller=nested entity=789" in lines[2]
    assert all("rejected" not in line and "unscoped" not in line and "after_scope" not in line for line in lines)
    assert lines[-1].endswith("remaining=0")
print("observation disabled/enabled/rejection/context/budget controls passed")
