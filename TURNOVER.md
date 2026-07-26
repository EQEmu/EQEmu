# Turnover — session of 2026-07-25/26

Read this before touching anything. Sections are ordered by what will bite you soonest.

---

## 1. State at handoff

**Server stack is UP and running everything below.** `world`, one `eqlaunch`, six zones, ucs,
queryserv, loginserver. Zone binary rebuilt and zones cycled onto it 2026-07-26 08:06 (verified: no
`(deleted)` in `/proc/<pid>/exe`).

**Client files** are staged in `aotv4_client_install/`. Installed during the session: rebuilt
`dinput8.dll`, `EQUI_AoTSpellChoiceWnd.xml`, `spells_us.txt`, `dbstr_us.txt`, `SkillCaps.txt`,
`BaseData.txt`. **Not yet installed:** `EQUI_AAWindow.xml` (renamed tabs) and the re-exported
`dbstr_us.txt`. See `aotv4_client_install/README_INSTALL.md`.

---

## 2. Works and confirmed in game

| Thing | Notes |
|---|---|
| **Level-up reward picker** | Native SIDL window, three cards, per-row spell icons. Icons confirmed correct after the final dll rebuild. |
| **Tank AA tree (5 AAs)** | Visible, named, purchasable. Hosted on native AA rows — see §4. |
| **Reward pool** | Back on the stock spell set: 2,585 stock + custom lines, 310 blacklisted, ~2,300 offerable. |
| **AA restored to base EQ** | `aotv4_aa_revert.sql` applied; verified 0 rows differ from pristine PEQ across 1,568 abilities / 6,652 ranks. |

## 3. Built but NEVER TESTED

Nothing below has been exercised in game. All of it compiles, deploys and reads correctly.

- **Shield Wall** (`/shield` damage splitting) — any class, permanent, up to 4 sharers, 20% surcharge
  per extra person, "Shielded" buffs naming their casters. Test plan: **`SHIELD_WALL.md`**.
- **World boss** (`#worldboss`) — armed for a random classic dungeon, spawns on first entry, loot
  rights to everyone who damaged it. Test plan + known failure points: **`WORLD_BOSS.md`**.
  ⚠️ Two identified risks around loot rights (hate list may be empty at death; corpse may not exist
  yet) — that doc explains how to tell them apart.
- **Seven custom spell lines** (43300-43382) — reptile, sloth, moonfire, promised, kindred, mark,
  thirst. In the pool, never cast.
- **Tank AA effects** — the AAs *display*, but no one has bought one and verified the effect fires.

---

## 4. The AA system — read before adding any AA

### ⚠️⚠️ Custom AAs MUST reuse existing native rows

**Creating an AA with a new id does not work.** The server resolves it, passes every check in
`CanUseAlternateAdvancementRank`, and sends the packet — proven with logging — and the client
silently discards it. Ability ids above the native max (**30195**) and rank ids above **65535** both
fail this way, with no error anywhere. This cost most of a session.

The working approach (`custom/sql/aotv4_aa_tank_hosted.sql`): **take over a native AA**. Keep its
ability id, rank ids, `title_sid` and rank chain; replace only `aa_rank_effects` and the `db_str`
strings.

### ⚠️ Walk the rank chain — never assume contiguous ids

`first_rank_id + 0..4` is NOT the rank list. Natural Durability's chain is
**107 → 108 → 109 → 7541 → 7542 → 7543**. Assuming a contiguous block wrote costs and levels onto
ranks 110/111, which belong to a different ability. Always follow `next_id` from `first_rank_id`
first. Terminate a chain early with `next_id = -1` to control the rank count.

### Current tank tree

| AA | Host ability | Real rank chain | Kind |
|---|---|---|---|
| Shield Oath | 28 | 107,108,109,**7541,7542** | native SPA 192 hate + 185 damage |
| Stonestride | 2 | 7-11 | native SPA 162 flat damage reduction |
| Unyielding | 3 | 12-16 | native SPA 172 evasion |
| Bloodied Bash | 4 | 17-21 | **marker** → `lua_modules/aotv4_aa_tank.lua` |
| Aegis Reflex | 6 | 27-31 | **marker** → `Mob::MeleeMitigation` + `Mob::HealDamage` |

### Healer tree (`type = 2`) — built 2026-07-26, **untested**

| AA | Host ability | Rank chain | Kind |
|---|---|---|---|
| Overflowing Grace | 8 | 37-41 | **marker** → `HealDamage` + `RuneAbsorb` |
| Triage Instinct | 9 | 42-46 | **marker** → `GetActSpellHealing` |
| Mender's Echo | 10 | 47-51 | **marker** → `HealDamage` |
| Cleansing Renewal | 11 | 52-56 | **marker** → the four cure sites |
| Borrowed Breath | 12 | 57-61 | **marker** → `HealDamage` |

All behaviour in `zone/aotv4_healer_aa.cpp`. Full design notes, the anti-farm gates and a
step-by-step test plan are in **`HEALER_AA.md`**. Two ranks were changed from the brief: Mender's
Echo r5 (the original wording asked to remove the recursion guard) and Borrowed Breath r1-4
(percentages are invisible on this server — made flat).

All `classes = 65535` (roles, not class locks). Ranks at levels 5/15/25/35/45, costs 3/4/5/6/8.

**Marker AAs** carry no effect rows and are read at runtime via `GetAA(<first rank id>)`, which
returns the rank you own. The rank id is the join between SQL and code — **a wrong id silently reads
0 forever**, so the AA looks bought and does nothing.

### Native AAs consumed

**Ten** are in use: Innate Stamina (2), Agility (3), Dexterity (4), Wisdom (6) and Natural
Durability (28) for the Tank tree; Innate Fire (8), Cold (9), Magic (10), Poison (11) and Disease
Protection (12) for the Healer tree. An eleventh, **Innate Intelligence (5)**, was consumed by
Bulwark and has been **restored** — name, ranks 22-26, effects and strings all back to pristine,
left disabled like the rest of the native set.

To restore another host: pristine `aa_ability`/`aa_ranks`/`aa_rank_effects` rows are in
`utils/sql/peq_aa_tables_post_rework.sql`, and pristine **`db_str`** (which that file does NOT
contain) is in `peq_recovered.sql.gz`, the 2026-07-24 dump that predates this work:

```bash
gzip -dc peq_recovered.sql.gz | awk '/INSERT INTO `db_str`/,0' \
  | grep -oE "\(<title_sid>,(1|4),'[^']*'\)"
```

### Known defects — ALL CLEARED 2026-07-26

1. ~~`aotv4_aa_tank_hosted.sql` is stale~~ — **rewritten** against the live DB: real rank chains
   (Shield Oath on 107,108,109,7541,7542), no Bulwark, chains terminated with `next_id = -1`,
   `type = 1` set explicitly. Verified idempotent by applying it twice.
2. ~~`aotv4_aa_tank.sql` is superseded~~ — **deleted**, along with every reference to it and to the
   dead 4000x/5002x ids in `attack.cpp`, `aa.cpp` and `aotv4_aa_tank.lua`.
3. ~~Ranks 110-112 restored by inference~~ — they were **still wrong** (guessed level 59/61 cost 5;
   pristine is level 55, costs 2/4). They belong to ability 29 Natural Healing, and are now restored
   from source and diffed field-for-field against the pristine dump.
4. ~~Bulwark code inert but present~~ — **removed** from `Mob::ApplyShieldWall`. It read
   `GetAA(50020)`, a rank id from the discarded new-id attempt, so it had always returned 0. Worth
   noting it was documented here as reading `GetAA(22)`, which would have become live again the
   moment host 5 was restored.

---

## 5. Four AA tabs — DONE client-side, needs one in-game look

The tab an AA lands on is **`aa_ability.type`**, not `category`. `category` is a sub-grouping header
*within* a tab. (An earlier draft of this doc said category; that was wrong and the experiment it
proposed is unnecessary.)

| `type` | Stock label | Renamed to | Enabled AAs |
|---|---|---|---|
| 1 | General | **Tank** | the 5 tank AAs |
| 2 | Archetype | **Healer** | none yet |
| 3 | Class | **Ranged** | none yet |
| 4 | Special | **Melee** | none yet |

Renamed in `aotv4_client_install/uifiles_default/EQUI_AAWindow.xml` — **client XML only, no source
and no SQL**. The tank tree is already `type = 1`, so it needed no change; healer/ranged/melee get
`type` 2/3/4 as they are built.

Evidence the labels are XML-driven rather than hardcoded: `strings eqgame.exe` contains **no**
"Archetype"/"Special"/`AAW_*Page` — only three `AAW_` control names (Save/Load buttons), so the rest
of the window resolves positionally. That also means the page and listbox `item=` names must not be
touched, only `<TabText>`.

**Remaining:** copy the file to `<EQ>/uifiles/default/` and confirm the tabs read Tank/Healer/Ranged/
Melee with the five AAs under Tank. Untested in game.

---

## 6. Hard-won constraints (all cost real time this session)

**Client / UI**
- **Texture names are CASE SENSITIVE** and resolved against `EQUI_Animations.xml`'s `TextureInfo`
  entries, not the filesystem. Spell sheets are `Spells01.tga` (capital S). Lowercase silently draws
  nothing — no error, no log.
- **`--` inside an XML comment crashes the client** before character select. Always run
  `aotv4_client_install/validate_ui_xml.sh` before copying any `EQUI_*.xml`. `UIErrorLog.txt` names
  the file and line.
- The validator used to **report every stock file as unbalanced** — it counted self-closing tags
  (`<TooltipReference />`, `<Text />`) as opens with no close. Fixed 2026-07-26. If it ever starts
  crying wolf again, check that first: a checker that always fails is worse than no checker, because
  the real failure then looks like the usual noise.
- **A missing `<Include>` in `EQUI.xml` is completely silent** — `CCustomWnd` returns, no window, no
  error. First thing to check when a window "does nothing".
- **Only `Spells01`-`Spells07` are declared**, so icon indices above 251 have no texture.
- **AA names come from `db_str`** (type 1 = title, type 4 = description) resolved by the CLIENT from
  its own `dbstr_us.txt`. `title_sid = -1` renders **no row at all** — it does not fall back to
  `aa_ability.name`. Any `db_str` change needs `./export_client_files` + reinstalling `dbstr_us.txt`.

**Spells**
- **`teleport_zone` is not "destination zone"** — it is "names an NPC or a zone". Every
  pet/familiar/warder stores its summon type there. Filtering travel on it pruned 95 stock pet
  spells. Detect travel by SPA (25/26/83/88/104) and `targettype 3`.
- **`spell_blacklist.lua` must be regenerated with the pool** (`gen_stock_pool.pl` does both). It was
  emptied when the custom set took over; re-pointing at stock without rebuilding it reopened every
  port, rez and Enchant-material spell.
- **`Client::Message` is printf-style** — a literal `%` in a description is eaten as a format token.
  The generator spells out "percent".
- **SPA 121 ReverseDS base must be NEGATIVE** (`if (rev_ds < 0)`); positive does nothing.
- **SPA 178 MeleeLifetap is percentage-only** — a flat per-hit amount cannot be expressed in
  `spells_new` at all, hence the Lua in the Thirst line.
- **Percentage mitigation is imperceptible** against this server's small post-mitigation numbers.
  Use flat per-hit reduction (SPA 162 base 100 / limit N).
- `spells_new` **is** in shared memory (world down → `./shared_memory` → restart); `npc_types` and
  AAs are **not** (zone restart only).

**Lua**
- `#reloadquest` does **not** reload `require`d modules — zone restart.
- Rank/spell ids in Lua tables are unchecked joins to SQL; wrong ones fail silently.

---

## 7. Backlog

1. Verify the AA tab mapping, rename tabs, build **healer / ranged / melee** trees
2. Test **Shield Wall** and the **world boss** (both have written test plans)
3. Convert the **"You Lost"** death window from GDI overlay to native SIDL (also Portal, Search)
4. Replace the placeholder **"Shielded" buffs** with a real short-buff / autoskill tracker —
   marker buffs are used for display in five systems now and it does not scale
5. **`era_system.check_unlock` is orphaned** — random AA was its only caller, so nothing advances the
   expansion race. Needs a new trigger (achievements are the obvious candidate).
6. Curate world boss loot; tune its 120k HP after a real kill
7. `/src` has many uncommitted files; the dll and quests repos are separate (see CLAUDE.md §0)

---

## 8. Where things are documented

| Doc | Covers |
|---|---|
| `CLAUDE.md` | The whole server. §3 picker + You Lost TODO, §5 spell pool, §6 AA, §17b Shield Wall, §17c world boss |
| `HEALER_AA.md` | Healer AA tree — design decisions, anti-farm gates, test plan |
| `SHIELD_WALL.md` | Player-facing `/shield` guide + server rules |
| `WORLD_BOSS.md` | World boss test plan and failure modes |
| `aotv4_client_install/README_INSTALL.md` | What to copy to the client, and how each failure looks |
| `aotv4_custom_spells.csv` | Every custom spell, exported |
