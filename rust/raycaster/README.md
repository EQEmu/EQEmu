# Rust map engine

Zone builds require the Rust map engine on native Linux x86-64. Other platforms
and cross builds fail configuration with an explicit diagnostic. The legacy C++
query engine and build selection are retired. There is no runtime switch or
automatic fallback after Rust errors. The actor bridge retains its independent
platform checks.

C++ retains V1/V2 model/group/terrain decoding and expansion. The maintained
crate owns geometry copied during a CXX call, an opaque boxed engine, and query
stamps. It depends on no experiment files or geometry SHA table. Queries need
exclusive engine access even though Map's public methods are const. Zone-thread
ownership is required; concurrent access has not been qualified. C++ release
runs the Rust box destructor exactly once.

CXX Result errors become C++ exceptions. Construction catches standard exceptions,
logs a load diagnostic, and retains the previous Map engine. Invalid query
coordinates propagate an exception; they do not become an unobstructed ray.
A Rust panic crossing CXX aborts. OOM may still abort in Box/CXX/runtime or native
allocations; recoverability is not guaranteed for all allocation failures.
No unsafe Rust is used in the core. Generated bridge code uses CXX's unsafe
machinery. Memory safety does not establish gameplay safety.

Empty valid geometry creates a loaded engine with absent bounds and no hits.
Invalid/nonfinite vertices fail, including unused vertices. Coordinates and
query endpoints must be finite and within +/-1e8. No triangles are dropped.
A complete replacement is built before Map swaps ownership. Map always rebuilds
from decoded stock `.map` files. It neither consumes nor
serializes engine caches; stale `.mmf` files are ignored. `zone convert_map`
refuses before startup and the batch conversion script is retired.
Map sentinels, optional outputs, start-position adjustments and rule/zone limits
remain in C++. Zone-dependent height checks require a hit and a present zone.

Admission is 8,000,000 vertices and 12,000,000 indices, including count multiples,
index validation, and finite checks. At most 144 MB of owned geometry, 16 MB of
visited stamps, 131071 nodes and 16 million leaf references are admitted. Counts
fit u32 and arithmetic fits usize on the required 64-bit target. Tree depth is
15, with bounded queries and stamp-wrap reset. Vec growth uses fallible reserve.
This is a count budget, not a strict RSS cap: decoder arrays, adapter copies,
allocator capacity and recursive partition lists add transient memory. The
largest historical mesh has 6,161,009 vertices and 6,684,999 indices; production
admission does not authenticate identities. Tests authenticate frozen assets.

Cargo builds frozen/offline with pinned CXX 1.0.194 and the repository Rust
1.99 toolchain. Supply offline dependencies in the disposable worker. CMake
requires external Cargo output; the worker owner cleans it. Sanitizer builds
instrument C++ callers and generated CXX through a separate Cargo target path.
Stable Rust code remains uninstrumented. See tests/map_bridge for focused tests
and retained qualification receipts for actual coverage.

The shared C++ decoder rejects truncated packed reads, unterminated names,
nonfinite scalars, invalid triangle/model indices and unknown placement models.
Packed and inflated V2 buffers are each limited to 512 MiB, and terrain dimensions
to 2048 quads per tile axis. Expanded output uses the geometry count limits above.
These new parser limits need exact-head large-asset admission checks before merge;
historical tests do not validate this revision. Decoder failures return false and
the Map loader logs failure. No malformed geometry becomes a valid empty engine.

See `tests/map_bridge/README.md` for disposable-only commands, caller observation
controls and the retired-code audit. Full exact-head acceptance remains a
parent/Stack gate; the historical corpus and soak are bounded prior evidence.
