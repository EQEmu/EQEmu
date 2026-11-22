# convert_map controls

Host-safe controls execute the real Perl wrapper with a harmless fake `zone` in
an isolated temporary directory. They check success, nonzero exits, missing binary,
POSIX signals, empty input, existing filename selection, and literal filenames
containing spaces, quotes and shell metacharacters. No native candidate runs.

```sh
perl -c utils/scripts/convert_maps_to_mmfs.pl
python3 tests/convert_map/batch_controls.py
```

The wrapper still selects lowercase `.map` names excluding `_lit` from `./maps`,
and passes basenames to `zone`. The native converter still uses configured MapDir
and lowercases the basename. Run the wrapper from the server directory with those
paths aligned. It uses `./zone` on Linux/FreeBSD and `zone.exe` on Windows. Child
stdout/stderr stream directly; a failed invocation or conversion makes the final
batch status nonzero while other selected maps are still attempted. Rust builds
load stock `.map` files directly and reject this legacy conversion command.

## Stack-only native acceptance

All following commands are UNEXECUTED in the implementation worktree. Run only in
the approved disposable Stack worker after parent integrates, pushes, and pins the
new exact source SHA. Retain full command output, exit codes, source/tree/binary
hashes, CMake caches and compile commands. Do not substitute old binaries or
promote prior failed source SHAs. This suite calls the real `main` and `Map::Load`;
it has no C++ stubs. Host fake-process passes do not qualify native dispatch.

Use matched dependencies and the existing worker flags in separate build dirs.
Keep Rust enabled by its default for the first build, with no `USE_MAP_MMFS` in
any translation unit. Build explicit OFF twice: without MMFs and with
`-DUSE_MAP_MMFS` consistently applied to all native translation units. There is
no new CMake MMF option. For example, append to the approved configuration:

```sh
cmake -S /source -B /output/convert-default -DCMAKE_TOOLCHAIN_FILE="$MATCHED_TOOLCHAIN" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build /output/convert-default -j1 --target zone
cmake -S /source -B /output/convert-off -DCMAKE_TOOLCHAIN_FILE="$MATCHED_TOOLCHAIN" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DEQEMU_RUST_MAP_RAYCASTER=OFF
cmake --build /output/convert-off -j1 --target zone
cmake -S /source -B /output/convert-off-mmf -DCMAKE_TOOLCHAIN_FILE="$MATCHED_TOOLCHAIN" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DEQEMU_RUST_MAP_RAYCASTER=OFF -DCMAKE_CXX_FLAGS=-DUSE_MAP_MMFS
cmake --build /output/convert-off-mmf -j1 --target zone
```

Preserve any required existing CXX flags when adding the MMF definition. Inspect
actual zone/main.cpp and zone/map.cpp compile commands for all three builds. Do
not silently change the default build to support MMFs. A fourth Rust+MMF build
can check that Rust refusal wins over the MMF macro, if that variant is already
part of the worker acceptance matrix.

Run from a worker with no network/DB/world access. The temporary directory and its
parent must contain no server configuration. Ordinary/sidecar negative dispatch
controls deliberately reach configuration loading and stop there. The supported
MMF controls write only a minimal local config, using the supplied valid stock map
copied into their temporary maps directory. No candidate assets are modified.

```sh
python3 /source/tests/convert_map/native_controls.py --zone /output/convert-default/bin/zone --mode rust
python3 /source/tests/convert_map/native_controls.py --zone /output/convert-off/bin/zone --mode cpp-no-mmf
python3 /source/tests/convert_map/native_controls.py --zone /output/convert-off-mmf/bin/zone --mode cpp-mmf --fixture /corpus/qeynos2.map
```

Require usage+status 1 for missing/extra/empty filename in every variant; ordinary
and sidecar commands must retain normal startup dispatch. Default Rust and non-MMF
C++ must refuse `zone convert_map file.map` with status 1 and the expected stderr,
without config/startup output. Mixed-case command spelling remains accepted.
Legacy MMF must return 0 and create a nonempty native cache from the valid map,
replace an invalid existing cache, and return 1 with no cache for invalid or
missing maps. Native controls have a 15-second per-process timeout, and record
stdout/stderr/status per invocation. Missing config, failures or timeouts are
failures, not grounds to silently substitute a stub or another build.

These focused checks supplement the parent's full default/OFF builds, Cargo,
map/caller/reload acceptance. They do not establish those broader gates or native
Windows behavior.
