# Retired convert_map controls

`zone convert_map <filename.map>` now always refuses with status 1 before any
zone, actor, configuration, logging or database startup. Command spelling remains
case insensitive. Malformed arity or an empty filename returns usage and status 1.
Stock `.map` files load directly. There is no MMF writer or batch converter.

Run only in the Stack owner's disposable worker, with an exact-head production
zone binary. The worker must have no network/DB/world access and no server config
in the temporary directory or its parent. Ordinary and sidecar negative dispatch
controls reach normal startup and stop on absent configuration. This exercises
real `main`, with no C++ stubs. Keep JSON stdout/stderr/status receipts.

```sh
python3 /source/tests/convert_map/native_controls.py --zone /output/server/bin/zone
```

The old batch fake-process suite, Perl wrapper, C++/MMF modes, cache-writing
fixture and OFF/MMF build matrix are retired. No host native execution is allowed.
See `tests/map_bridge/README.md` for the remaining qualification handoff.
