# Guide To Building From Source Without Installer

This guide is far from exhaustive, you should expect to have some experience with building C++ code before considering compiling the code from scratch.  You should instead consider using the installer scripts if you don't want to hack on the code directly.

### CMake

This fork maintains Server builds on native Linux x86-64 only. Other platforms
and cross compilation fail zone configuration. EQEmu uses CMake as the build system.  You will need CMake 3.20 or higher to build from source.

### Dependencies

The following libraries are required to build from source:
- uuid (unix)

The following libraries are not strictly required but in many cased recommended.
- [Perl](https://www.perl.org/ "Perl") (Required for Perl Quest Scripting)

##### Linux
For Linux you simply can install the dependencies from your package manager, below is an example of doing it on Ubuntu using apt-get.

    sudo apt-get install libperl-dev uuid-dev

### Running CMake

##### Linux
Configure a native Linux build:

	mkdir build
	cd build
	cmake -G "Unix Makefiles" -DEQEMU_BUILD_TESTS=ON -DEQEMU_BUILD_LOGIN=ON ..

### Narrow C++ format-warning gate

`cmake/FormatWarnings.cmake` defines the explicit policy `-Wformat
-Werror=format` for GNU, Clang and AppleClang with a GNU-style frontend.
It enables the compiler's basic printf/scanf-family format diagnostics and
makes that diagnostic group fatal, including literal format/argument type
mismatches. It does not enable `-Wall`, `-Wextra`, `-Wformat=2`, or broad
`-Werror`. Nonliteral format analysis and broader warning policies are deferred.

The policy is **PRIVATE to the zone executable's C++ compilations**, including
`zone/actor_body.cpp`, `zone/mob.cpp`, and (with tests enabled)
`zone/inserver_tests.cpp`. It also checks headers as seen by those compilations;
it is not applied globally or propagated to common, gm_commands_zone,
lua_zone, perl_zone, or vendor targets. Unsupported compilers/frontends
(including MSVC and clang-cl) retain their existing flags, without this gate's
coverage. Unannotated variadic wrappers such as `EQEmuLogSys::Out` are not
automatically checked. This is not a memory-safety guarantee.

With `EQEMU_BUILD_TESTS=ON`, every normal CMake configure compiles two tiny
static-library controls using the selected C++ compiler and the same policy;
no control executable is linked or run, and CTest is not required. The
`std::snprintf("v=%.2f", int)` control must fail with the format group's
int/double mismatch diagnostic, while the explicit-double equivalent must
compile. An unrelated failure does not satisfy the negative control. Results
are rebuilt on every configure rather than reused from a cached probe.
Compiler controls temporarily use `LC_ALL=C` so GCC's translated diagnostics
cannot invalidate the English diagnostic checks. The caller's locale setting is
restored immediately after each compile, including rejected compilations, before
result handling; production compilation does not inherit this override.
Automatic configure-time regressions also compile through this same helper with
an initially unset and a non-English `LC_ALL`, checking safe compilation,
int/double rejection and locale restoration. The non-English locale does not
need to be installed because the compiler must see `C`. These extra compile-only
probes retain output in `CMakeFiles/eqemu-format-controls/locale-*.log`.
The diagnostic matcher normalizes UTF-8 curly quotation delimiters to ASCII
before checking the format group and int/double evidence; original logs remain
unchanged. Configure also exercises this production matcher against ASCII/UTF-8
GCC and Clang diagnostic samples, successful-compilation results, and unrelated
or partial errors. These checks do not replace the compiler controls.
Configure prints both outcomes and the mismatch diagnostic; complete compiler
output is retained under `CMakeFiles/eqemu-format-controls/{safe,unsafe}.log`
in the build directory. A failed control stops configuration.

Production verification is separate from these controls: in an isolated native
build configured with `CMAKE_EXPORT_COMPILE_COMMANDS=ON`, inspect the entries
for `zone/actor_body.cpp`, `zone/mob.cpp` and `zone/inserver_tests.cpp` for both
flags, then require the default build to succeed. Also confirm that dependency
compile commands do not acquire this policy. Controls alone do not establish
production enforcement or replace the full utility/in-server suite.

Initial scope remains zone-wide, without warning suppressions. The isolated
fixture for `367c7a4b501740a0b5167539cb915d4de05cf687` completed native
configuration, then stopped during native build on one reported GCC 13
`-Werror=format-truncation` diagnostic in `EntityList::MakeNameUnique`.
Its `entity.cpp` compile command contained both policy flags. GCC's format
group includes truncation diagnostics, not just argument-type mismatches.
The subsequent fixture for `d7891a4c11a830e2ed289fc225e9151be75acf0a`
also completed native configuration but stopped on the same diagnostic: even
an explicit 61-byte prefix plus three suffix digits and NUL needs 65 bytes.
The formatter now builds the suffix separately and copies it into the remaining
space with `strn0cpy`, avoiding overlapping format arguments on all platforms.
This preserves the legacy 63-byte output limit, including truncation of the last
suffix digit at a 61-byte prefix; repairing that uniqueness limitation would be
a separate behavior change. No warning suppression is added. The automatic
in-server suite checks empty, short, 59-, 60-, 61-, 62- and 63-byte inputs,
including return-pointer identity, NUL termination and the buffer boundary.
Its boundary expectations now reflect the actual legacy truncation instead of
expecting a 65-byte result in a 64-byte buffer.

That stopped build does not establish a complete warning count, actor/in-server
compile-command verification, timing, or full-suite success. Development quick
checks cannot configure/build this CMake scope; full isolated validation must
supply those results for the repaired snapshot. If substantial unrelated zone
diagnostics appear, explicitly narrow the policy to actor/in-server translation
units and document measured findings rather than mass-cleaning or suppressing
them. Later expansion to other targets or diagnostics requires a separate
noise/coverage decision; logger annotations and analyzer/sanitizer work remain
out of scope.

### T3 callback validation jobs

From the reviewed Stack checkout containing `scripts/validation-job`, start
ordinary full validation with `scripts/validation-job start full`. Supply
`--repo` with the absolute Server checkout, `--commit` with its full committed
SHA, `--websocketpp` with an absolute initialized websocketpp checkout,
`--thread` with the exact callback destination, and `--t3-base-dir` with the
current T3 instance's base directory. The source must be clean; dirty edits are
rejected rather than captured. Native/Cargo candidate execution stays in the
disposable worker.

Start immediately returns JSON with `job_id` and `artifacts`. The optional
`--jobs-dir` precedes the subcommand and defaults to
`~/.local/state/eqemu-validation-jobs`. Run `scripts/validation-job status`,
`scripts/validation-job cancel`, or `scripts/validation-job retry-notification`
followed by that job ID, using the same `--jobs-dir` prefix when overridden.
There is no `--wait`; completion uses the callback. The trusted
`scripts/t3-validation-notify` adapter wraps the supported bundled T3 CLI.

Retained `job.json` records immutable source commit/tree and helper hashes;
`state.json` records process identities. Inspect `result.json` for the raw
validator exit and cleanup receipt or explicit unknown state,
`validator-result.json` for its final JSON,
and `validator.stdout.log` / `validator.stderr.log` for raw diagnostics.
`notification.json` reports callback delivery separately; delivery is not a
validation or cleanup result.

The sanitizer interface is `scripts/validation-job start full-sanitizers`, with
the same arguments plus `--config` selecting the exact admitted diagnostic
configuration. Stack must review, admit and activate that separate worker release
before using this command; the active release must match the selected configuration.
The wrapper's CLI/process tests do not establish sanitizer qualification.

### T3 sanitizer validation

Stack owns native/Cargo execution in the disposable worker. Configure a fresh
build there with `-DEQEMU_SANITIZERS=ON -DEQEMU_BUILD_TESTS=ON` and
`-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`. Keep its build, Cargo output and artifact
identity separate from ordinary validation. The option defaults to OFF and
requires Linux GCC or Clang with a GNU-style frontend.

The profile applies `-fsanitize=address,undefined -fno-sanitize-recover=all`
at compile and final link, plus `-g -O1 -fno-omit-frame-pointer
-fno-optimize-sibling-calls` at compile. It covers common, utility tests, selected
server/client-file executables including world/zone, and project libraries such
as gm_commands_zone, lua_zone and perl_zone, including project object libraries.
The existing source-specific `-O0` for common/patches/{sod,sof,rof,rof2,uf}.cpp
still overrides `-O1`; those five sources retain full sanitizer instrumentation.
Sanitizer builds compile each lua_zone binding source separately. Ordinary builds
retain Lua unity batches of eight; other targets keep their existing unity settings.
Separately built dependencies under libs/submodules, imported libraries and the
vendored cppunit framework are excluded. Header code compiled in an instrumented
target is covered. The private perlbind ASan option is not this profile.

Initial coverage gap: Cargo still compiles generated CXX without these flags in
`cmake/ActorBridge.cmake` and `rust/actors/bridge/build.rs`. Neither generated CXX
nor Rust code has sanitizer instrumentation from this profile. Linking their
archive into an instrumented executable does not close that gap.

Use these exact worker runtime settings and an absolute installed
`ASAN_SYMBOLIZER_PATH` supported by the selected sanitizer runtime. Stack's
pinned GCC diagnostic worker selects `/usr/bin/addr2line`; LLVM is not required
by this profile. Stack must qualify actual file/line reports from every deliberate
control before accepting that symbolizer selection:

```sh
ASAN_OPTIONS=halt_on_error=1:abort_on_error=0:detect_leaks=1:symbolize=1:exitcode=1
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1:exitcode=1
```

Build `tests` and the non-installed, opt-in `tests_sanitizer_controls` target.
Run the existing complete utility inventory and existing in-server assertions
through Stack's selected validation path. Run each control separately through
that same execution path: `pass` must exit 0; `use-after-free`, `out-of-bounds`,
`undefined` and `leak` must exit nonzero with the corresponding diagnostic and
symbolized file/line in `tests/sanitizer_controls.cpp`. Expected frames are
`SanitizerUseAfterFree`, `SanitizerOutOfBounds`, `SanitizerUndefined` and
`SanitizerLeak`. A crash without the expected report does not qualify a control.
Leak checks require orderly process exit; killing world/zone cannot qualify
their exit-time leak checks. Keep physical-memory budgets without an ASan-
incompatible virtual-address limit. No suppressions are supplied.

Stack must verify actual common/test/world/zone and project-library compile
commands, final link flags, runtime/symbolizer availability, failing controls,
and real suite results for the exact candidate SHA. Record elapsed time, peak
physical memory and retained diagnostic bytes. Configure-only checks establish
flag placement, not runtime qualification. This source combines master
`bf50825db4d61a7b45026f5335975316615454f2`, actor staging history through
`788c86269d9b53ca2b11ca8fa7183826f6c330d6`, and the selected sanitizer build
support. Actor staging scenarios remain default-off and require explicit opt-in;
the conditional Linux x86-64 actor CXX bridge remains. The map bridge is
unconditional and restricts maintained zone builds to native Linux x86-64.
Historical results qualify only their recorded revisions. This composition has
no new ordinary full-suite or sanitizer qualification result. Dedicated population
controls do not replace live population qualification; see `rust/actors/README.md`.

### Bootstrap Rust bridge (zone)

On native x86-64 Linux builds, the zone target links the bootstrap
`actor-bridge` static library from `rust/actors`. This integration targets `x86_64-unknown-linux-gnu` with the
repository's pinned Rust toolchain and locked dependencies. Other platforms
and cross builds skip the actor bridge, but the unconditional map backend
rejects those zone configurations.
Cargo uses `--release --frozen --offline`; it cannot fetch dependencies or update the lockfile.

Rust execution (including build scripts and proc macros) belongs only in the
isolated validator guest, not on the host. The guest must supply its pinned
Cargo via `CARGO` or `PATH`, and its pinned `RUSTC`, `RUSTDOC`, `CARGO_HOME` and
offline dependency environment. CMake does not execute Cargo during configure.
The build uses the guest's `CARGO_TARGET_DIR` when set. Otherwise it uses
`../.eqemu-rust-target/<hash-of-cmake-build-path>` beside the source tree, so
`cmake -S . -B build` and the checked-in presets keep working without placing
Cargo outputs in source. The source parent must be writable, or supply an
external `CARGO_TARGET_DIR`. Cargo outputs inside the source tree are rejected.
No Rust toolchain or dependencies are installed by CMake. The owner of the
build must remove its external Cargo target directory during cleanup; CMake's
clean target does not remove Cargo's retained incremental state.

On every bridge-enabled zone startup, including utility invocations, a scalar
C ABI call to `eqemu_rust_bridge_v1` must return 42 or startup fails. This live reference keeps
the bridge in the actual zone executable under linker dead stripping. It is
only a bootstrap **link check**, not an actor control interface; it adds no
actor policy, NPC hooks or gameplay behavior. Successful startup is silent and
continues the existing native path without a runtime feature flag.

Full enabled Rust checks and exact-zone symbol/hash qualification, native
utilities, fresh-consumer checks and cleanup remain validator responsibilities;
this integration alone is not evidence that those checks passed.

### Early Rust formatting

The developer command uses the same pinned formatter, workspace scope and
configuration as the isolated validator. It sends the current working snapshot
to the dedicated worker. It does not run Cargo or repository build scripts on
the implementation host. The reviewed snapshot-capable worker/client must be
installed and activated first; an older installation cannot provide this operation.

In an approved isolated development environment, configure the absolute path to
the reviewed installed client once per checkout:

```sh
git config --local eqemu.formatterClient /absolute/reviewed/installation/scripts/format-worker-snapshot
```

The path is operator configuration, outside this checkout. Do not point it at
candidate source. The command also accepts `--client /absolute/path` for one
explicit invocation. From the repository root:

```sh
python3 utils/scripts/dev-format.py
git diff
python3 utils/scripts/dev-format.py --check
```

The first command can boot a disposable worker VM. It applies only verified
formatting changes to captured Rust files whose input bytes remain unchanged.
The trusted installed frontend validates and applies the result; this repository
does not maintain a second snapshot policy or formatter-output writer.
It never stages files, modifies native code, adds files or removes files. Review
and stage the changes yourself. Check mode can also boot a VM; it changes no
source and saves a successful result under the checkout's private Git directory.
A formatting result alone does not create a check receipt. Neither operation is
full build, test or gameplay acceptance.

#### Optional pre-commit check

Enable this hook only in an approved isolated development environment, after
reviewing the checked-out hook/helper. AFK does not install or depend on it.
Inspect any existing hook configuration before replacing it:

```sh
git config --get core.hooksPath
git config --local core.hooksPath .githooks
```

To remove this local override, run `git config --local --unset core.hooksPath`.
If you replaced a prior value, restore that value instead.

The hook checks staged whitespace errors and conflict markers using Git's
`diff --cached --check`. For Rust/config changes it then checks that a prior
successful isolated check matches the current formatter identity, snapshot and
the actual index bytes. Its frontend calls are identity-only. It never boots a
VM, formats, builds, installs tools, stages or discards files. Native-only and
documentation-only changes need no formatter receipt.

Partial staging of formatter inputs is deliberately unsupported. Stage all
intended Rust/config changes and deletions, then run the explicit `--check`
command before committing. Untracked formatter inputs, stale receipts, changed
tools, missing setup and alternate Git indexes, including `git commit --only`,
produce a refusal with instructions. Unrelated native changes can remain unstaged.
Receipts are editable local convenience data, not an acceptance credential.
Bypassed or missing hooks do not bypass AFK's independent pre-publication
formatting or final exact-head, check-only validation.

### Building

##### Linux
From the build directory you can simply call make to build.  

For example.

	make -j4

### Map raycaster

Ordinary zone builds use the maintained Rust backend. Native Linux x86-64 is the
supported target; unsupported platforms and cross compilation fail configuration.
The Rust backend is unconditional. There is no C++ query backend or automatic
fallback. Stock `.map` files load directly; stale `.mmf` caches are ignored.
`zone convert_map` is retired and exits nonzero before startup.

Use the repository `rust-toolchain.toml` pin, Rust 1.99.0, and CXX 1.0.194 from
`rust/raycaster/Cargo.lock`. CMake invokes Cargo frozen/offline at build time,
using the existing actor bridge pattern. Provision the lockfile dependencies and
GNU Linux target in the approved disposable worker first. `CARGO`, `RUSTC`,
`RUSTDOC`, and `CARGO_HOME` may select that worker's pinned installation.
`CARGO_TARGET_DIR` must be outside the source tree. CMake otherwise chooses a
per-build directory beside the source tree. Do not use candidate Cargo or native
builds on the host for this landing's validation.

See `rust/raycaster/README.md` and `tests/map_bridge/README.md` for limits,
ownership, focused controls and exact-head acceptance gates.

The maintained GitHub Actions build runs on Ubuntu x86-64. It provisions
Rust 1.99.0 and fetches both committed Cargo lockfiles before CMake invokes the
frozen/offline builds. The former Windows job and preset are retired. Hosted CI
results must be checked on the PR; disposable worker results do not establish a
hosted pass.
