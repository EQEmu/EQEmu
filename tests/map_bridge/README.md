# Map backend acceptance

This standalone suite has no assets, database or running server dependency.
Build and run it only in the Stack owner's approved disposable worker. No
candidate executable, compiler, Cargo, rustfmt or Clippy was run on the host.
The implementation handoff's executed checks are static `git diff --check` and
source inspection only. All commands below remain unexecuted on this head.

With source mounted at `/source`, external output at `/output`, offline Cargo
provisioned from the repository toolchain and lockfile, and GLM/FMT include roots
provided by the worker's matched dependencies:

```sh
cmake -S /source/tests/map_bridge -B /output/focused-default -DCMAKE_BUILD_TYPE=Release
cmake --build /output/focused-default -j1
ctest --test-dir /output/focused-default --output-on-failure
cmake -S /source/tests/map_bridge -B /output/focused-off -DCMAKE_BUILD_TYPE=Release -DEQEMU_RUST_MAP_RAYCASTER=OFF
cmake --build /output/focused-off -j1
ctest --test-dir /output/focused-off --output-on-failure
cargo test --manifest-path /source/rust/raycaster/Cargo.toml --frozen --offline --target x86_64-unknown-linux-gnu
cargo fmt --manifest-path /source/rust/raycaster/Cargo.toml --check
cargo clippy --manifest-path /source/rust/raycaster/Cargo.toml --frozen --offline --target x86_64-unknown-linux-gnu -- -D warnings
```

The bridge tests exercise the real opaque engine, copies, normals, errors, misses,
optional outputs and repeated destruction. Geometry tests call the production C++
decoder with compressed synthetic V2 files. Wrapper tests compile the real Map and
decoder with explicit Zone/Config/log/rule/path stubs. They cover empty/invalid
loads, transactional Map replacement, query outputs, normals and MMF bypass.
They do not instantiate a production Zone. The reload ownership test calls the
same production `ReplaceCollisionMap` helper with a destruction-counting fake Map.
It establishes ownership ordering, not production Zone reload acceptance.
Observation controls test disabled/wrong opt-in output, rejected NPCs, nested
context restoration, entity identity, and exhaustion of the 4096-line budget.

## Exact-head integration handoff

Parent must record `git rev-parse HEAD` matching the pushed candidate before
building. Use the existing Stack owner, matched vcpkg/dependencies and rootless
worker workflow. No new runtime or staging deployment is authorized here.
Historical worker scripts are external inputs, not default tests:

- `239480beac57c78770a209b8fb4f3a2b79fd68b3:tests/map_bridge/guest.py` documents
  the focused historical environment. Do not run its old checkout as the candidate.
- `3c2f12b8fc8e12cdc9cca7e0ab753fd22a8ad597:tests/map_server_validation/`
  and `tests/map_reload/` contain the matched dependency/full-target/reload workflow.
- `6a037f008f9398df5967fa70ded0de32a9e83335:tests/server_collision_ab/`
  contains the Qeynos2 and PoK routes, scenario, evidence comparator and controls.
  Stack may adapt pinned transport outside Git; never substitute historical binaries.
- Retained roots are `~/.local/state/eqemu-map-translation/` under
  `rust-map-server-validation-20261111`, `zone-map-reload-20261111`, and
  `server-collision-npc-completion-20261115`.

The owner supplies the matched toolchain file and existing feature arguments:

```sh
cmake -S /source -B /output/server-default -DCMAKE_TOOLCHAIN_FILE="$MATCHED_TOOLCHAIN" -DEQEMU_BUILD_TESTS=ON -DEQEMU_BUILD_LOGIN=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build /output/server-default -j1
/output/server-default/bin/tests
cmake -S /source -B /output/server-off -DCMAKE_TOOLCHAIN_FILE="$MATCHED_TOOLCHAIN" -DEQEMU_BUILD_TESTS=ON -DEQEMU_BUILD_LOGIN=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DEQEMU_RUST_MAP_RAYCASTER=OFF
cmake --build /output/server-off -j1
/output/server-off/bin/tests
```

Record actual CMake caches showing ordinary default ON and explicit OFF, zone
compile commands showing the selected definition, link command/archive closure,
source and binary hashes, toolchain, flags and resource limits. Full ordinary
targets, utility tests, focused tests and the actor bridge must all pass. Configuration
text alone does not establish the default. Prior workers used 4 GiB/no swap and
one build job, then 2 GiB/no swap runtime. Do not weaken host policy for rootless
setup. New parser limits must admit all three historical large meshes, including
6,161,009 vertices and 6,684,999 indices, without dropping triangles.

Run matched default/OFF/OFF-repeat fixtures in Qeynos2 AND PoK with fixed ground,
clear and blocked LoS, and ordinary NPC caller controls. Require exact native
endpoints/results, ground/model offset, route admission, adjusted nodes, dispatch,
FixZ and final pose comparison. Include failed Zone reload after missing file and
invalid header, preserved last valid query, valid changed-height replacement and
recovery with post-attempt timestamps. A stub helper pass cannot replace this.

`EQEMU_COLLISION_OBSERVE=server-collision-npc-2011` enables the same binary's
bounded observation of NPC type 2011 only. Unset it to disable. RAII scopes borrow
string literal caller names and restore the previous entity/context, including
nested rejected NPCs. Context and the 4096-line budget are thread-local. The actual
engine still requires exclusive zone-thread access; this does not make Map safe
for concurrent queries. Logs identify backend ON/OFF. Hooks print to stdout and
never change geometry, queries, route decisions, API or persistent state. Keep the
opt-in limited to disposable fixtures. Historical scenario output is compatible.

The 513-map corpus, four data contracts, and 24-hour soak are prior bounded
evidence only. No exact-head run or new soak is claimed. Full client/world shutdown
central-866g and sanitizers remain separate/deferred; neither is fixed here.

## Legacy deletion assessment

The real production query consumer is `zone/map.cpp`, through `Map::impl::rm`.
`zone/raycast_mesh.h` declares the shared virtual interface, scalar typedefs and
legacy factories. `zone/raycast_mesh.cpp` implements the C++ BVH, normals, query
stamps, brute-force queries, native MMF construction and serialization. There is
no `common/raycast_mesh.*` in this base. No non-Map production caller of the
factory/interface was found. `common/memory/ksm.*` is shared allocation support,
not part of the query backend, and stays.

After the default landing qualifies, remove `zone/raycast_mesh.cpp` and its
`zone/CMakeLists.txt` source entry, the OFF selection in
`cmake/MapRaycaster.cmake` and `Map::InstallGeometry`, the legacy factory/MMF
declarations in `zone/raycast_mesh.h`, and `Map::LoadMMF`, `Map::SaveMMF` plus
strip/add extension helpers and `USE_MAP_MMFS` signature/cache branches in
`zone/map.cpp`/`zone/map.h`. Existing `.mmf` files need no migration because Rust
ignores them. Retain or rename the virtual interface while the Rust adapter uses
it, or simplify it separately. Retain the C++ decoder, checked scalar loads,
geometry regression tests, reload helper, ownership tests, Rust core tests and
bridge/wrapper tests. Replace the MMF bypass test with a stale-cache-is-ignored
control if the `USE_MAP_MMFS` switch disappears. Remove explicit-OFF acceptance
jobs only after archiving matched parity evidence. No backend deletion is in this PR.

Remaining blockers are exact-head ordinary build/link/default/OFF coverage,
two-zone endpoint/normal/tie/arithmetic/short-ray/endpoint parity and ordinary NPC
caller acceptance, failed reload preservation, large-asset admission with the new
bounds, and a deliberate platform decision. Removing OFF also removes the build
escape for Windows/macOS/non-x86 Linux and cross builds until Rust support is
implemented there or the fork explicitly narrows its supported targets. Linux
actors already need Cargo, but the map requirement is unconditional for normal
zone builds and fails instead of silently skipping.

Once those gates and platform disposition are accepted, delete the legacy query
engine. Upstream and Git history preserve recovery; retaining old code alone is
not a reason to keep a second backend. Keep permanent fast regressions for valid
empty versus rejected load, V1/V2 parsing/model/group/terrain, normal winding and
first-triangle ties, endpoints and tiny/zero rays, optional miss outputs, invalid
coordinates/indices, lifetime/copy ownership and failed reload. Heavy corpus and
runtime suites remain explicit standalone acceptance inputs.
