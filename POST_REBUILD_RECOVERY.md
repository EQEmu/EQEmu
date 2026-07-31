# POST-REBUILD RECOVERY RUNBOOK

**Written:** 2026-07-11. **Updated 2026-07-21** — the two things that made a rebuild painful are now
fixed in `devcontainer.json`, so a normal rebuild should "just work":
- **`appPort` now includes `"5999:5999/udp"`** (the RoF2/SoD login port). Without it the client
  **times out at the username/password screen** — see CLAUDE.md §2 "RoF2 login needs 5999/udp".
- **The MariaDB datadir is now a persistent named volume** (`aotv4-mysql-data` →
  `/var/lib/mysql`), so a rebuild **no longer wipes the `peq` DB**. Steps 2-4 below are only needed
  the **first** time (when the named volume is created empty) or if the volume is ever reset.

**Why this file still exists:** the named volume is created **empty** the first time it's mounted;
until it's seeded once, `peq` is absent. And if the volume is deleted (or you're standing up a new
machine), you re-seed from the snapshot. Steps below restore `peq` from scratch. After a *routine*
rebuild with the volume intact, skip to **step 5** (shared memory) → **step 6** (start stack).

---

## ⚠️ STEP 0 — IF `peq` IS MISSING, DO NOT IMPORT A SNAPSHOT YET (added 2026-07-24)

> ⚠️⚠️ **UPDATED 2026-07-30 — "MISSING" IS THE EASY CASE. THE DANGEROUS ONE IS `peq` BEING PRESENT
> AND STALE**, which is what happened tonight: the volume held the Jul 24 import while six days of
> work sat on another container's layer, so every check below passed and the database was still
> wrong. **Run `bash .devcontainer/custom/tools/db_sanity.sh` first** — it dates the datadir against
> the shared-memory blobs and server logs on `/src` and exits 1 if they disagree. Full write-up,
> including the two-Docker-daemons trap, is **CLAUDE.md §25**.

**The previous container still has the database.** This happened on **2026-07-24**: the volume was
committed Jul 21 02:45, but a mount only takes effect on the *next* rebuild — and none happened until
Jul 24 22:31. So the DB lived on the **container layer** for 3 days, and the fresh container came up
with an empty volume. Importing the snapshot here would have silently discarded 3 days of work (the
entire 1,445-row achievement catalog, the 43xxx spell set, the class auras — the achievement seed
exists **nowhere on disk**). The old container was still running the whole time.

**Always check the old containers before seeding.** Run on the Windows host — full procedure, the
`-u root` requirement, and how to pick the right container are in **CLAUDE.md §17**:
```powershell
docker ps -a --format "{{.ID}}  {{.CreatedAt}}  {{.Status}}  {{.Image}}"   # incl. stopped; vsc-aotv4-*
docker exec -u root <ID> sh -c "ls /var/lib/mysql | tr '\n' ' '; echo"     # a `peq` entry = jackpot
```
⚠️ **`-u root` is mandatory** — as `vscode` the per-DB dirs are `drwx------ mysql`, so any check
reports "no DB" on a container that has the full database.

Only if **no** container has a `peq` datadir do you continue below — and then seed from
**`/src/peq_recovered.sql.gz`** (Jul 24, achievements included), **not** the older `aotv4_current.sql`:
```bash
gzip -dc /src/peq_recovered.sql.gz | sudo mariadb     # then the grants in step 3
```

---

## What SURVIVES the rebuild (all on the `/src` = `C:\AoTv3\AoTv4` 9p mount)
- All source + the **built `zone` binary** (`/src/build/bin/zone`, built 06:05 **with the backstab
  rework** — do NOT rebuild it unless it fails to launch).
- Config symlinks `build/bin/login.json` + `eqemu_config.json` → `.devcontainer/override/` (already
  correct: `sod_port=5999`).
- `devcontainer.json` (publishes `5999:5999/udp`; mounts the `aotv4-mysql-data` DB volume).
- **The `peq` database itself** — now on the persistent named volume, so it survives a rebuild.
- The DB snapshot **`/src/peq_recovered.sql.gz`** (36 MB gz / 341 MB SQL, dumped **2026-07-24 23:18**
  from the recovered container — **233 tables**, includes the `custom_achievement_*` set: 1,445
  achievements, 2,514 objectives, 54 categories, the 43xxx custom spells + class auras, 8 characters).
  **This is the canonical recovery point.** Verify any dump with
  `gzip -dc x.sql.gz | grep -c "^CREATE TABLE"` (expect 233) + a `Dump completed on …` tail marker.
- ⚠️ The OLDER snapshot **`/src/aotv4_current.sql`** (Jul 21, ~404 MB — full `peq` dump *with all migrations
  already applied*: gear tiers 27k Hallowed + 27k Mythic, spelldmg rule, 7 characters incl. Ashrem,
  login accounts) **predates the achievement system entirely** — seeding from it costs the achievement
  catalog, the 43xxx spells and the class auras. Use it only if `peq_recovered.sql.gz` is unavailable.
  ⚠️ `/src/Deez.sql` has been BOTH the pre-migration dump and (later) a full copy — do not trust its
  name. Whatever you seed from, verify with step 4.
- The world pre-migration auto-backups in `/src/build/bin/backups/*.tar.gz` — a useful mid-point
  fallback (e.g. `peq-2026-07-23.tar.gz`, 223 tables, taken just before the achievement migration).
- `custom/sql/*` migrations (do NOT need to re-run — they're baked into `aotv4_current.sql`).

## What is GONE and must be rebuilt (routine rebuild, volume intact)
- **Nothing DB-side** — the `peq` database + `peq@127.0.0.1` user persist on the named volume.
  (Only when the volume is first created / reset do you run steps 2-3 to seed + grant.)
- Shared memory segment (rebuilt in step 5 — it lives in the container overlay, so always rebuild).
- All running server processes (started in step 6).

---

## FAST PATH — one command
After a rebuild, just run the recovery script (idempotent: seeds the DB only if the volume is empty,
then rebuilds shared memory + starts the stack + prints a health check):
```bash
bash /src/.devcontainer/recover.sh
```
The exact manual steps it automates are below (use them to debug if the script reports a failure).

---

## EXACT STEPS (run from the dev container after rebuild)

### 1. Start MariaDB
```bash
sudo service mariadb start
sleep 3
(ss -ltn 2>/dev/null || netstat -ltn) | grep 3306 && echo "3306 UP" || echo "MariaDB DOWN"
```

### 2. Import the snapshot (self-creates the `peq` database)
```bash
time sudo mariadb < /src/aotv4_current.sql          # ~1 min
```

### 3. Recreate the `peq` user + grants (the rebuild dropped it)
```bash
sudo mariadb -e "CREATE USER IF NOT EXISTS 'peq'@'127.0.0.1' IDENTIFIED BY 'peqpass';
                 GRANT ALL PRIVILEGES ON *.* TO 'peq'@'127.0.0.1'; FLUSH PRIVILEGES;"
```

### 4. Verify the DB over TCP as `peq` (expect 4 characters)
```bash
mysql -h127.0.0.1 -upeq -ppeqpass peq -N -e "
  SELECT 'chars', COUNT(*) FROM character_data
  UNION ALL SELECT 'buckets', COUNT(*) FROM data_buckets
  UNION ALL SELECT 'hallowed', COUNT(*) FROM items WHERE id BETWEEN 300000 AND 599999
  UNION ALL SELECT 'mythic',   COUNT(*) FROM items WHERE id BETWEEN 600000 AND 899999
  UNION ALL SELECT 'spelldmg_rule', rule_value FROM rule_values WHERE rule_name='Spells:IgnoreSpellDmgLvlRestriction';"
# expect: chars=4, hallowed=15621, mythic=15621, spelldmg_rule=true
```
If those look right, **the migrations are already in** — do NOT re-run `custom/sql/*`.

### 5. Rebuild shared memory (world must be DOWN — it is, nothing started yet)
```bash
cd /src/build/bin && ./shared_memory 2>&1 | tail -3
```

### 6. Start the full stack (detached so a shell timeout can't kill them)
```bash
cd /src/build/bin
setsid nohup ./loginserver > logs/loginserver_manual.log 2>&1 < /dev/null &
sleep 2
setsid nohup ./world       > logs/world_manual.log       2>&1 < /dev/null &
sleep 5
setsid nohup ./eqlaunch zone > logs/eqlaunch_manual.log  2>&1 < /dev/null &
setsid nohup ./ucs         > logs/ucs_manual.log         2>&1 < /dev/null &
setsid nohup ./queryserv   > logs/queryserv_manual.log   2>&1 < /dev/null &
sleep 5
for p in loginserver world eqlaunch ucs queryserv; do echo "$p: $(pgrep -x $p | wc -l)"; done
```

### 7. Verify health — THE WHOLE POINT: loginserver must bind **5999**
```bash
(ss -lunp 2>/dev/null || netstat -lunp) | grep -E ':5998|:5999'    # both must be bound by loginserver
grep -E 'Connected to database|successfully authenticated' \
  /src/build/bin/logs/world_manual.log /src/build/bin/logs/loginserver_manual.log | tail -3
grep -iE 'fatal|malloc|Cannot continue' /src/build/bin/logs/world_manual.log && echo "WORLD CRASH" || echo "world ok"
```

### 8. Client
- `eqhost.txt` stays: `[LoginServer]` / `Host=127.0.0.1:5999`  (NO change from before).
- Log in with account **mikethefiend** (or mikethefiend1). Characters: Ashrem (GM), Cexoos,
  Apheyawus, Eqpoqapon.
- **Run the client WINDOWED** (the `dinput8.dll` overlay needs it).

---

## Gotchas / fallbacks
- **`zone` won't launch** (missing lib after image change): `cd /src/build && ninja zone`, then redo step 5+.
- **Snapshot import errors**: fall back to `sudo mariadb < /src/Deez.sql` (Jul 10), then re-apply the
  two migrations, then rebuild shared memory:
  ```bash
  sudo mariadb --database peq < /src/.devcontainer/custom/sql/aotv4_item_spelldmg_healamt.sql
  sudo mariadb --database peq < /src/.devcontainer/custom/sql/aotv4_gear_tiers.sql
  ```
- **Client still can't reach login after all this**: confirm `5999:5999/udp` is in `devcontainer.json`
  `appPort` AND that the rebuild actually picked it up (this whole rebuild was to publish it). See
  CLAUDE.md §2 "RoF2 login needs 5999/udp PUBLISHED".
- **Keep exactly one `eqlaunch`.** If clients time out entering world, check for rogue bare `./zone`
  procs + a single `eqlaunch` (CLAUDE.md §10).
- Once login is confirmed working, this file + `/src/aotv4_current.sql` can be deleted (or kept as a
  recovery point). Consider the persistent-`/var/lib/mysql`-volume follow-up so this never recurs.
