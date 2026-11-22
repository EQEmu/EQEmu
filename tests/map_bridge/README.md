# Map backend acceptance

Build and execute candidate code only in the existing Stack owner's disposable
workers. These standalone tests do not expand the default utility suite or need
production assets, a database or a running server. Native Linux x86-64, repository
Rust 1.99.0, CXX 1.0.194 and frozen/offline Cargo are required. The legacy C++
backend, OFF matrix and duplicate MMF wrapper variant are retired.

## Worker commands

Parent pins the pushed head as `EXPECTED_SHA`; Stack checks a clean exact source
checkout and records source/tree/toolchain/dependency hashes, full command output
and exit status. The following is an executable worker handoff, not a claim that
it ran. Supply the matched toolchain, GLM/FMT roots and existing feature arguments.
Cargo output must be external to `/source`. Provision committed lockfiles and the
GNU target before entering offline validation. Reuse accepted frozen inputs and
Rust outputs where valid; never substitute an old executable for this head.

```sh
set -eu
cd /source
: "${EXPECTED_SHA:?pin the pushed Server commit}"
: "${MATCHED_TOOLCHAIN:?supply the matched worker toolchain}"
: "${CARGO_TARGET_DIR:?supply external worker Cargo output}"
test "$(git rev-parse HEAD)" = "$EXPECTED_SHA"
test -z "$(git status --porcelain)"
git show -s --format='%H %T %P'
cmake -S /source -B /output/server -DCMAKE_TOOLCHAIN_FILE="$MATCHED_TOOLCHAIN" -DEQEMU_BUILD_TESTS=ON -DEQEMU_BUILD_LOGIN=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build /output/server -j1
/output/server/bin/tests
cmake -S /source/tests/map_bridge -B /output/focused -DCMAKE_BUILD_TYPE=Release
cmake --build /output/focused -j1
ctest --test-dir /output/focused --output-on-failure
cargo test --manifest-path /source/rust/raycaster/Cargo.toml --frozen --offline --target x86_64-unknown-linux-gnu
cargo fmt --manifest-path /source/rust/raycaster/Cargo.toml --check
cargo clippy --manifest-path /source/rust/raycaster/Cargo.toml --frozen --offline --target x86_64-unknown-linux-gnu -- -D warnings
cargo test --manifest-path /source/rust/actors/Cargo.toml --workspace --frozen --offline --target x86_64-unknown-linux-gnu
cargo fmt --manifest-path /source/rust/actors/Cargo.toml --all --check
cargo clippy --manifest-path /source/rust/actors/Cargo.toml --workspace --all-targets --frozen --offline --target x86_64-unknown-linux-gnu -- -D warnings
python3 /source/tests/convert_map/native_controls.py --zone /output/server/bin/zone
```

Use the existing normal full-build features and resource limits in addition to
the minimal command above. Record cache, compile commands and link/archive closure:
`rust_raycast_mesh.cpp` must compile and link; `raycast_mesh.cpp` must not. No
backend/MMF compile definition or selection is needed. Reject old CMake cache
entries rather than treating an ignored retired switch as a supported variant.
Require all ordinary targets, 95 utility tests, 10 Rust tests and 43 actor tests
plus doc tests, formatting and Clippy. Hosted CI status is separate and must be
reported explicitly; equivalent local checks cannot establish hosted success.

Bridge tests call the real Rust opaque engine and cover copies, normals, errors,
misses, optional outputs and destruction. Geometry tests call the production C++
V1/V2 decoder with synthetic data, including model/group/terrain bounds. Wrapper
checks compile the real Map/decoder/adapter with explicit Zone/Config/log/rule/path
stubs. They cover valid empty versus invalid load, transactional replacement,
query outputs, normals and stale `.mmf` caches being ignored without rewriting.
They do not instantiate a production Zone. `reload_ownership` tests production
`ReplaceCollisionMap` ownership ordering with a fake Map. Observation controls
check opt-in, NPC filtering, nested context, identity and budget, not gameplay.

## Production acceptance still required

Use the sole existing Stack owner, not a new worker launcher in Server. Compare
new-head Qeynos2 AND PoK production ground, clear/blocked LoS and ordinary NPC
callers against accepted PR118 receipts. Require matching endpoints/results,
ground/model offsets, route admission, adjusted nodes, dispatch, FixZ and final
pose. Exercise real Zone failed reload after missing file and invalid header,
preserved last-valid query, changed-height replacement and recovery with
post-attempt timestamps. Stub passes cannot replace production reload acceptance.

Reuse the three frozen large inputs and require admission without triangle loss,
including 6,161,009 vertices and 6,684,999 indices. Recheck the retained 16,512
bit-exact comparisons against accepted Rust outputs where feasible, without
building C++. The prior 513-map corpus and soak remain historical evidence only.
No live staging, new soak, full client shutdown or sanitizer acceptance is claimed.

PR118 head `4534b5b372a611a3324805cee8b326912a89f675` merged as
`86c25b8d37fe9c73940b10c9c9a2098c73b776a9`, tree
`49e7459d4d545ead382b917bce0cab074555635d`. Accepted qualification is external:
`~/.local/state/eqemu-map-translation/pr118-beb1068-20261121-v1/final-4534b5b3-attempt2/FINAL-QUALIFICATION-HANDOFF.json`,
SHA256 `a3d1ba4a3c29b6da1541059e8ee40c390bcb7788e10ea3bf7cfdc96ae82df91c`.
Final index SHA256 is
`8cf0be022c1566cefdb444f7fdd600f4f6b3a0376566ce359ded2db6eef02ed0`.
Old failures and parent branches remain evidence; their passes do not qualify a
new head. Historical transport/scenario sources remain available in Git at
`3c2f12b8fc8e12cdc9cca7e0ab753fd22a8ad597:tests/map_server_validation/`,
`tests/map_reload/` at that revision, and
`6a037f008f9398df5967fa70ded0de32a9e83335:tests/server_collision_ab/`.

`EQEMU_COLLISION_OBSERVE=server-collision-npc-2011` retains bounded thread-local
NPC 2011 observations. The accepted Rust label `backend=ON` is preserved for
receipt compatibility. Exclusive zone-thread engine ownership is still required.
The observation controls do not make concurrent Map queries safe.

## Retired-code audit

The real production query consumer was `Map::impl::rm` in `zone/map.cpp`. No
other production consumer of the legacy factories was found. Removed:
`zone/raycast_mesh.cpp`, its build source, C++ factory declarations, conditional
backend selection, MMF serialization/read/write/cache helpers and signature,
`utils/scripts/convert_maps_to_mmfs.pl` and its batch controls. `convert_map`
retains only an early explicit refusal and useful dispatch regression checks.
Git history preserves the removed implementation and prior ON/OFF evidence.

Retained: the virtual query interface used by the Rust adapter, C++ V1/V2
model/group/terrain decoder and checked scalar loads, shared `common/memory/ksm.*`,
Map public queries, Rust engine arithmetic/ties/normals/endpoints, owned CXX
lifetimes and transactional replacement. No engine math or decoder bounds change.
Native Linux x86-64 is Brian's selected maintained Server platform. Windows CI
and preset are retired; other platforms and cross builds fail configuration.
