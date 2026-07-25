# AoTv4 — All-Classes Random-Progression EQ Server

EQEmu server + RoF2 client mod. The headline feature is a **level-up reward window**: on each
level you're offered **3 random rewards** (spells or class-specific combat abilities) and pick
one, which the server scribes/trains. The window is drawn by a client-side `dinput8.dll`
so it works on a **vanilla RoF2 client — no MacroQuest/E3 required**.

> **⚠️ ALL CLASSES (was Bard-only).** Originally every character was forced to Bard; as of **2026-07**
> the server is opened to **all 16 classes**. The four pure-melee classes (Warrior/Monk/Rogue/Berserker)
> are turned into casters **client-side** by the dll (`core_allcasters`, §14) + **server-side** mana
> (`CalcMaxMana`). The reward-pool spells are opened to every class, and the reward abilities cast as
> **spells not songs** even for a Bard (client `IsBardSong` detour, §14). Sections below that still say
> "everyone is a Bard" are **historical** — Bard is now just one of the 16. Existing pre-pivot characters
> remain Bard (a valid class; no forced reclass).

---

## 0. Version control — THREE separate git repos, and the parent ignores two of them

`/src` is its own repo, but **`.devcontainer/.gitignore` has `repo/`**, so the two most valuable trees
are NOT in it — they are independent nested clones with their own remotes and branches:

| Tree | Repo / remote | Branch holding AoTv4 work |
|---|---|---|
| `/src` (server C++, SQL, docs) | this repo | `aotv4-spell-rebuild` |
| `.devcontainer/repo/eq-core-dll` (the **dll**) | `Twochords/Dinput` | `aotv4-advloot-module` |
| `.devcontainer/repo/quests` (**all Lua**) | clone of `ProjectEQ/projecteqquests` | `aotv4` |

⚠️ **`git status` in `/src` tells you NOTHING about the dll or the Lua.** As of 2026-07-25 every AoTv4
Lua module (`spell_choice`, `aa_choice`, `aa_pool`, `era_system`, `pok_travel`, `bazaar_broker`, the
43xxx spell scripts…) had **never been committed anywhere**, and the dll had nothing since the
`tradeskill window` commit. Both now have a first recovery point on the branches above (local commits,
not pushed). **`cd` into each repo and check it separately before assuming work is safe.**

## 1. Environment & layout

- `/src` (dev container) == `C:\AoTv3\AoTv4` (Windows, 9p mount). The EQ **client** is a
  separate RoF2 install on Windows; the dev container holds the **server + sources**.
- Server build/run dir: `/src/build/bin` (config symlinked from `.devcontainer/override/`).
- **Server C++ source:** `/src/zone`, `/src/world`, `/src/common`, … (rebuildable; see §8).
- Quests / Lua: `/src/.devcontainer/repo/quests/` (symlinked into `build/bin/quests`).
  - `quests/global/global_player.lua` — global player event hooks.
  - `quests/lua_modules/` — `spell_choice.lua` (core logic), `spell_pool.lua` (gen),
    `spell_icons.lua` (gen), `skill_pool.lua` (combat-ability rewards), `spell_blacklist.lua` (gen).
- Client mod source: `/src/.devcontainer/repo/eq-core-dll/` (builds `dinput8.dll`).
  - `core_spellwindow.cpp` — the spell window, the **AA window** (§6), the **Portal window** (§11),
    **and** the skill-unlock hook; all share the `dsp_chat` + `ProcessGameEvents` detours.
  - `core_skillunlock.h` — declares `EnableSkillUnlock()` (impl is in `core_spellwindow.cpp`).
  - `core_allcasters.cpp` / `core_allcasters.h` — the **all-classes-as-casters** patch (§14). Its OWN
    translation unit + OWN dedicated detours; NOT piggy-backed on the spell-window / tradeskill hooks.
  - `_options.h` — feature flags; `core_init.h` — wires them in `InitOptions()`.

## 2. Running the server

The stack is 5 processes, started from `/src/build/bin`:

```bash
cd /src/build/bin
nohup ./loginserver  > logs/loginserver_manual.log 2>&1 &
nohup ./world        > logs/world_manual.log       2>&1 &
nohup ./eqlaunch zone> logs/eqlaunch_manual.log     2>&1 &   # zone launcher (boots zones on demand)
nohup ./ucs          > logs/ucs_manual.log          2>&1 &   # chat/mail/bazaar-trader search
nohup ./queryserv    > logs/queryserv_manual.log    2>&1 &   # logging
```

- **"Server down" but you reach char select:** `eqlaunch` isn't running → world logs
  `No zoneserver available to boot up`. Restart `eqlaunch zone`. Keep **exactly one** eqlaunch.
- Zones are **dynamic**: `eqlaunch` boots `./zone` processes on demand and respawns them; idle
  zones self-terminate. That's normal, not a crash.

### RoF2 login needs 5999/udp PUBLISHED (client "times out at username/password")
The RoF2 client logs in on the **SoD stream** (`login.json` `sod_port: 5999`), NOT the Titanium
port 5998. The dev container reaches the Windows client only through `devcontainer.json`
**`appPort`**, which therefore **MUST include `"5999:5999/udp"`** (alongside 5998 world-link,
7000-7005 zones, 9000 world). If it's missing, the loginserver runs fine and world connects, but
the client's login packets never reach the container → **timeout at the username/password screen**
with **zero client lines in `logs/loginserver_manual.log`** (that empty log is the tell). Diagnose:
`(ss -lunp | grep :5999)` shows the loginserver bound locally, but the port isn't forwarded to the
host. Fix = `appPort` entry + a container rebuild. Client `eqhost.txt` stays `Host=127.0.0.1:5999`.

### The DB and the login port both survive rebuilds now (persistent volume)
A container rebuild used to wipe **both** the `peq` database (datadir was on the ephemeral overlay)
and the 5999 port (missing from `appPort`) — the recurring "server won't come up after a rebuild"
pain. Both are fixed in `devcontainer.json`: `appPort` includes `5999:5999/udp`, and the MariaDB
datadir is a **named volume** (`source=aotv4-mysql-data,target=/var/lib/mysql,type=volume`) that
**persists across rebuilds**. See `POST_REBUILD_RECOVERY.md`. The canonical DB snapshot lives at
**`/src/aotv4_current.sql`** (full `peq` with all migrations baked in) — needed only for the ONE
import after the volume is first created, or to seed a brand-new volume; routine rebuilds keep the DB.

## 3. The level-up reward window

No opcode patching — communication rides on **chat**:

```
LEVEL UP
  server (Lua) ──► client : "SPELLCHOICEDATA name1|icon1^name2|icon2^name3|icon3"
                            (server keeps the real ids/types in a data bucket, keyed by char)
  dinput8.dll  ── swallows that line, shows a layered GDI window with 3 rows + icons
  player clicks "Select N"
  dinput8.dll  ──► server : "/say spellpick N"     (this echo is also swallowed)
  server (Lua) ── event_say validates N against the stored bucket, then scribes a spell /
                  trains a discipline / raises a combat skill
```

The **id list is server-side**; the player only sends an **index 1-3**, so a modified client
can't grant an arbitrary reward — only one of the three it was actually offered.

### Server side (Lua)
- `global_player.lua`: `event_connect` sends `SKILLUNLOCKDATA` (see §4) — the old **Bard-force is REMOVED**
  (all classes now, §14); `event_level_up` auto-grants non-combat skills then calls `spell_choice.offer(e)`;
  `event_say` calls `spell_choice.handle_say(e)`.
- `spell_choice.lua` — builds the 3 choices (spells + ~1 combat skill), emits `SPELLCHOICEDATA`,
  validates picks. Stored bucket tokens are **typed**: `S:<spellid>` (spell/disc) or
  `K:<skillid>` (combat skill). Tunables at top: `CHOICE_COUNT`, `LEVEL_BAND`, `SAY_TRIGGER`,
  `SKILL_OFFER_CHANCE` (~0.7), `SKILL_GRANT_PCT` (0.25).
- `spell_pool.lua` (**generated**, see §5), `spell_icons.lua` (**generated**),
  `spell_blacklist.lua` (**generated**), `skill_pool.lua` (hand-curated; see §4).

### Client side (`core_spellwindow.cpp` → `dinput8.dll`)
Gated by `areSpellChoiceWindowEnabled` (`_options.h`). RoF2, image base `0x400000`, addresses
rebased at runtime:
- **Chat hook** — detour `CEverQuest::dsp_chat` (`0x51F1A0`): swallow `SPELLCHOICEDATA`,
  `SKILLUNLOCKDATA`, and the `spellpick` echo; parse `name|icon`.
- **Per-frame heartbeat** — detour `ProcessGameEvents` (`0x539E60`, main thread): spawn the
  overlay thread once, and run the pick **on the game thread** via `CEverQuest::InterpretCmd`
  (`0x51FCE0`) → `/say spellpick N`.
- **Window** — a separate **layered, top-most GDI window** on its own thread. We do NOT draw
  through EQ's D3D device: the RoF2 client renders via EQGraphicsDX9.dll's own device, which
  shares no vtable with any device we can create, so Present/swap-chain hooks never fire
  (proven empirically: `present=0 swap=0` with all vtables patched). Works in **windowed mode**.
  Draggable by the title strip; NOT `WS_EX_NOACTIVATE` (a click takes focus so EQ stops
  mouse-look → dragging doesn't rotate the camera); shown only while EQ or the overlay itself
  is the **foreground** window (so it never floats over other apps).
- **Icons** — decode `uifiles/default/spellsNN.tga` (own TGA loader: type 2 + RLE 10, 24/32-bit).
  Mapping: `sheet = icon/36 + 1`, `cell = icon%36`, columns = `sheetWidth/40`, cell 40×40.

### The picker is now a NATIVE SIDL window (2026-07-25) — `core_spellchoice_native.cpp`
The GDI overlay above is superseded (it stays in the dll but is never shown). Own TU, no detours of
its own; our `dsp_chat` calls `SpellChoiceParseTransport`. Wire format is **unchanged**
(`SPELLCHOICEDATA` / `SPELLDESCDATA` / `/say spellpick N`) so the server needed no edits.
- **Layout = three "cards"** (icon, name, one-line summary, button) over a shared STML description
  pane — NOT a listbox: a listbox can't carry a per-row icon here, and its column header + red
  selection bar looked nothing like EQ. Two-step commit (Select previews → Confirm learns), because
  a mis-clicked level reward can't be undone.
- ⚠️ **Per-row icons in a SIDL window — the trick.** This build maps **no** runtime icon API:
  `CTextureAnimation::SetCurCell`, `CSidlManager::FindAnimation` and `CListWnd::SetItemIcon` are all
  absent from `eqgame.h` (only `GetItemIcon` exists), so a texture cell **cannot be chosen from
  code**. Instead the choice moves into the XML: one `Ui2DAnimation` per icon (already pointing at
  the right cell of the right `SpellsNN.tga`) + **one hidden Button per (row, icon)** carrying an
  **inline `<ButtonDrawTemplate>`** (the form stock `EQUI_Inventory.xml` uses for its icon buttons);
  the dll just `Show()`s the matching one, and `CXWnd::Show` **is** mapped. Only
  viable because the reward pool uses **22 distinct icons** (113 custom spells + 4 skill icons).
  Generated by **`aotv4_client_install/gen_choice_xml.pl`** → `EQUI_AoTSpellChoiceWnd.xml` (1150
  lines, 77 pieces). Keep `@ICONS` there and `kIcons[]` in the cpp identical; a new pool icon in
  neither simply draws nothing. Same trick applies to any other window that wants icons.
- ⚠️⚠️ **Texture names in SIDL are CASE SENSITIVE, and `/src/EQUI_Animations.xml` is the authority.**
  A `<Texture>` is resolved against the texture registry built from that file's ~588 `<TextureInfo>`
  entries — **not** against the filesystem. The spell sheets are declared **`Spells01.tga`** (capital
  S, 256×256, `A_SpellIcons` is the stock `Grid` animation over them with `CellWidth/CellHeight` 40).
  Writing `spells01.tga` resolves to **nothing**: the button is created, found and shown, but draws
  an empty hole — **no `UIErrorLog.txt` entry, no error anywhere**. This cost a full round trip; it
  is invisible to every check we have, so grep `EQUI_Animations.xml` for the exact spelling before
  referencing any texture. (Our GDI loaders get away with lowercase because they open the file
  through Windows, which is not case sensitive — so a name that works there can still fail in SIDL.)
- ⚠️⚠️ **Texture names in SIDL are CASE SENSITIVE, and `/src/EQUI_Animations.xml` is the authority.**
  A `<Texture>` is resolved against the texture registry built from that file's ~588 `<TextureInfo>`
  entries — **not** against the filesystem. The spell sheets are declared **`Spells01.tga`** (capital
  S, 256×256; `A_SpellIcons` is the stock `Grid` animation over them with `CellWidth/CellHeight` 40).
  Writing `spells01.tga` resolves to **nothing**: the button is created, found and shown, but draws
  an empty hole — **no `UIErrorLog.txt` entry, no error anywhere**. It is invisible to every check we
  have, so grep `EQUI_Animations.xml` for the exact spelling before referencing any texture. (Our GDI
  loaders get away with lowercase because they open the file through Windows, which is not case
  sensitive — a name that works there can still fail in SIDL.)
- Verify a SIDL tag exists before using it: `strings -n 3 eqgame.exe | grep -qx "<Tag>"`
  (`Ui2DAnimation`/`Frames`/`Texture`/`Hotspot`/`Cycle`/`Duration`/`PressedFlyby` all pass;
  `Cell` does not). Then `aotv4_client_install/validate_ui_xml.sh` before copying.
- **Ctrl+Q**: opens the native picker when a reward is pending, else the old Reward Journal on its
  **Lost** tab — the journal's Spell and AA tabs are dead (this module owns `SPELLCHOICEDATA`, and
  random AA is retired: `areAAChoiceWindowEnabled=false`, the `AACHOICEDATA` line is parsed and
  swallowed but shows nothing).
- **Client install:** copy `EQUI_AoTSpellChoiceWnd.xml` to `uifiles/default/` **and**
  `<Include>EQUI_AoTSpellChoiceWnd.xml</Include>` in `EQUI.xml`. Missing either = `CCustomWnd` can't
  find its screen and returns **silently** — no error, no window.

> **Loader-lock:** the `Enable*` functions run inside `DllMain` (DLL_PROCESS_ATTACH). Only
> **memory-patch detours** (`EzDetour`) are safe there. Window/D3D/thread creation must be
> deferred to runtime (first chat line / first `ProcessGameEvents`), or startup hard-fails
> (`0xc0000142`).

## 4. Combat skills & cross-class abilities

Goal: a Bard inherits **other classes' skills/abilities**. Three layers had to agree.

**(a) Reward-gating (Lua).** `skill_pool.lua` is the rotation pool — **only class-specific
activated abilities** (Backstab, Bash, Kick, Disarm, Taunt, Berserking, Frenzy, and the monk
strikes Dragon Punch/Eagle Strike/Flying/Round Kick/Tiger Claw). Passive skills (weapon skills,
Offense/Defense, Dodge/Parry/Riposte/Block, Double/Triple Attack, Dual Wield, Intimidation) are
**auto-learned** via `global_player.lua` `free_skills` and never rotate. A picked skill is set
to `SKILL_GRANT_PCT` of its cap.

**(b) Client unlock hook (`core_spellwindow.cpp`, `areSkillsUnlocked`).** The RoF2 client hides
skills a class isn't coded for. We detour `CSkillMgr::GetSkillCap` (`0x5BAEC0`,
args `pChar, level, class, skill, …`) and **only reveal hidden skills** — a native skill
(cap > 0) passes through untouched (so weapon skills are never removed); real skill ids `0..76`
that the class can't natively have get a level-scaled cap (unused slots `77+` stay 0, no "None"
rows). Reward-gated *combat* skills are revealed **only if earned** — the server sends
`SKILLUNLOCKDATA <csv of earned skill ids>` (skills with value > 0) on connect and after each
pick; the hook hides un-earned combat skills from both the Skills window and the ability picker.

**(c) Server execution (`/src/zone/special_attacks.cpp`, `Client::OPCombatAbility`).** EQEmu
class-gates the activated specials. Patched to be **skill-gated** (any class that trained the
skill): Kick (added `GetSkill(SkillKick) > 0` to `can_use_kick`), the monk strikes (dispatch if
`is_monk_special`, not just `class == Monk`), Backstab (`|| GetSkill(SkillBackstab) > 0`).
Disarm/Taunt/Frenzy were already skill-gated; Bash needs a shield. `MonkSpecialAttack` has no
internal class checks (damage = `GetBaseSkillDamage`), so it's safe cross-class. **Requires a
zone rebuild** (§8).

## 5. Spell pool & blacklist (era / junk filtering)

`spell_pool.lua` is generated from `spells_new`, indexed by learn level. The reward is
**expansion-capped by level**: a char is only offered spells at/below its level, and a spell's
learn level sits in its expansion's band (classic 1-50, Kunark/Velious/Luclin 51-60, PoP+ 61-70…).
Filter: `id < 10000` (drops ~30k modern rank/revamp spells), no `Rk.` versions, learnable
(`classes8` = lowest class level; we set `classes8 = LEAST(real class cols)` for Bard usability,
so it doubles as the index). Regenerate:
```bash
mysql -h127.0.0.1 -upeq -ppeqpass peq -N -e "
  SELECT classes8,id,name FROM spells_new
  WHERE id<10000 AND name NOT LIKE '% Rk. %' AND classes8 BETWEEN 1 AND 100
  ORDER BY classes8,id;"   # then format to  [lvl]={ {id=,name=}, ... }  (see git history)
```

`spell_blacklist.lua` (**generated**) = `{ [id]=true }` of spells **never** offered, applied at
runtime in `spell_choice.gather_candidates`. Criteria:
- resurrect (effect `81`), Enchant-material (name `Enchant %`),
- curse-counter cures (effect `116` with **negative** base = removes curse counters),
- LDoN dungeon-object utility (`targettype = 34` — Iony's/Reebo's appraise/disarm/unlock lines).

> No expansion column exists in `spells_new`, and Allakhazam/EQ-Resource are JS-rendered (can't
> crawl). So era filtering is the `id<10000` + level approximation, not a surgical classic list.

## 6. Death-driven AA rewards (roguelite) — ⚠️ RETIRED 2026-07-25, kept as history

> **Random AA is gone and AA is back to BASE EQ**: earned as AA experience, spent in the client's own
> AA window. The whole picker is unwired — `global_player.lua` no longer requires `aa_choice` (no
> `grant_picks` on death, no `handle_say`, no `reoffer_if_banked`), the dll's AA overlay is **deleted**
> (§3), and `aa_choice.lua`/`aa_pool.lua` remain on disk but nothing loads them. The dll still swallows
> stray `AACHOICEDATA`/`AADESCDATA` lines so they can never reach chat.
> **DB revert = `custom/sql/aotv4_aa_revert.sql`** (restores `aa_ranks.level_req`, `aa_ability.classes`
> and `grant_only` from `utils/sql/peq_aa_tables_post_rework.sql`, because `aotv4_aa.sql` overwrote the
> originals in place). Until it runs, `grant_only=1` hides all 382 tagged AAs from the native window.
> ⚠️ Retiring this **orphans `era_system.check_unlock`** — see §12.


A second progression layer parallel to the level-up window. **On death, ALL of the run's
experience is banked as Alternate Advancement and the character is reset to level 1** — AAs are
the permanent meta-progression; levels are per-run. A second random picker (its own chat channel,
so it can drive a separate dll window) then spends the banked points.

### Server side (Lua)
- `global_player.lua` `event_death(e)`: `SetLevel(1)`, `SetEXP(0, GetAAExp())`, then bank
  `floor(total_xp / divisor)` points by calling `aa_choice.grant_picks(e, banked)`. **The points go
  into a PRIVATE data bucket (`aa_bank_<charid>`), NOT the native unspent pool** — so the native AA
  (V) window shows 0 spendable points and the player can't bypass the picker to buy AAs directly.
  Picks are granted **free** (`ignore_cost=true`) and the cost is deducted from the bucket.
  **Diminishing returns:** `divisor = AA_XP_BASE * (1 + GetSpentAA()*AA_SCALE)` — the more AAs you
  own, the more XP each new point costs (`AA_XP_BASE=15M` ≈ 10 points at a level-50 death;
  `AA_SCALE=0.005`). `event_say` calls `aa_choice.handle_say(e)` (alongside `spell_choice`).
- `aa_choice.lua` — **budget-driven** picker. The player's *unspent* pool (`GetAAPoints`) IS the
  budget. Each pop offers `CHOICE_COUNT` (3) random AAs that are **affordable** (`cost ≤ budget`)
  **and prereq-satisfied**, emits `AACHOICEDATA <banked>^name|icon|cost|cls^…` + a sibling
  `AADESCDATA d1^d2^d3` line (drives the dll window) **plus** chat saylinks (work with no client
  mod). Picking trains the next rank **free** (`GrantAlternateAdvancementAbility(id, cur+1, true)`)
  and deducts the cost from the private bank; it keeps popping until nothing is affordable
  (`MAX_PICKS=30` cap). Tunables at top. Buckets: `aa_bank_/aa_choice_/aa_seed_/aa_pcount_<charid>`.
  Seed RNG on a per-offer counter (post-death `GetEXP`/level are 0).
  **Per-AA cap (`RANK_BUDGET=10`):** an AA is only offered while its *total* investment
  (`next_rank * cost`) stays within the cap, so no single AA needs more than ~one death's bank —
  cheap AAs (cost 2) still reach ~rank 5 (enough for rank≤3 prereqs), a cost-9 AA stays rank 1.
  Keep `RANK_BUDGET` in step with the per-death bank (`AA_XP_BASE`); raise BOTH as the level cap rises.
- **dll window** (`core_spellwindow.cpp`, `areAAChoiceWindowEnabled`): a SECOND layered GDI window
  (cool slate theme, lower-third placement) parallel to the spell window. It **shares the same two
  detours** (`dsp_chat`, `ProcessGameEvents`) — both can't re-hook the same address, so the AA code
  lives in those bodies, gated by feature flags. Shows **banked AA in the header**, each choice's
  **cost**, a Train button → `/say aapick N` (echo swallowed like `spellpick`), and a **bottom
  description panel** that shows the hovered row's text. Both windows have the hover/desc panel
  (`DrawDescPanel` + `RowAtY`, fed by `SPELLDESCDATA`/`AADESCDATA`).
- `aa_pool.lua` (**generated**) — `pool[level]={ {id=,name=,icon=,cls=,cost=,desc=,pr={{a=,r=}}}, … }`.
  `icon` = cast-spell `new_icon`, else a thematic fallback by name (heal/caster/melee/defense/song/
  craft). `cost` = first-rank cost, `desc` = `db_str` type 4 (truncated), `pr` = first-rank prereqs.
  Generated from the **Bard-tagged** (`classes&128`) set — which the migration restricts to **classic
  (expansion ≤4), level ≤60, real-named, deduped-by-name** AAs (so no "Unknown DB String"/modern
  clutter). `spell_desc.lua` (gen, `db_str` type 6) feeds the spell window's hover the same way.
  The picker's `prereqs_met` mirrors the server's prereq loop so it never offers a rejectable AA.
- **Hiding untrained from the native window:** the tagged set is also flagged `grant_only=1`. The
  native AA window then lists each only once **trained** and can't buy it there (the picker grants
  via `ignore_cost=true` → `CanPurchase` `check_grant=false`, so `grant_only` doesn't block us).

### Three things had to agree (like combat skills)
1. **Class-agnostic.** Migration `aa_ability.classes |= 128` adds the Bard bit. The AA loader does
   `a->classes = e.classes << 1` (zone/aa.cpp:1792), so the DB's `1<<(class-1)` (Bard=128) becomes
   `1<<class` (256) in memory — which is exactly what the **stock** gate `classes & (1<<GetClass())`
   checks. ⚠️ **Do NOT "fix" that line to `GetPlayerClassBit`** — the `<<1` makes it correct;
   "fixing" it breaks every narrow-class AA. `aa.cpp` stays **stock**.
2. **Expansion.** AAs start in Luclin (`aa_ranks.expansion ≥ 3`). With classic theming
   (`Expansion:CurrentExpansion=0`), the rule **`Expansion:UseCurrentExpansionAAOnly` blocks ALL
   AAs** — set it **`false`**; expansion gating is done by the pool instead (like spells). The
   per-char `m_pp.expansions` check still applies (it = `RuleI(World, ExpansionSettings)` = all).
3. **Level gate.** Migration lowers `aa_ranks.level_req` `1..60 → 1` (deps preserved) so normal
   ranks are buyable at the always-level-1 post-death state. Ranks with original `level_req > 60`
   stay gated (unreachable while picking at level 1 — acceptable; they're raid-tier).

### Reset AAs for testing (char must be at **character select**)
```sql
SET @cid:=(SELECT id FROM character_data WHERE name='Ashrem');
DELETE FROM character_alternate_abilities WHERE id=@cid;
UPDATE character_data SET aa_points=0, aa_points_spent=0, aa_exp=0 WHERE id=@cid;
```
GM `#resetaa aa` only *refunds* spent → unspent (doesn't zero the pool). The migration +
`UseCurrentExpansionAAOnly` live in `.devcontainer/custom/sql/aotv4_aa.sql`.

## 7. Database setup
- **All 16 classes are playable** (§14). Historically every char was forced to Bard (`class = 8`); the
  force is removed and `char_create_combinations` restored to all classes. Existing chars stay Bard.
- **`spells_new.classes8`** = the spell's min learnable level (= the reward-pool index). It's now also
  opened across every class column (`classes1..16`) so all classes can scribe/cast the reward spells.
- **Bard `skill_caps`** (`class_id=8`) raised so skills scale; client also needs the exported
  `SkillCaps.txt` (`export_client_files`) installed in the EQ root.
- **Expansion lock:** `rule_values Expansion:CurrentExpansion = 0` (Classic).
- Test char `Ashrem`: Bard, GM. (Reset a char's combat skills via
  `UPDATE character_skills SET value=0 WHERE id=<charid> AND skill_id IN (...)` — only sticks
  while that char is at **character select**, else the live zone saves over it.)

## 8. Building

**`dinput8.dll`** (Windows, Visual Studio): project
`eq-core-dll/src/eq-core-dll-vs2022.vcxproj` (toolset **v143**). Non-default settings for the
legacy MQ2-derived code: `ConformanceMode=false`, `LanguageStandard=stdcpp14`, `SDLCheck=false`,
defines `_CRT_SECURE_NO_WARNINGS;_WINSOCK_DEPRECATED_NO_WARNINGS`. `core_spellwindow.cpp` links
`gdi32`/`user32` via `#pragma comment`. **Close EQ before copying** (it locks the dll).

**Server (zone, etc.)** in the dev container — ninja:
```bash
cd /src/build && ninja zone     # ~minutes; relinks bin/zone
```
After a server-code change you MUST restart **every** booted zone (see §10), not just the pool.

## 9. Porting elsewhere

**To a player's CLIENT:** copy `dinput8.dll` next to `eqgame.exe`; run **windowed**; stock
`uifiles/default/spellsNN.tga` provide icons. Install the exported `SkillCaps.txt` (+ optionally
`spells_us.txt`, `dbstr_us.txt`) in the EQ root so skill caps/levels match the server. The dll's
offsets (`0x51F1A0`, `0x539E60`, `0x51FCE0`, `0x5BAEC0`) are for this exact RoF2 build — a
different build needs them re-derived. (`EQUI_SpellChoiceWnd.xml` is dead — overlay is self-drawn.)

**To another SERVER:** copy the `quests/lua_modules/*` + the `global_player.lua` hooks;
regenerate `spell_pool.lua`/`spell_icons.lua`/`spell_blacklist.lua` **and** `aa_pool.lua` from
that DB; apply the `special_attacks.cpp` patches and rebuild zone; run `custom/sql/aotv4_aa.sql`
(AA class/level migration) and set `Expansion:UseCurrentExpansionAAOnly=false`; set the DB (Bard
class, `classes8`, skill caps, expansion). The windows are generic (`SPELLCHOICEDATA name|icon` /
`spellpick N`; `AACHOICEDATA name|icon|cost` / `aapick N`). `aa.cpp` stays stock.

## 10. Gotchas / lessons learned
- **Lua + server-code reloads:** `#reloadquest` does NOT reload `require`d Lua modules, and a
  rebuilt `zone` binary only takes effect in **freshly-booted** zones. After a Lua or C++ change,
  kill **all** zone procs (`for p in $(pgrep -x zone); do kill -9 $p; done`) so `eqlaunch`
  respawns them — killing only `dynamic_*` leaves already-booted named zones (e.g. freporte) on
  the old code. Then **relog**.
- **Can't hook EQ's D3D device** — dummy-device vtable trick fails here; use the layered window.
- **DllMain loader lock** — defer window/thread/D3D creation out of the `Enable*` functions.
- **Skill cap hook** — only *reveal* (cap 0 → value); never override a native cap, and bound to
  ids `0..76` or you get "None" rows. Skill values live in `CHARINFO2` (not the `pChar`
  `CHARINFO` passed to `GetSkillCap`) — that's why earned-state comes via server sync, not memory.
- **Activated abilities = 3 layers** — client display (`GetSkillCap`), client *send*
  (only for skills it thinks you have), and server *execute* (`special_attacks.cpp` class gates).
  All three must allow it.
- **Spell icon mapping** — raw index (`icon/36`, `icon%36`), 0 = none; derive columns from sheet
  width, don't assume 6.
- **Bazaar** — RoF2 only ships the revamped (DoN) bazaar; the old Luclin bazaar is unobtainable
  on a vanilla client.
- **AA class bitmask is shifted** — `aa_ability.classes` is loaded as `classes << 1`
  (zone/aa.cpp), so the gate `classes & (1<<GetClass())` is correct *as-is*. Tag Bard with the DB
  bit **128** (`classes | 128`), NOT 256, and **don't** rewrite the gate to `GetPlayerClassBit`.
  Symptom of getting it wrong: 65535 (all-class) AAs grant but narrow-class AAs silently fail.
- **AAs aren't in shared memory** — the zone loads `aa_ability`/`aa_ranks` straight from the DB at
  boot, so DB edits just need a zone restart (no `./shared_memory` rebuild). Rules are also read at
  boot — a `rule_values` change (e.g. `UseCurrentExpansionAAOnly`) needs a restart, not a reload.
- **`items` (and `spells_new`) ARE in shared memory** — unlike AAs, item edits need a full rebuild:
  stop world+zones, `cd build/bin && ./shared_memory`, restart world (zones reboot on demand). Do it
  with world DOWN, or world keeps the stale mmap.
- **Gear tiers** (`custom/sql/aotv4_gear_tiers.sql`): two tiers above native, generated from
  obtainable equippable gear (slots>0, in lootdrop_entries/merchantlist, **base ids <300,000** —
  never re-tier our own rows). **Hallowed = base id +300,000** (stats/resists/dmg ×2, hp/mana/end
  ×2, AC ×1.5, int→.5 spelldmg, wis→.5 healamt, str/agi/dex /10 → attack/strikethrough/accuracy,
  instruments ×1.5, tradeable). **Mythic = +600,000** (hp/mana/end ×2.5, AC ×2, int/wis→1×
  spelldmg/healamt, +½ heroic on every stat+resist, instruments ×2, **No Drop**). All three tiers:
  classes/races=65535, not-lore; **native + Hallowed tradeable, only Mythic No Drop**. Derived stats
  use the scaled (×2) values.
  Two C++ hooks (tracked patches): `NPC::AddLootDropTable` (loot.cpp) rolls **25% Hallowed / 5%
  Mythic** per base drop (mode-independent, since `lootdrop_entries.chance` is a weight in the
  dominant weighted loot mode); `AoTv4MythicReward` (questmgr.cpp + lua_client.cpp) upgrades
  **quest-reward** gear to its Mythic tier (epics never hand out native). Re-run the SQL → rebuild
  shared memory → restart.
  - ⚠️ **Tier offset = 300,000 (step), NOT 1,000,000.** RoF2 item LINKS encode the id in a
    5-hex-digit field masked to `0xFFFFF` (1,048,575, see `common/say_link.cpp`), so an item id
    ≥ 1,048,576 makes its chat link render as garbage (and can desync the client link parser for
    following lines). The old +1M/+2M scheme broke every Mythic link (and Hallowed of base >48,575).
    Base ids top out at 147,494, so the step is **300,000**: Hallowed = base+1×step, Mythic =
    base+2×step, band `[300000,900000)`, all under the ceiling; base recovery is `id % 300000`.
    The step lives in **5 C++ spots** (loot.cpp, npc.cpp, questmgr.cpp, attack.cpp, and
    tradeskills.cpp `AOTV4_TIER_STEP`) — keep them in step with the SQL. Renumbering existing tier
    ids is a one-time `custom/sql/aotv4_tier_renumber.sql` (migrates live inventory/merchant/etc.
    refs; spares the refine bag 2000060).
- **Never run `make zone`** — that Makefile target is NOT a build; it runs a **bare `./zone`** (no
  instance args), which registers with world as an unmanaged static zone ("Zone started with name
  [.] by launcher [NONE]") and **breaks zone routing** (clients time out on enter-world). To rebuild
  the zone binary use `cd /src/build && ninja zone`. If clients can't zone in, check for rogue bare
  `./zone` procs and **exactly one** `eqlaunch` (`pgrep -x eqlaunch`); kill extras + restart world.
- **`small` is a Windows macro** — `<rpcndr.h>` does `#define small char`, so a local named
  `small` (e.g. `HFONT small`) miscompiles into `HFONT char` with a cascade of bogus errors. Avoid
  `small`/`hyper` as identifiers in the dll.
- **AA grant rejected silently** — `GrantAlternateAdvancementAbility(...false)` returns false (no
  message) if any of: class bit, `rank->expansion` vs `UseCurrentExpansionAAOnly`, `level_req`,
  cost > unspent, or unmet `aa_rank_prereqs`. The picker pre-filters on affordability + prereqs so
  only grantable AAs are ever offered.
- **Orphaned zones = "characters time out on login."** If you restart `eqlaunch` while old `zone`
  procs are still alive, those zones become orphans still registered with world; world routes clients
  to a dead zone → login timeout. Tell-tale: many `zone` procs with **pids lower than the current
  `eqlaunch`**. Fix: kill ALL `zone` + `eqlaunch` + `world`, then restart world → eqlaunch (exactly
  one eqlaunch). Prefer **not** restarting eqlaunch at all — kill zones and let the current eqlaunch
  respawn them.
- **Detach long-running procs or a shell timeout kills them.** Start world/eqlaunch with
  `setsid nohup ./world > log 2>&1 < /dev/null &` — a plain `nohup … &` still dies when the foreground
  shell that launched it times out (SIGTERM hits the whole process group).
- **Bazaar seller double-pay** — the native `ServerOP_BazaarPurchase` flow **already pays the seller**
  (`trader_pc->AddMoneyToPP` in the seller's zone). Do NOT also pay in `BuyTraderItemOutsideBazaar`;
  the single authoritative payout is the **world** handler (online → route to seller's zone; offline →
  escrow bucket). See §13.
- **Trader item serials aren't stable across relog** (`GetNextItemInstSerialNumber` is a runtime
  counter). Anything that must survive a relog (e.g. the shop's `item_sn` handle) needs a persistent
  key — the player shop uses a `shopsn_<charid>` counter bucket, not the item serial.

## 11. Discovered-PoK-book travel network

A personal fast-travel system over the Plane of Knowledge books.
- **Discover** — `global_player.lua` `event_click_door`: clicking any door whose destination is
  `poknowledge` calls `pok_travel.discover(zone)` AND **`return 1`** (cancels the door's `HandleClick`
  so the book does NOT teleport you to PoK — `Handle_OP_ClickDoor` skips `HandleClick` when the event
  returns non-zero). Books are pure discovery markers.
- **Ungated** — the PoK books are PoP-era (`doors.min_expansion 4/7/9`), so on a Classic server
  (`CurrentExpansion=0`) they'd be content-filtered out. `custom/sql/aotv4_pok_travel.sql` sets
  `min_expansion=max_expansion=-1` on all `dest_zone='poknowledge'` doors so they always spawn.
  Doors load from the DB at zone boot (NOT shared memory) → restart zones after.
- **Lua** — `pok_travel.lua` + `pok_portals.lua` (gen: short→{id,x,y,z,h,long} for the 33 book-zones).
  Discovered set in bucket `pok_found_<charid>`. Chat protocol: `PORTALDATA short|Long^…` out; `/say
  portalgo <short>` travels (`MovePC`); `/say portalreq` = silent list refresh (dll); `/say portals` =
  list + saylinks (no-mod fallback).
  - ⚠️ **Landing-coord rule (regeneration):** landing = the PoK-side **return** door's dest **when one
    exists** (game-tuned, lands ~by the book). **14 of the 33 zones have NO PoK→zone return book**
    (bazaar, feerrott2, freeportwest, freportw, innothule, innothuleb, mistythicket, shadowrest,
    steamfont, steamfontmts, tox, toxxulia, weddingchapel, weddingchapeldark) — for those the landing
    MUST be the zone's **own book position** (`doors.pos_x/y/z` where `dest_zone='poknowledge'`), NOT
    the zone **safe point**. The generator originally used the safe point → those 14 dropped you at
    map-center; they're hand-corrected in `pok_portals.lua` (each tagged `-- book … (no return door)`).
- **Opening** — clicking a PoK book opens the window: `event_click_door` calls `pok_travel.open`
  which sends **`PORTALOPEN`**; the dll's chat detour sets `g_portalVisible=true`. (So you open the
  menu *at a book* — no hotkey.) `pok_travel.discover` still fires on the same click (attune if new).
- **dll window** (`core_spellwindow.cpp`, `arePortalWindowEnabled`): a 3rd layered window (teal).
  Scrollable list (mouse-wheel + title-bar **Up/Dn** page buttons, `PORTAL_VIS=9`) of discovered
  zones; a row's **Travel** button queues `g_portalPendingShort` → `/say portalgo <short>` on the
  game thread. **Closes** on: right-click, Travel, EQ losing foreground, a **20s idle timeout**
  (`PORTAL_TIMEOUT_MS`, reset on any interaction → covers walking away), or `PORTALCLOSE` which
  `event_enter_zone` sends on any zone change. Swallows `PORTALDATA`/`PORTALOPEN`/`PORTALCLOSE` +
  `portalgo`/`portalreq` echoes.

## 12. Expansion-unlock-on-AA-maxout (era progression) — ⚠️ trigger orphaned 2026-07-25

> `era_system.check_unlock` had exactly ONE caller: `aa_choice.handle_say` (after a successful AA pick).
> Random AA is retired (§6), so **nothing advances `aotv4_era` any more** — the server is frozen at
> whatever era the bucket holds (Classic unless it was already advanced). Everything else in the module
> still works and is still called: `clamp_level` (level cap, from `event_level_up`) and `sync_zone`.
> A replacement unlock condition needs designing — achievements (§15) are the obvious candidate, since
> the AA-maxout test is meaningless once AA is base EQ.


A server-wide progression race: the **first character to MAX every AA in the current era's tier
unlocks the next expansion for EVERYONE** (Classic → Omens of War). Plan/ceilings/cadence live in
`AOTV4_EXPANSION_PLAN.md`. Source of truth = the **GLOBAL** data bucket `aotv4_era` (0..8); every
zone reads it, so gating is consistent and survives restarts.

- **`era_system.lua`** — the era table (`name/expansion/cap/cap_exp` per era 0-8), `current()`,
  `level_cap()`, `check_unlock(client)` (maxout test + advance + `eq.world_emote` broadcast +
  `eq.set_rule` content bump), `clamp_level(client)`, `sync_zone()`.
- **Maxout test** = *every AA with `era <= current` is at its max picker rank* (`aa.mr` in the
  pool). This is authoritative — robust to the picker's floored-cost accounting and escalating
  native rank costs (so `GetSpentAA()` is NOT used as the threshold).
- **`aa_pool.lua`** is now **era-tiered**: 353 AAs (Classic..OoW), all at `pool[1]`, each with
  `era` (0-8 unlock tier) + `mr` (max picker rank). Regenerated from the DB; greedy-filled by
  native expansion to the plan's cumulative ceilings. `aa_choice.gather_affordable` offers only
  `aa.era <= era_system.current()`.
- **Level cap** is enforced LIVE in `global_player.event_level_up` via `era_system.clamp_level`
  (Lua, no rule restart): holds the character at the era cap and pins exp at `cap_exp` so a death
  banks the tuned amount (not an ever-growing pile from grinding at cap). The cap also gates the
  **spell** pool for free (the spell picker only offers <= player level).
- **Detection** is in `aa_choice.handle_say` after a successful pick → `era_system.check_unlock`.
  The first char to max the current tier advances `aotv4_era`; others already past that bar don't
  re-trigger (next ceiling is higher). A single char can cascade tiers across picks (still gated by
  their own death-spending).
- **Live vs restart**: level/spell/AA gating is fully live off the bucket. The **content** side
  (zones/items/doors that filter at zone BOOT via `Expansion:CurrentExpansion`) only fully opens
  after the rule is persisted + world restarted — `aa_choice` bumps the rule live per-zone as a
  best-effort, and `event_enter_zone` calls `sync_zone()`. See `aotv4_aa.sql` step 4.
- **DB**: `aotv4_aa.sql` tags the 353-AA set Bard (`classes|128`) + `grant_only`, lowers
  `level_req 1-70 → 1`, sets `CurrentExpansion=0`. AAs load at zone boot (not shared memory) → a
  **zone restart** applies DB changes. **Reset the race**: `DELETE FROM data_buckets WHERE` key
  `='aotv4_era'`.

> ⚠️ Keep the `aa_pool.lua` generator's WHERE in step with `aotv4_aa.sql` step 2 (same filter:
> enabled, type 1-5, real-named, `expansion 0-8`, first-rank `level_req <= 70`, dedup by name) so
> **pool == tagged set**. An AA in the pool but not tagged Bard → its grant silently fails.

## 13. Player shop (permanent escrow trader) — `/trader` (was `/shop`)

An AFK player shop that works from **any city** (no Bazaar zone, no NPC, no Trader's Satchel), documented
in full in **`HANDOFF.md`**. Items are **escrowed** (they leave your bags when listed → no dupe), searchable
via the native `/bazaar`, delivered by parcel, and paid to the seller on sale (instantly if online, else
banked to next login). Managed by a **three-tab native SIDL window** (`/trader`): **Set Price** (price an
inventory item into a persistent price book + see that item's price history) / **List Item** (choose a
quantity and add a priced item to your shop) / **My Shop** (pull a listing back).

- **Protocol (chat, dll swallows all of it):** `/trader` → dll rewrites to `/say shopopen`; server replies with
  four lines — `SHOPINVDATA slot|itemid|name|vendor|stackqty|bookprice^…` (inventory + its saved price),
  `SHOPBOOKDATA itemid|name|price^…` (the price book), `SHOPLOGDATA itemid|name|old|new|when^…` (price-change
  history, newest first), `MYSHOPDATA itemsn|itemid|name|cost|qty^…` (current listings). dll → `/say
  shopsetprice <itemid>:<copper>` (persist a book price), `/say shopadd slot:qty,…` (escrow; **price comes
  from the book, NOT the command** — an unpriced item is rejected), `/say shoppull <itemsn>` (unlist →
  cursor), `/say shoprefresh`.
- **Price book (2026-07, `zone/trading.cpp`):** per-character `item_id→price` persisted in the
  `shopbook_<charid>` data bucket; a capped (20-entry) change log in `shoplog_<charid>`. `SetItemPrice`
  (upsert + append log), `GetPriceBook`, `GetPriceLog` — all Lua-bound (`lua_client.*`). `GetSellableInventory`
  now carries the book price per row; `AddItemsToShop` reads the price from the book (so `shopadd` sends only
  `slot:qty`). Rationale: you set prices once, then list from them without re-typing. **Needs a zone rebuild.**
- **Server (`zone/trading.cpp`, `Client::` + Lua-bound):** `GetSellableInventory`, `AddItemsToShop`
  (**insert rows FIRST, then delete items** — loss-safe; unique `item_sn` from the `shopsn_<charid>`
  counter because item serials change across relog), `GetMyShopListing`, `PullShopItem`. Rows persist in
  the `trader` table across logout/reboot (searchable while offline).
- **Payout is ONE place — `world/zoneserver.cpp` `ServerOP_BazaarPurchase`:** seller online (any zone) →
  native `trader_pc->AddMoneyToPP` in their zone; seller offline → bank `shop_escrow_<charid>` (paid on
  login by `bazaar_broker.pay_escrow`). ⚠️ **Never also pay in the zone buy path** — that was a 2× bug.
- **Lua:** `quests/lua_modules/bazaar_broker.lua` (backend, no NPC despite the name) + `global_player.lua`
  (`event_say` → `handle_global_say`; `event_connect` → `pay_escrow`).
- **dll (`core_spellwindow.cpp`):** the two-tab window (reuses the coin-field/scroll code); shared
  `DrawCloseX`/`CloseXRect` put a red **[X]** on the Journal/Portal/Shop windows; `IsInGame()`
  (`pinstLocalPlayer` @ `0xDD2630`) gates every overlay `WM_TIMER` so they **auto-hide off char-select**.
  `IsAoTOverlay(fg)` (window-class prefix `AoT`) makes the overlays treat **each other** as the same app
  in that `active` check, so focusing one overlay doesn't hide the others — **multiple overlays coexist**.
  The shop window is draggable across its whole top strip. `/shop` intercept is in `core_bazaar.h`
  (`areTradeAnywhereEnabled=true`).
- **Native trader window ANYWHERE — attempted, NOT viable (2026-07). The `/shop` window is the answer.**
  We tried to open the real RoF2 trader window (`CBazaarWnd`) outside the Bazaar and it **does not render**,
  even though the window *object* exists everywhere. Findings (so nobody re-treads this):
  - `pinstCBazaarWnd` @ `0xD1FCB0` / `pinstCBarterWnd` @ `0xF70CF0` are **non-null in all zones** (runtime
    Ctrl+B vtable dump) — built at UI-load. `/trader`→`0x4EAB40`, `/buyer`→`0x4E6F60` (from `__CommandList`
    @ `0xACD5A8`); the handler has no zone check — it guards `window+0x1D4` then calls the show wrapper
    `vtable[0x90]`=`0x864100` → `CXWnd::Show(1,1,1)` (`vtable[0xd8]`).
  - **But calling Show (via the handler OR directly, bypassing the `[0x1d4]` gate) shows NOTHING outside the
    Bazaar** — TESTED in-game (Ctrl+T and a `/trader` intercept both fail; `/shop` works, so the dll/detour
    is fine). `[0x1d4]` is the window's "active" state (written by Bazaar UI code @ `0x652BBA`/`0x654D28`;
    Deactivate = `0x63ADB0`). The window only renders after `CBazaarWnd::Activate` runs **on Bazaar entry**
    (positions it, wires it into the Bazaar screen). Show just re-shows an already-activated window; outside
    the Bazaar it was never activated, so nothing draws. Replicating that activation is a large, fragile,
    untestable-without-rebuild client patch — **not worth it**, especially since the Bazaar zone may not exist.
  - Leftover harmless code: the `/trader`/`/buyer` intercepts in `core_bazaar.h` (call Show → no-op visually)
    and the Ctrl+T/Ctrl+Y/Ctrl+B diagnostics in core_spellwindow.cpp. Kept as documentation of the dead end.
  - **Resolution:** the custom **`/shop`** window already delivers the full requirement (offline trader, any
    zone, NO Bazaar zone needed, global `/bazaar` search, escrow/parcel payout). It was **restyled to the
    EQ-native look** (2026-07, `VendorPaint`/`VDrawBtn`/new `VBevel` + title-bar in core_spellwindow.cpp):
    gold/tan metallic window frame (dark→bright→mid gold band), near-black background, a **gold "Trader" title
    bar** (`VTBAR_H`=24, embossed strip + gold underline, `[X]` in its top-right), sunken recessed list panel +
    raised header strip, EQ beveled-grey buttons, sunken coin input fields, alternating row bands, cream/gold
    text. ⚠️ **Layout uses shared helpers** (`VTabBtn`/`VUpBtn`/`VField`/`VActBtn`/`VRowY`/`VLIST_Y`/`hy`) for
    BOTH paint and click hit-testing — all include `VTBAR_H`, so the title-bar shift stays consistent; the drag
    region (WndProc) is the only raw-constant spot. `VDrawBtn`/`VBevel` are shared with the search window.
    **Needs a dll rebuild.**
  - **SUPERSEDED by a native SIDL window (2026-07).** The GDI restyle only *imitates* EQ chrome; the real fix
    (asked "why not use the window XML as reference?") is a **native `CCustomWnd`** — same approach as the
    achievement window (§15), which renders with the client's own UI engine so it looks truly native AND
    works in any zone. Built from the reference XMLs the user supplied (`/src/EQUI_BazaarWnd.xml` etc.):
    - **`EQUI_ShopWnd.xml`** (`aotv4_client_install/uifiles_default/`) — Screen `item="ShopWnd"` (`WDT_Def`
      frame, Titlebar/Closebox, title "Trader"), a `Listbox` (`WDT_Inner`, cols Item/Qty/Price), 4 coin
      `Editbox`es, buttons Set Price / Add to Shop / Pull Item / Refresh + Add-Items/My-Shop tabs.
    - **`ShopWnd : CCustomWnd`** (core_spellwindow.cpp) — REUSES the GDI backend (same `g_vendor*`/`g_my*`
      state + `VendorQueue`/`VendorDoAdd`); `WndNotification` maps clicks → the same `/say shopadd/shoppull/
      shoprefresh`. `EnsureShopWindow` wires `ppSidlMgr`/`ppWndMgr` from client globals (the §15 fix).
      `SHOPINVDATA` now calls `ShopSidlShow()` (was `g_vendorVisible=true`); `MYSHOPDATA`→`ShopWndRefreshIfOpen`;
      teardown `ShopWndOnUiReset()` is called from the achievement `CleanGameUI`/`ReloadUI` detour (it owns
      those). Old GDI overlay stays in the dll but is never shown.
    - ⚠️ **SIDL editbox I/O gotchas (learned the hard, crashy way) — THREE traps:**
      1. **Raw struct member offsets are WRONG for this client build.** Accessing `((CEditWnd*)w)->InputText`
         reads empty and `w->WindowText` reads a garbage pointer → `GetCXStr` on it **CRASHES**. `EQClasses.h`
         member offsets don't match this RoF2 build. Only **address-mapped METHODS** (`FUNCTION_AT_ADDRESS`) are
         reliable. So **read** an editbox via the method `w->GetWindowTextA()` (`0x411190`) and pull chars from
         the returned `CXStr` (which wraps one `CXSTR*`): `CXStr s = w->GetWindowTextA(); PCXSTR raw =
         *(PCXSTR*)&s; if (raw) GetCXStr(raw, buf, n);` (`ShopGetInt`). **Set** labels via `SetWindowTextA`
         (`ShopSetLabel`). Don't pre-fill editboxes (and `CXStr::operator char*` isn't address-bound — never cast).
      2. **Clicking a coin field clears the Listbox selection**, so at button-click `GetCurSel()` is −1 and any
         "apply to selected row" no-ops (symptom: Set Price never changes the price). Remember the row on
         list-click in `m_sel` and use `SelRow()` (single-item fallback), NOT live `GetCurSel()`.
      3. `/echo` is NOT a valid EQ command (that's an MQ2 thing) — for in-window feedback use `WriteChatColor`
         (MQ2Main.h / MQ2PluginHandler.cpp is in the build) or set a label; `/echo` returns "invalid command".
    - **Stackable quantity (2026-07):** `Client::AddItemsToShop` (trading.cpp) lists a **partial stack**
      (`item_charges = list_qty`; `DeleteItemInInventory(slot, del_qty)` where `del_qty` 0 = whole stack) so you
      can list N of a stack and keep the rest. Client shows real Qty + has a `SHW_Qty` field (default = full
      stack); state in `g_vendorStack`/`g_vListQty`/`g_myQty`. Field splitting is `VSplitN(s, out, n)` (the shop's
      old `VSplit5` was renamed — a duplicate def collided with the loot window's `VSplit5`). Buyer still
      purchases the whole listing (per-unit buys would be a bigger change). **Needs a zone rebuild + dll rebuild.**
    - **Three tabs (2026-07, `ShopWnd` in core_spellwindow.cpp):** `g_vendorTab` 0/1/2 = Set Price / List Item /
      My Shop (`SHOP_TAB_*`). One shared `SHW_ItemList`; `ApplyVis()` shows/hides each tab's controls via
      `CXWnd::Show`. Set Price → `shopsetprice`; List → `shopadd slot:qty` for the selected row (`VendorDoAdd`);
      Pull → `shoppull`. History listbox `SHW_HistList` is filtered to the selected item's id (`FillHistory`),
      relative "when" via `ShopWhenStr`. EQUI_ShopWnd.xml is CX564×CY600 (single shared item list -- a
      per-tab second listbox for filling List/My Shop was tried and caused a UI-load error, so it was
      dropped; keep the layout to ONE list of each type). **Needs a dll rebuild.**
    - **Command: `/trader`** (core_bazaar.h) routes to `/say shopopen` → server SHOPINVDATA → `ShopSidlShow`.
      The old `/shop` and `/buyer` intercepts were **removed** (the native trader/buyer windows can't render
      standalone).
    - **Client install:** `aotv4_client_install/SHOP_WINDOW_INSTALL.md` — copy `EQUI_ShopWnd.xml` to
      `uifiles/default/` + `<Include>` it in `EQUI.xml`, rebuild the dll. Like the achievement window, expect
      a possible iteration to nail the SIDL wiring.
- ⚠️ **Bazaar search MUST be global — `common/bazaar.cpp` `Bazaar::GetSearchResults` (2026-07 fix).** Escrow
  sellers list from ANY zone (the `trader` row's `char_zone_id` is wherever they stood — never a single Bazaar
  zone), so the STOCK scope filters break trader-anywhere: `Local_Scope` restricts to the searcher's zone, and
  the `else if (trader_id > 0)` branch restricts to one `char_id`. RoF2 sends `Local_Scope`/`AllTraders_Scope`
  and populates `trader_id` from the selected trader, so **stock scoping shows each player only their own /
  their-zone listings** (symptom: "others can't see my listing; needs a relog"). Fix: for the non-alternate
  path, add **no** trader/zone filter — `search_criteria_trader` stays `"TRUE"` → all listings match (item
  name/type/cost filters still apply). Keep the NonRoF per-trader-inspect path + the (rule-gated, off) shard
  path intact. The buyer's trader **list** was already global (`GetDistinctTraders`, no zone filter when
  `UseAlternateBazaarSearch=false`). **Needs a zone rebuild** (bazaar.cpp is in `libcommon`).
- **Live visibility on list** is already wired: `AddItemsToShop` calls `SetTrader(true)` +
  `SendBecomeTraderToWorld(this, TraderOn)` → world fans `ServerOP_TraderMessaging` to every zone
  (`ZSList::SendPacket`) → each zone pushes `AddTraderToBazaarWindow` to all online RoF2 clients. So a listing
  appears in others' open bazaar windows without a relog — but the buyer must still click **Search** (the
  bazaar has no passive item feed). This only *worked* once the global-search fix above unmasked it.
- ⚠️⚠️ **The `trader` table is PERMANENT escrow storage — NEVER truncate or wholesale-delete it (2026-07).**
  A listed item is removed from the player's satchel and lives ONLY as its `trader` row, so any stock code
  that clears the table destroys REAL items (not a runtime cache like on live EQ). Five stock deletion paths
  were closed — do not reintroduce them:
  1. **`Database::ClearTraderDetails`** (`common/database.cpp`) — ran `TRUNCATE TABLE trader` on **every world
     boot** (`world_boot.cpp`), wiping ALL shops on any restart. Now only clears the transient
     `active_transaction` lock.
  2. **`Client::TraderEndTrader`** (`zone/trading.cpp`) — the stock `!persist_listings` branch did
     `DeleteWhere(char_id)`; it's called on **zoning, camp, inventory moves, invalid-item, …** (default flag
     = delete). Delete is disabled — end-trader only flips live state now.
  3. **Trader's-Satchel move** (`zone/inventory.cpp`) — moving any item in/out of a satchel `while IsTrader()`
     ended trading (→ deleted). The whole block is removed: escrowed items aren't in the satchel, so staging
     the next item to list must not disturb the shop.
  4. **Native satchel relist** (`TraderStartTrader`) / **price=0 unlist** (`UpdateTraderItemPrice`, zonedb.cpp)
     / **Bazaar-zone cycle** (`CheckToClearTraderAndBuyerTables`, entity.cpp) — dormant native-window paths,
     defensively neutered so they never wipe escrow.

  Listings are removed by EXACTLY three legit paths: **unlist (`PullShopItem`), a sale, and login
  reconciliation (`ReclaimOfflineShop`)**. All fixes are **world + zone** binaries → need a restart/cutover.
- **Do NOT resurrect** (removed on request): the Bazaar Broker NPC (2000050), the Shopkeeper stand-in NPC
  (2000051), the Trader's-Satchel `vpset/vshop` flow, or `Bazaar:UseAlternateBazaarSearch` (keep it false —
  it's Bazaar-zone-sharded, the opposite of trader-anywhere).

## 14. All classes unlocked (was Bard-only) — 2026-07 pivot

Everyone used to be forced to Bard; now all 16 classes are playable, with the 4 pure-melee classes made
into casters. Three layers had to agree (like combat skills, §4):

**(a) Client — `core_allcasters.cpp/.h`.** Its OWN translation unit, OWN detours (not piggy-backed on the
spell-window/tradeskill hooks), gated by `areAllClassesCasters` (`_options.h`). RoF2 image base `0x400000`,
addresses rebased at runtime. Four detours:
- `EQ_Character::IsSpellcaster` (**0x443F50**) → always **1**: spellbook + spell-gem bar + casting for every
  class (RoF2 hides them for Warrior/Monk/Rogue/Berserker via a class→bool table @ 0x443f90).
- `EQ_Character::Max_Mana` (**0x581E60**) → a melee `0` return becomes `level * AOTV4_MELEE_MANA_PER_LEVEL`
  (**=40**), so the mana VALUE scales like a hybrid. **Must match the server formula** (see (b)).
- mana-gauge predicate (**0x59FB90**) → always **1**: `CPlayerWnd::Draw` (0x718cf0) shows the mana GAUGE
  (`CPlayerWnd+0x22c` "PlayerMana") only when this class→bool predicate is non-zero — forcing 1 renders the
  blue bar for melee. (A SECOND caster test, distinct from IsSpellcaster.)
- `EQ_Spell::IsBardSong` (**0x432960**, `bool __stdcall(SPELL* spell, int class)`, `ret 8`) → skill-gated:
  mirrors the server's `IsBardSong` so the **reward spells cast as normal spells, not songs, even for a Bard**.
  Stock logic = "caster is a Bard AND the spell has a Bard level" (never checks skill); our reward spells
  carry a Bard level (so a Bard can memorize them) but use **skill 98**, a non-song placeholder. Detour keeps
  the stock result but only genuine song skills stay songs: **Brass 12, Singing 41, Stringed 49, Wind 54,
  Percussion 70** (SPELL::Skill is a BYTE @ **+0x270**; per-class level array @ +0x246, Bard @ +0x24e).

**(b) Server mana.** `zone/client_mods.cpp CalcMaxMana`: the pure-melee else-branch = `GetLevel() * 40`.
Keep this constant in step with the dll's `AOTV4_MELEE_MANA_PER_LEVEL` so the client gauge max == the server
mana ceiling. (Server + client use different stat formulas; a shared `level*constant` keeps them in lockstep.)

**(c) Character creation.** `char_create_combinations` restored to **all 16 classes** (was class-8-only) —
the world loads it at boot to both drive the create-screen UI (greying out disallowed classes) and validate
create requests. Source: `custom/sql/aotv4_all_classes_creation.sql` (replaces the retired
`aotv4_bard_only_creation.sql`). **World restart** required (combos load at world boot). The Bard-force block
in `global_player.event_connect` is removed — characters keep their created class.

### Custom reward spell set (43000-43149 offered + 43150+ helpers)
- Generated by `.devcontainer/custom/spells/gen_spells.py` from `spell_design.csv` (score-based auto-tiering
  across levels 1-30, `LEVEL_CAP=30`). Spells use **skill 98** (a non-song placeholder; server `IsBardSong`
  is already skill-gated). Class levels opened to all 16 classes (offer level = `classes8`).
- ⚠️ **Renumbered 50xxx → 43xxx** (RoF2 caps spell LINKS and the spellbook packet at **id < 45000**; the
  custom set was originally at 50000+). Trigger/proc refs live in `effect_base_value`/`effect_limit_value`
  and must be renumbered too (Moonfire's heal @ limit2, Firefist/Chi Block @ limit1 were missed once).
- ⚠️ **`gen_spells.py` is DESYNCED from the live DB** — still emits 50xxx and lacks the live fixes below.
  **Do a source-sync before ever regenerating** or it reverts everything. (Memory: `gen-spells-source-sync`.)
- **Two description sources, keep in sync:** the level-up **picker** reads `spell_desc.lua` (server sends it
  via `SPELLDESCDATA`); the in-game **spellbook** reads `db_str` (via `descnum=id`). ⚠️ A literal `%` in a
  description renders as garbage (`0:00:00`) — the client treats `%` as a format token; spell out "percent".
- **Descriptions live in:** `spell_design.csv` (master) → `spell_desc.lua` (gen) → `db_str` (live). Client
  needs `spells_us.txt` (class levels + cast times) + `dbstr_us.txt` (descriptions) re-exported +
  installed; the current bundle is dropped at `C:\AoTv3\AoTv4\aotv4_client_install` (== `/src/aotv4_client_install`).

### Custom melee mitigation (why "deflected by your armor" spams)
`zone/attack.cpp Mob::MeleeMitigation` (~1063) replaces stock AC mitigation with an **AC-vs-offense roll**
(`AoT:Mit*` rules). A full-mitigation roll (`rolled_mit >= 1.0`) zeroes damage and prints **"…was deflected
by your armor!"** (line 1156 — uses `GetName()`, the raw `a_cave_rat009`; should be `GetCleanName()`). A
high-AC char vs a weak mob deflects nearly every hit → message spam (it's on the filterable `OtherMissYou`
channel). Rune/mitigation buffs (SPA 55/78/161/162/163) DO still apply — via `ReduceDamage` (line 4331,
after MeleeMitigation). ⚠️ **% mitigation is imperceptible vs the tiny post-mitigation numbers** (`3 * 1/100 = 0`);
for low-level buffs prefer a **flat per-hit cap** — SPA 162 `base=100, limit=N` subtracts exactly N/hit
(how Passive Protection was fixed). Damage shields (SPA 59) use a **negative** base (native convention).

## 15. Achievements + class-aura rewards — 2026-07

A DB-defined **achievement system** (ported/adapted from the `Barathos/EQEmu-feature-achievements` fork)
whose rewards can **scribe a class aura**. The client shows a **native SIDL window**; the server owns all
progress, completion, and validation. Same "server → `ACH|...` chat line → dll → window" pattern as our
other overlays (§3/§6/§11/§13).

**Server (`zone/achievement_manager.cpp/.h`, singleton `achievement_manager`).** DB-defined objectives
(`level`/`zone_visit`/`task_complete`/`skill`/`*_kill`) tracked per character; on completion, rewards queue
(auto-claim grants immediately). Gameplay hooks call it: `attack.cpp` `NPC::Death` (3 kill sites),
`client.cpp` `SetSkill`, `exp.cpp` `SetLevel`, `client_packet.cpp` `CompleteConnect`, `task_client_state.cpp`
`IncrementDoneCount`. `#ach [window|status|categories|category|detail|rewards|claim|check]` command
(`gm_commands/achievements.cpp`). **AoTv4 additions:** an **`any_kill`** objective type (kill anything) and
**`class_mask` enforcement** in `ProcessMatchedObjectives` (`GetPlayerClassBit`, `0`=any) so per-class
achievements only advance for that class; a **`scribe_spell`** reward type in `AwardQueuedReward` (reward_id
= a spell id → scribes it into the spellbook; idempotent).
- **DB**: `custom_achievement_*` tables via `common/database/database_update_manifest_custom.h` (custom
  migrations v1-4); `common/version.h` `CUSTOM_BINARY_DATABASE_VERSION = 4`. **World applies the migration
  at boot** (not zone). Reward types: title_text/suffix/set, item, currency, coin, live_item_request,
  **scribe_spell**.
- ⚠️ **Migration pre-backup needs DB creds.** World auto-dumps the DB before applying migrations via
  `mysqldump --defaults-extra-file=login.my.cnf`; it builds that cnf from **`content_database`** creds
  (`IsDumpContentTables`). If `content_database` is absent/blank, the cnf gets empty creds → mysqldump
  auths as the OS user → **"Access denied" → migration aborts**. Fix (done): a `content_database` block
  (peq/peqpass) in `eqemu_config.json`. One-off unblock: write `login.my.cnf` (`[mysqldump]` peq creds)
  **read-only** so `BuildCredentialsFile` can't overwrite it.

**The 16 class auras (the reward).** One per class, granted by "*Class*: First Blood" (kill anything **as
that class** → scribe that class's aura). Auras (§ generic mechanic): a **cast spell** (SPA 351, self, spawns
the aura) + an **`auras` row** (cast → aura NPC → effect spell, `aura_type=1` OnAllGroupMembers = **group-
shared**, distance 60) + an **effect spell** (the group buff). Our set: cast spells **43500-43515**, effect
spells **43550-43565**, aura NPCs **2000100-2000115**; achievements **9000001-9000016** (category 9000),
objectives `any_kill` + class_mask bit, rewards `scribe_spell` → the class's cast spell. **1-active is the
default** (`GetAuraSlots()` = base 1; never grant `aura_slots` bonuses). Generated by
`/tmp` scripts (clone templates 8468/8469/2000003); to regen see `custom/spells` history.

**Client dll (`core_achievements_native.cpp/.h`, flag `areAchievementsNativeEnabled`).** The reference was a
standalone dll that re-detoured `dsp_chat`/`InterpretCmd` — **which we already own** — so it was refactored
to `.cpp`+`.h` (single definition across our TUs) and **routed through our existing detours**: our
`dsp_chat` calls `NativeAchievementParseTransport` (swallow `ACH|`) + swallows the `#ach` echo; our
`InterpretCmd` (`core_bazaar.h`) calls `NativeAchievementHandleLocalCommand` + `NativeAchievementRewriteCommand`
(`/ach …` → `/say #ach …`). `InitAchievementsNative` installs ONLY the `CDisplay::CleanGameUI`/`ReloadUI`
detours (window lifecycle) — conflict-free only because `isMQInjectsEnabled=false` (else MQ2's CleanUI hook
takes those addresses). The window is a real SIDL `CCustomWnd` (`NativeAchievementWnd`), created on demand.
- ⚠️ **This dll had NEVER used native SIDL windows** (all our overlays are self-drawn GDI), so the MQ2 UI
  managers `ppSidlMgr`/`ppWndMgr` weren't reliably wired — `CCustomWnd`/`NativeAchievementEnsureWindow`
  silently returned at `if (!pSidlMgr || !pWndMgr)` and **the window never appeared** (log stopped at
  `ACH|window|show` with no "creating achievement window"). Fix: in `NativeAchievementEnsureWindow`, set
  them directly from the client globals if unset — `ppSidlMgr = (CSidlManager**)((0x15D3D08-0x400000)+baseAddress)`,
  `ppWndMgr = (CXWndManager**)((0x15D3D00-0x400000)+baseAddress)`. Any future native-window feature in this
  dll must do the same. `native_achievements.log` (in the EQ root) is the dll's own debug trace — the fastest
  way to diagnose the window (it logs every `ACH|` line received + the `EnsureWindow` pointer values).
- **Client install** (see `aotv4_client_install/ACHIEVEMENT_WINDOW_INSTALL.md`): rebuild OUR `dinput8.dll`;
  copy `EQUI_NativeAchievementWnd.xml` + `Achievement_*.tga` to `uifiles/default/`; add
  `<Include>EQUI_NativeAchievementWnd.xml</Include>` to `EQUI.xml`. The `#ach` **text** commands work
  without any client change; the UI files are only for the native window.

**Filling out the categories (2026-07).** The seed (from the fork) populates Classic→PoP well but left header
categories showing 0/0 and later in-scope tiers empty. Fixes:
- **Parent-count rollup (code):** `LoadCategorySummaries` (`achievement_manager.cpp`) now rolls child-category
  achievements up onto their parent (`a.category_id = c.id OR a.category_id IN (children of c, enabled)`), so
  **Exploration/Hunter/Slayer/Progression/Tradeskill** headers show the sum of their leaves (277/136/385/490/72)
  instead of 0. Leaves have no children so they stay direct-only. **Needs a zone rebuild + restart.**
- **Beyond-OoW hidden (SQL, `custom/sql/aotv4_achievements_fill.sql`):** the era system caps at OoW, so the
  Dragons-of-Norrath..Rain-of-Fear Exploration (cats 209-219) + Hunter (3309-3319) categories are set
  `enabled=0` (unreachable zones; reversible if the server ever extends). `LoadCategorySummaries`'
  `WHERE c.enabled=1` drops them from the window.
- **Server Custom (cat 8), same SQL file:** class-agnostic **kill milestones** (`any_kill` 100/1k/10k/50k →
  Bloodthirsty/Warmonger/Harbinger/Reaper). `any_kill` counts every kill across the char's life (survives the
  roguelite death reset). **NB: a character is ONE class**, so an "all 16 auras" achievement would be
  unearnable — Class-Mastery auras are per-class only.
- **GoD/OoW Hunter (generated, `custom/sql/aotv4_achievements_hunter_god_oow.sql` via `/tmp/…/gen_hunter_god_oow.sh`):**
  cats 3307/3308 filled from **OUR DB's named mobs** (not Live lists — those wouldn't match our spawns). Named =
  npc_types whose name starts `#` (excl. `#Trigger`/`#Zoner`/level-99 system NPCs); `a_`/`an_`/lowercase = trash.
  One `npc_name_kill` objective per distinct clean-name (top 12 by level), `zone_id` = the sub-zone it spawns in.
  ⚠️ **`npc_name_kill` matches `o.zone_id == <zone killed in>` AND `LOWER(o.target_name) == LOWER(GetCleanName())`**
  — `GetCleanName`==`CleanMobName` (`common/strings_legacy.cpp`): `_`→space, keep `[A-Za-z`+backtick]`, **strip
  `#`/digits/apostrophes**, then we LOWER. bonzz.com gave the canonical zone list + target counts but **no mob
  names** (they live on eqresource/raidloot) — irrelevant since we source earnable targets from our own spawns.
- **Epics (cat 7, `custom/sql/aotv4_achievements_epics.sql`):** 16 achievements (one per class, epic 1.0
  weapon), completed by a **new `item_receive` objective type**. `ProcessItemReceive(client,item_id)`
  (`achievement_manager.cpp`) matches `item_receive`+`target_id`; **hooked live** in `Client::SummonItem`
  (quest turn-ins/summons) + `Client::PushItemOnCursor` (loot→cursor), and **backstopped** by
  `ProcessItemInventory` in `RecheckAutomatic` (credits already-owned epics on level/zone/#ach). class_mask
  = `1<<(class-1)` gates each to its class. Epic item ids resolved from OUR `items` DB (the weapons that
  actually exist here). **Needs a zone rebuild.**
- ⚠️ **Achievement-id ranges are NOT category-bound — check before inserting.** Zone Slayer occupies
  **700001-700448**; a first pass put Epics at 700001-700016 and `ON DUPLICATE KEY UPDATE` silently hijacked
  16 Zone Slayer rows (their `zone_kill` objectives dangled). Epics moved to **710001-710016** (objectives
  71000001+). Other ranges in use: Character 1xxx/110xx, Exploration 200xxx, Task 300xxx, Tradeskill 46xxxx,
  Creature Slayer 5xxxxx, Zone Slayer 7000xx-7004xx, Server Custom 8000xx, Meta 900xxx, Class Mastery 9000xxx,
  GoD/OoW Hunter 3307xx/3308xx. **Every category is now populated** (Epics 16, Exploration 277, Hunter 136,
  Slayer 385, Progression 490, Tradeskill 72, Server Custom 4, Character 22, Class Mastery 16, Meta 4).
- **Parent headers drill down (code):** the dll shows categories as a flat list with children indented; a parent
  (Hunter/Exploration/…) has NO direct achievements, so selecting it used to show an empty list. `LoadAchievements`
  now returns the union of the selected category + its enabled child categories (grouped by `category_id`), so
  clicking "Hunter" lists all 136. Leaves are unaffected (no children).
- ⚠️ **Live-refresh must stay small — chat-burst drops (fix).** `PushLiveUpdate` originally re-sent ALL ~40
  `ACH|category` lines per completion; the RoF2 chat pipe drops/reorders that burst, losing the trailing
  `ACH|achievement_state` row-flip → "the window updates only sometimes." Now it sends the **state flip first**,
  then only the **affected leaf + its parent** (1 status + 1 state + ≤2 category = ~4 lines). Any per-completion
  push here must stay minimal for the same reason. (A big parent drill-in still sends ~N achievement lines, same
  load as the pre-existing 490-row Task Completion leaf — if a huge list ever truncates, batch it.)
- **Native EQ-menu "Achievements" button is intentionally NOT hooked** — it opens RoF2's stock `CAchievementsWnd`
  (which we never populate), not our window. Redirecting it needs RE of the client's native-open call; by choice
  the window is opened via `#ach`/`/ach` instead.

## 16. Advanced Loot window (`/advl`) — 2026-07

A native SIDL **Advanced Loot** window in **complement mode**: the stock RoF2 loot window still opens
(`MakeLootRequestPackets` is untouched); ours lists the SAME corpse and adds **Loot / Leave / Never**,
**Loot All**, and a persistent **Never** filter list. Same "server → chat line → dll → window" pattern as
§3/§6/§11/§13/§15.

- **Protocol (chat, dll swallows all of it):** server → `LOOTDATA <n>^lootslot|itemid|icon|name|npcname|qty^…`
  (pushed from `Handle_OP_LootRequest`; `qty` = stack size for stackables else 1, appended **last** so an
  older 5-field dll parse still works), `FILTERDATA <n>^itemid|icon|name|rule^…`, `LOOTCLOSE`
  (from `Handle_OP_EndLootRequest` + `event_enter_zone`). dll → `/say alspick <lootslot> loot|leave|never`,
  `/say alslootall`, `/say alsrefresh`, `/say alsfilters`, `/say alsfilterdel <itemid>`.
- **Server (`zone/client_packet.cpp`, `Client::`):** `SendAdvLootData` / `SendAdvLootFilters` /
  `SendAdvLootClose` / `AdvLootSlot` / `HandleAdvLootSay`. The `/say als*` commands are intercepted in
  **`Client::ChannelMessageReceived`** (`zone/client.cpp`, before EVENT_SAY/broadcast) so they never spam
  chat or reach quests — **NOT** in Lua, unlike the other windows. **Needs a zone rebuild.**
- ⚠️ **Looting reuses the NATIVE `Corpse::LootCorpseItem`** — `AdvLootSlot` synthesizes an `OP_LootItem`
  packet and calls it, so lore/cursor/weight/corpse-removal are the stock checks and there is **no bespoke
  item-grant path** (the lesson from the escrow work, §13). It only works while the corpse is actually being
  looted (`LootCorpseItem` requires `IsBeingLootedBy`) — which is why this is complement mode, not a
  standalone accumulating loot list.
- **`lootslot` is the handle**, not an entity id (the dll's `g_lootEid[]` is historically misnamed).
  `Corpse::RemoveItem` does **not** renumber lootslots, so `alslootall`'s slot snapshot stays valid while
  it loots down the list.
- ⚠️ **Two traps when driving `LootCorpseItem` from a server-side batch (both fixed; don't reintroduce):**
  1. **The 10 ms loot throttle is per zone TICK.** `Corpse::m_loot_cooldown_timer(10)` is checked against
     the global `current_time`, which does **not** advance inside one packet handler — so in a tight loop
     the first `Check()` passes and resets it and **every later item fails**, each failure calling
     `SendEndLootErrorPacket` + `ResetLooter()` (Loot All would loot one item and kill the loot session).
     `Corpse::ResetLootCooldown()` (corpse.h, `Timer::Trigger()`) is called between items. Safe because
     `LootCorpseItem` removes the item from the corpse, so a slot can't be looted twice.
  2. **`auto_loot` must be 1.** With `0`, `LootCorpseItem` does `PutLootInInventory(slotCursor,…)` — every
     item piles on the **cursor**, and `Character:CheckCursorEmptyWhenLooting` (default **true**) then
     refuses the next loot *and* calls `ResetLooter()`. With `1` it uses `AutoPutLootInInventory` (free bag
     slot, cursor only as fallback). `alslootall` additionally **stops cleanly** if the cursor is occupied.
  The synthesized `OP_LootItem` carries a **server-space** lootslot; the ACK `QueuePacket(app)` runs through
  `ENCODE(OP_LootItem)` (`ServerToRoF2CorpseMainSlot`), so it keeps the native loot window in sync.
- **Never list** = data bucket `alsnever_<charid>` (CSV of item ids). A filtered item is only **omitted from
  LOOTDATA** — it stays on the corpse and still shows in the native loot window. Nothing is destroyed.
- **dll — its OWN translation unit `core_advloot.cpp/.h`** (2026-07-25; was inside `core_spellwindow.cpp`),
  like `core_allcasters` / `core_achievements_native`. Flag `areLootWindowEnabled` → `InitAdvLoot()`.
  It installs **NO detours**: the dll already owns `dsp_chat` / `InterpretCmd` / `ProcessGameEvents` and two
  modules can't hook the same address, so those call in via `AdvLootParseTransport` (LOOTDATA/FILTERDATA/
  LOOTCLOSE), `AdvLootIsOurEcho` (`/say als*` echoes), `AdvLootShow` (`/advl`, from `core_bazaar.h`) and
  `AdvLootOnUiReset` (from the `CleanGameUI`/`ReloadUI` detour `core_achievements_native.cpp` owns).
  `AdvLootWnd : CCustomWnd` over `EQUI_AdvLootWnd.xml`, with the same `ppSidlMgr`/`ppWndMgr` wiring fix as
  §15. **Static layout only** and **one listbox** (§13). The game-thread command queue is shared as
  `AoTQueueGameCommand()` (`VendorQueue` is now a wrapper on it).
  ⚠️ New `.cpp` files **must be added to `eq-core-dll-vs2022.vcxproj`** or they silently don't compile.
  The old **GDI** loot overlay was deleted (~300 lines) — it had been dead since the SIDL window landed.
- ✅ **CORRECTIONS (2026-07-25), proven by the reference sources in `src.rar` + a binary probe.** Three
  "limits" recorded here were wrong and cost real work:
  - **Native TABS exist.** `TabBox` + `Page` + `TabText` + `TabBorderTemplate`/`PageBorderTemplate` are
    all in RoF2's parser, and `FT_DefTabBorder`/`FT_DefPageBorder` are in our `EQUI_Templates.xml`.
    The Advanced Loot window hand-rolls tabs with a toggle button because this was not known.
  - **MULTIPLE Listboxes per window are fine.** `EQUI_AoTSpellBook.xml` ships four (list + detail,
    twice). The "one listbox" rule in §13 came from a malformed shop XML misdiagnosed as a hard limit;
    it forced the loot Filters view into one shared list with permanently mislabeled headers.
  - **`CListWnd::GetItemData` works** (`spellList->GetItemData(sel)` in the reference). §16's claim
    that it returns 0 for every row was a misattribution: the real fault was unassigned `lootslot`
    values aliasing every handle onto one item (see `Corpse::AssignLootSlots`).
  - Still genuinely unavailable on this build: `GetCurCol`, the `(point,row,col)` `GetItemAtPoint`
    overload, `SetColumnLabel`, and `CTextureAnimation::SetCurCell` (so no per-row item icons).
  - **Verify before believing a limit:** `strings -n 4 eqgame.exe | grep -qx "<Tag>"` answers whether
    the SIDL parser knows a tag. Validate the method with a nonsense tag first.
- ✅ **ALWAYS run `bash aotv4_client_install/validate_ui_xml.sh` before copying any `EQUI_*.xml` into
  the client.** It checks the four things that have actually broken us: `--` inside a comment, tag
  balance, undefined `<Pieces>` targets, and modern-only tags (`<Sortable>`, column `<Tooltip>`) that
  no RoF2-shipped file uses. The `--` bug has cost TWO sessions — the second time it was written into
  a comment that was itself warning about the bug. Do not trust a visual scan.
  (`<Pieces>` also accepts a type-qualified `Screen:NAME` form — the validator strips that prefix.)
- ⚠️ **A `--` inside an XML comment CRASHES the client at UI load** (bit us on `EQUI_AdvLootWnd.xml`).
  Double hyphens are illegal in XML comments; the SIDL parser aborts the **whole file** and the client
  dies before char select. Diagnose with **`UIErrorLog.txt`** in the EQ root — it names the file and the
  exact line (`ParseNodeList() SyntaxError` + `Error reading XML.`). Applies to every `EQUI_*.xml` we ship;
  use commas/single hyphens in comments. Container has no xmllint/python, so scan with
  `sed 's/<!--/«/g; s/-->/»/g' f.xml | grep -n -- "--"` (must print nothing).
- **Client install:** `aotv4_client_install/ADVLOOT_WINDOW_INSTALL.md` (copy the XML to `uifiles/default/`,
  `<Include>` it in `EQUI.xml`, rebuild the dll).

## 17. Database access from Windows + container recovery — 2026-07-24

### Connecting a GUI client (HeidiSQL) to the peq DB
MariaDB runs **inside the dev container** (`eqemu_config.json` → `127.0.0.1:3306`; there is no separate DB
container). To reach it from Windows it is published as a **real Docker port**, exactly like the EQ ports:

- `devcontainer.json` `appPort`: **`"127.0.0.1:3307:3306"`** → HeidiSQL connects to **`127.0.0.1:3307`**,
  user `peq` / `peqpass`. Host-side bind is loopback, so the DB is reachable from Windows only, never the LAN.
- ⚠️ **Do NOT use `forwardPorts` / the VS Code PORTS tunnel for MySQL.** It was `forwardPorts: [3306]` and it
  **does not work**: MySQL is a **server-speaks-first** protocol (the server sends the greeting before the
  client says anything), and the tunnel fails exactly there → HeidiSQL reports **"Lost connection to server at
  'handshake: reading initial communication packet'"**. In the PORTS panel a tunnel shows Origin
  `Dev Containers`; a published port shows **`Statically Forwarded`** (what 5998/5999/7000-7005/9000 use, which
  is why the EQ client always worked). `forwardPorts` was removed so it can't also grab 3307 and shift the port.
- ⚠️ Three things must ALL be true or you get a *different* wall each time:
  1. **`bind-address = 0.0.0.0`** in the container. Stock is `127.0.0.1` (`50-server.cnf:27`), and a published
     port forwards to `eth0` where nothing would be listening. Set via a drop-in
     `/etc/mysql/mariadb.conf.d/99-aotv4.cnf` written by **`postStartCommand`** — an edit made by hand inside
     the container lives on the **container layer** and is silently lost on the next rebuild.
  2. **A `peq'@'%'` grant.** A published port arrives from the **docker gateway (172.17.0.1)**, NOT loopback,
     so the stock `peq'@'127.0.0.1'` user does not match → "Access denied for user 'peq'". Both users now exist.
  3. **Right port.** Connecting to `127.0.0.1:3306` on Windows hits whatever local MySQL owns 3306 (that's WHY
     VS Code picked 3307) — the tell is `Access denied for user 'peq'@'localhost'`, i.e. a *different server*
     answering, not a credential problem.
- `postStartCommand` also **starts MariaDB on every container start**, so it no longer has to be started by hand.

### A rebuild does NOT destroy the DB — the OLD CONTAINER STILL HAS IT
The `aotv4-mysql-data` volume (§2) was committed **Jul 21 02:45** but a mount only takes effect on the next
rebuild, and none happened until **Jul 24 22:31** — so the DB lived on the *container layer* for those 3 days
and the fresh container came up with an empty volume. **The data was never lost**: the previous container was
still there (stopped or even running) with its datadir intact. Before importing any snapshot, **check the old
containers first** — a snapshot import silently costs every day of work since the snapshot was taken.

```powershell
docker ps -a --format "{{.ID}}  {{.CreatedAt}}  {{.Status}}  {{.Image}}"   # incl. stopped; image vsc-aotv4-*
foreach ($c in "<ids>") { docker exec -u root $c sh -c "ls /var/lib/mysql | tr '\n' ' '; echo" }
docker exec -u root <ID> sh -c "du -sh /var/lib/mysql/peq; ls -l /var/lib/mysql/peq/character_data.ibd; ls /var/lib/mysql/peq | grep -c custom_achievement"
docker exec -u root <ID> sh -c "service mariadb start; sleep 8; mysqldump --single-transaction --routines --events --databases peq | gzip > /tmp/peq_recovered.sql.gz"
docker cp <ID>:/tmp/peq_recovered.sql.gz C:\AoTv3\AoTv4\peq_recovered.sql.gz
```
- ⚠️ **`docker exec` MUST use `-u root`.** Dev-container images default to user `vscode`, and the per-database
  dirs are `drwx------ mysql` → `du /var/lib/mysql/peq` returns **Permission denied**. A check written as
  `du … || echo "no peq db"` therefore reports **"no DB" on a container that has the full database** — this
  cost an hour and nearly triggered a needless snapshot import.
- **Pick the right container by evidence, not by name:** `du` size (live DB ≈ 750 MB), `character_data.ibd`
  mtime (the newest = last session; InnoDB flushes lazily so the mtime lags the true last write), and a
  `custom_achievement*` file count > 0 (proves the Jul-23 achievement work is in it).
- MariaDB will not be running in an old container (PID 1 is the sleep loop) — that is normal, `service mariadb
  start` first. Its startup runs **InnoDB crash recovery**, replaying `ib_logfile0`, which is exactly why you
  dump from a started server instead of copying the datadir files.
- Verify a dump before trusting it: `gzip -dc x.sql.gz | grep -c "^CREATE TABLE"` (expect **233** now — 223
  pre-achievements) and a `Dump completed on …` marker at the tail. **36 MB gzipped ≈ 341 MB SQL is normal.**
- **Canonical recovery point is now `/src/peq_recovered.sql.gz`** (Jul 24 23:18, full `peq`, achievements
  included), NOT the stale `/src/aotv4_current.sql` (Jul 21) that `POST_REBUILD_RECOVERY.md` used to point at.
