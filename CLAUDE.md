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
  `SKILL_OFFER_CHANCE` (**0.125** -- roughly one level in eight; it sat at **0.0** for a while, meaning
  combat abilities were never offered at all, while this line still claimed 0.7), `SKILL_GRANT_PCT` (0.25).
- `spell_pool.lua` (**generated**, see §5), `spell_icons.lua` (**generated**),
  `spell_blacklist.lua` (**generated**), `skill_pool.lua` (hand-curated; see §4).

### Reroll — pay coin for three different rewards — 2026-07-28
Do not like the three offered? Press **Reroll** under the description pane on the Choose tab and pay.
Price box sits beside the button. Pure Lua + the picker XML; **no item and no vendor NPC**.
`spell_choice.M.reroll` / `.reroll_cost` / `.send_reroll_cost`. Protocol: `/say spellreroll` out,
**`SPELLREROLLCOST <copper>`** back (both swallowed by the dll).
- **Price = `5p × (rerolls_bought + 1)`** — 5p, 10p, 15p, … Counted per character in bucket
  `rerollbuy_<charid>` and ⚠️ **never reset by the roguelite death**: rerolls get dearer over a
  character's whole life, not per run.
- ⚠️⚠️ **A TOKEN ITEM + VENDOR WAS BUILT FIRST AND THEN DELETED** (item 147495 "Destiny Fragment",
  npc 2000210). Charging coin straight from the button is simpler and needs no shared-memory rebuild,
  no spawn rows and no hand-in script. If a token is ever wanted again, the reason the vendor could
  not be a `merchantlist` merchant still stands: **merchantlist prices are STATIC** — no per-character
  column, no hook — so an escalating price cannot be expressed there at all.
- ⚠️⚠️ **Gate ORDER in `M.reroll` is load-bearing**: pending offer → funds → **build** → *then* charge.
  `TakeMoneyFromPP` has no refund path, so charging before building would bill for a reroll that turns
  out to have nothing left to offer.
- ⚠️ **It REPLACES the queue's front offer, never appends** — the front is what the window shows, so
  appending would leave the disliked set still pickable and the player would have paid for nothing.
- ⚠️ `GetCarriedMoney` is **carried** coin only, not bank. The refusal message says so, because
  "you cannot afford it" while standing on 500p in the bank reads as a bug.
- ⚠️ `build_offer()` is shared by `M.offer` and `M.reroll` deliberately: two copies would drift on the
  skill chance, level band and shuffle. Reroll bumps the same `spell_seed_` counter, which is what
  makes it produce a *different* set rather than re-deriving the one you paid to remove.
- ⚠️ The dll validates **nothing** — `g_rerollCost` is a DISPLAY value. Pricing, the affordability
  check and taking the coin are all server-side, so a modified client cannot reroll for free.
- ⚠️ The cost box is an **STMLbox, not a Label** — Label has no confirmed border on this build, and the
  price must arrive from the server (it is per character) rather than being hardcoded client-side.
- ⚠️ In `gen_choice_xml.pl` the detail box shrank 174 → **140** to make room; it could not move *up*
  because the three cards occupy 34..208 and `DETNAME_Y` sits directly under them.

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
  the dll just `Show()`s the matching one, and `CXWnd::Show` **is** mapped.
  Generated by **`aotv4_client_install/gen_choice_xml.pl`**, which **reads the icon set out of
  `spell_icons.lua` + `skill_pool.lua`** rather than hardcoding it → `EQUI_AoTSpellChoiceWnd.xml`
  (~8600 lines, 154 icons, 471 pieces). **Rerun it whenever the pool is regenerated.**
  The dll keeps **no** copy of the icon list: it looks the button up by name at runtime
  (`ASC_Icon<row>_<icon>`), so the XML is the single source of truth and an undefined icon just
  draws nothing. (An earlier `kIcons[]` array had to match the generator exactly and drifted the
  moment the pool changed from 22 icons to 154.) Same trick applies to any window wanting icons.
- ⚠️ **Only `Spells01..Spells07` are declared** in the client's `EQUI_Animations.xml`, so icon
  indices **above 251** have no texture and are skipped by the generator (1 spell in the stock pool).
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
- ✅ **The "You Lost" death window is now a native SIDL window** (2026-07-27) —
  `core_lostwindow.cpp/.h` + `EQUI_AoTLostWnd.xml`, its OWN translation unit like `core_advloot`.
  Installs no detours; our `dsp_chat` calls `LostParseTransport` (swallows `LOSTDATA`), Ctrl+Q calls
  `LostWindowShow`, and the `CleanGameUI`/`ReloadUI` detour that `core_achievements_native.cpp` owns
  calls `LostWindowOnUiReset`. **Wire format unchanged** (`LOSTDATA name^name^…`) so the server was
  not touched. Scrolling, dragging and the 45-second idle timeout are all gone — the UI engine does
  the first two and a real window with a close box does not need the third, so the list now stays
  until dismissed. The GDI `PaintLostOverlay`/`LostOverlayWndProc`/`LostOverlayThreadProc` remain in
  `core_spellwindow.cpp` but are **unreachable** (`if (false && …)`); delete once confirmed working.
  Install: `aotv4_client_install/LOST_WINDOW_INSTALL.md`.
  - ✅ **It is the "Death Book" and the dates EXPAND** (2026-07-28). One header row per death —
    `[+]`/`[-]`, date, killer, item count — and only an open one lists its items, so 400 entries read
    as a table of contents rather than a wall of names. The newest death starts open.
    ⚠️ **`LostBuildGroups` resets the open/closed state**, so it is called only where the DATA is
    replaced (`LOSTDATA`, and the LAST `LOSTLOG` chunk), never from `Refresh()` — and never on a
    partial log, which would split one death across two chunks into two dates.
    ⚠️⚠️ **The toggle is DEFERRED to `LostWindowTick`, not done in the click handler.** Toggling calls
    `Refresh` → `DeleteAll`, which would destroy the listbox's rows from inside the listbox's own
    notification while the client is still walking them. ⚠️ The selection is also read *after*
    `CSidlScreenWnd::WndNotification` has run — inside it, `GetCurSel` is still one click behind.
    ⚠️ Row index is NOT a data index (headers interleave, a closed death hides many rows) — every
    click goes through the explicit `g_rowGroup[]` map.
- ✅ **The Search window is now a native SIDL window and is called "Allaclone"** (2026-07-28) —
  `core_allaclone.cpp/.h` + `EQUI_AoTAllacloneWnd.xml`, its OWN translation unit like `core_advloot`.
  Installs no detours; our `dsp_chat` calls `AllacloneParseTransport` (swallows `SRCHDATA`/`SRCHDET`)
  and `AllacloneIsOurEcho`, `ProcessGameEvents` calls `AllacloneTick`, and the `CleanGameUI`/
  `ReloadUI` detour that `core_achievements_native.cpp` owns calls `AllacloneOnUiReset`.
  **Wire format unchanged** (`/say srch <kind> <term>`, `/say srchdet <kind> <id>`) so the server was
  not touched at all — `Client::SearchList`/`SearchDetail` are as they were.
  Command is **`/allaclone`**, with `/search` and `/find` kept as aliases; the AoT menu button is
  relabelled. `EnableSearchWindow()`/`ShowSearchWindow()` in `core_spellwindow.cpp` are now
  **forwarders** so the switch is one place instead of a rename through every caller, and
  `g_searchEnabled` stays false, which makes the old overlay's thread/paint/chat handler unreachable
  without deleting them yet.
  - ⚠️ **NO `TabBox` here, deliberately.** Native tabs are the right widget when each page owns its
    contents; the four kinds (item/npc/spell/recipe) share ONE search box, ONE list and ONE detail
    pane, so tabs would mean four copies of each. They are **mode buttons**, and the active one is
    shown by rewriting its LABEL (`> Items <`) because no latched button state can be set reliably
    from code on this build.
  - ⚠️ **Enter is POLLED, not hooked.** An `Editbox` gives no usable "text committed" notification
    here, so `AllacloneTick` watches the box and searches once the text settles (450 ms) — the same
    approach the AdvLoot filter box already uses. ⚠️ An empty box must still be RECORDED as the last
    searched term or the poll re-fires every frame forever.
  - ⚠️ `SRCHDET` lines are escaped into STML (`<` `>` `&`) before display — an item name containing
    `<` would otherwise swallow the rest of the panel.
  - Install: `aotv4_client_install/ALLACLONE_WINDOW_INSTALL.md`.
- 📌 **TODO — the Portal window is still a GDI overlay** and should get the same treatment for the
  same reasons: self-drawn chrome never matches EQ, it can't scale with the UI, and it only works
  windowed. Client-side-only change; its wire format can stay.
- ⚠️ **A `<Button>` uses `<Template>BDT_Normal</Template>`, NOT `<ButtonDrawTemplate>`.** The tag
  name differs from the inline form used for icon buttons, and only the ~34 `BDT_*` names actually
  defined in `EQUI_Templates.xml` resolve — anything else draws an empty hole with no error, the
  same failure mode as a mis-cased texture name.
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

> **2026-07-26: the pool is back to the STOCK spell set.** The 113-spell custom set (43000-43149)
> briefly replaced it and proved mostly redundant with native spells. Regenerate with
> **`perl .devcontainer/custom/spells/gen_stock_pool.pl`**, which writes all three generated modules
> (`spell_pool.lua` 2605 spells over 78 levels, `spell_icons.lua`, `spell_desc.lua`) from
> `id < 10000` + the **20 unique customs** that are kept — the Lua-driven abilities (every effect
> slot 254; behaviour lives in `quests/global/spells/43xxx.lua`, `aotv4_summon_table`,
> `aotv4_reactions`). The other 93 customs stay in the DB but are **never offered**; full restore of
> all 202 rows is `custom/sql/aotv4_custom_spells_backup.sql`, and the superseded generated modules
> are in `custom/spells/old_custom_pool/`. **Rerun `aotv4_client_install/gen_choice_xml.pl` after
> any pool regen** so the picker's icon set matches (§3). Zone restart to reload the Lua modules.
> ⚠️ `%` in a description is spelled out by the generator: `Client::Message` is printf-style, so a
> stray `%` is eaten as a format token.
>
> **Adding new custom spells to the pool: use band `43300-43349`** (helpers/triggers at **43350+**,
> which are never offered). The generator pulls that band wholesale, so unlike the 43000-43149
> customs they may use effect slots. Two lines exist so far, both filling the empty low half of a
> native SPA 323 "add defensive proc" line and both cloned **via a temp table** so all ~236 columns
> stay byte-identical to stock (never hand-list the columns):
>   - `custom/sql/aotv4_reptile_line.sql` — Druid *Skin of the Reptile* (stock 8008/8009, level 68),
>     heal-on-being-hit. Six tiers **8/18/28/38/48/58** = ids **43300-43305** / triggers 43350-43355.
>   - `custom/sql/aotv4_sloth_line.sql` — Shaman *Lingering Sloth* (stock 8015/8016, level 68, the
>     line's **only** stock member), retaliation-slow. Ids **43306-43311** / triggers 43356-43361.
>   - `custom/sql/aotv4_moonfire_line.sql` — a **lifetap weighted 25% damage / 75% heal**, ids
>     **43312-43317**. Damage ≈ half the native lifetap for the level, healing 3× the damage.
>     ⚠️⚠️ **The 3× split cannot be expressed in the DB.** EQEmu hardcodes the tap ratio —
>     `int64 healed = damage;` (`zone/attack.cpp:4287`) — and `spells_new` has no ratio column. So
>     the rows are **real taps** (`targettype 13` ST_Tap, what `IsLifetapSpell` keys on,
>     `common/spdat.cpp:108`) and the engine pays the first 1×, while
>     `quests/global/spells/433xx.lua` + `lua_modules/aotv4_moonfire.lua` add the remaining 2×.
>     **Change a damage value in the SQL and you MUST change that tier script's bonus (= 2× damage).**
>     Staying a genuine tap matters beyond the heal: `IsLifetapSpell` excludes it from
>     `IsDamageSpell`/`IsAnyDamageSpell`/`IsDamageOverTimeSpell` and drives the tap emote.
>     ⚠️ No tier is named "Moonfire": stock **2877 Moonfire** (Druid 60 nuke) is already in the pool
>     and would be a real duplicate.
>     ⚠️⚠️ **This line's Lua bonus never actually ran until 2026-07-29** — `EVENT_SPELL_EFFECT_NPC`
>     had no argument builder, so `e.caster_id` was nil against any monster and `tap_bonus` returned
>     early every time. Moonfire was a plain 1:1 engine-paid lifetap for its whole life. See §10.
>   - `custom/sql/aotv4_promised_line.sql` — **delayed "bloom" heals**, ids **43324-43329** / blooms
>     43368-43373. Cast it, wait 3 ticks, it heals when the buff expires naturally. Uses the native
>     **SPA 289 `CastOnFadeEffect`** (`common/spdat.h:1352`) — no Lua needed, unlike 43017 Languid
>     Healing which predates knowing about it. Cloned from stock 9755/9758 (*Promised Renewal*).
>     ⚠️ The native Promised family is effectively **unreachable**: it starts at level **73** and
>     every tier above 9755 is **id ≥ 10000**, which the pool's `id < 10000` filter excludes.
>     ⚠️ Keep the **SPA 44 + SPA 148 pair**: slot 2 (SPA 44) carries the heal amount as a *stacking
>     marker* and slot 3 (`StackingCommand_Block`, base 44) reads it, which is how Promised heals
>     refuse to overwrite a stronger one. Slot 2's base must equal the real heal or the tiers stack.
>     All six share `spellgroup` 43324 with ascending `rank` so a higher tier replaces a lower.
>   - `custom/sql/aotv4_kindred_line.sql` — **melee proc that heals YOUR GROUP**, ids **43330-43335**
>     / procs 43374-43379. Self buff with **SPA 85 `WeaponProc`**; on a swing the proc heals the
>     whole group. Modelled field-for-field on native **`1976 Blessing of the Blackstar`** (proc of
>     31348 Blackstar, Mace of Night): SPA 0 positive base, `targettype 41`, `goodEffect 1`,
>     range 100, no duration, `spell_category -99`. `39661 Soothing Wave` (+550) is the same shape.
>     ⚠️ **Hopebringer was NOT a guide** — its proc `3606 Light of Marr` shipped as a **stub**
>     (`targettype 5`, `goodEffect 0`, SPA 0 base **−1** = one point of damage; recourse 4119 empty).
>     **Fixed by `custom/sql/aotv4_light_of_marr_fix.sql`** → a 300-point group heal on the 1976
>     pattern, calibrated between Blackstar (100, classic) and Sword of the Sanative (550, lvl 100).
>     That spell is shared by **6 items** — Hopebringer, Bloodstone Blade of the Zun'Muram, and the
>     Hallowed/Mythic tier of each — so one edit repairs all six. Item proc, never pool-offered, so
>     no pool regen needed; still needs a shared-memory rebuild.
>     The engine routes it: `Mob::ExecWeaponProc`
>     (`zone/mob.cpp:5358`) casts a **beneficial** proc on `this` (the swinger) and a detrimental one
>     on the victim, then `SpellFinished` resolves `case ST_Group` (`zone/spells.cpp:2162`).
>     ⚠️ Two fields on the PROC spell are load-bearing: **`goodEffect=1`** (picks the swinger branch
>     — set 0 and it heals the mob you just hit) and **`targettype=41`** (ST_Group — without it only
>     the swinger is healed). Same trick works for a **defensive** proc (SPA 323) since
>     `TryDefensiveProc` calls the same function. Nearest native precedent: 6191 Aura of Runes, whose
>     proc is beneficial but self-targeted.
>   - `custom/sql/aotv4_mark_line.sql` — **reverse damage shield**, ids **43336-43341**, no helper
>     rows (SPA 121 is self-contained). Cloned from stock `2507 Mark of Retribution` (Cleric **54**,
>     the line's floor; then 65/69/74). ⚠️ **It is a DEBUFF cast on the ENEMY** (`goodEffect 0`) —
>     the "Mark of" naming reads like a blessing but the marked creature wounds *itself* on every
>     melee hit it lands. ⚠️ **SPA 121's base must be NEGATIVE**: `Mob::DamageShield` reads it off
>     the *attacker's* bonuses and the guard is `if (rev_ds < 0)` (`zone/attack.cpp:3476`, `:3591`),
>     so a positive base silently does nothing.
>   - `custom/sql/aotv4_thirst_line.sql` — **flat heal per melee hit**, ids **43342-43347**.
>     ⚠️ **It is a SELF BUFF; a "debuff that heals whoever hits it" is NOT buildable.** The DS family
>     covers damage both ways (59 = buff on defender hurts attacker, 121 = debuff on attacker hurts
>     attacker) but has **no healing counterpart in either direction**, and `Mob::MeleeLifeTap`
>     (`zone/mob.cpp:6852`) reads the bonus off the *attacker* and heals the attacker.
>     ⚠️⚠️ **The rows are INERT MARKER BUFFS (all slots 254) — the heal is paid by Lua.** SPA 178
>     `MeleeLifetap` is percentage-only by construction (`damage * mod/100`, `mob.cpp:6860`) and
>     nothing else fires on every successful melee hit, so a FLAT per-hit amount cannot be expressed
>     in `spells_new` at all. `lua_modules/aotv4_thirst.lua` holds the amounts and pays out from
>     `global_player.event_damage_given`; **the SQL's numbers are documentation only**. Same
>     inert-marker pattern as 43022 Divine Aura / 43035 Blade Turn / 43056 Counterattack.
>     ⚠️ That hook runs on **every damage event for every player** — keep the arithmetic early-outs
>     first, before any `FindBuff`. Reactions run first and own the return value (a negative override
>     negates the hit, which must not pay a leech).
>     **CHARGES are native**: `numhits` + `numhitstype = 5` ("Outgoing Hit Successes") makes the engine
>     decrement per successful melee hit and fade the buff at zero (`CheckNumHitsRemaining`,
>     `spell_effects.cpp:7075`, fired from `CommonOutgoingHitSuccess`, `attack.cpp:6690`), and the
>     client shows the counter for free. It decrements **any** buff with `hit_number > 0` of that type
>     — no real effect needed. Native precedent: the **Blade Attunement** line (4644/4646/4648) is
>     likewise an inert buff (slots = filler 10) with `numhits 100, type 5`; native charges run 5-800.
>     ⚠️ Charge and heal are counted in **different places**, so a hit absorbed to 0 damage spends a
>     charge without paying a heal. The `numhitstype` legend is in `spell_effects.cpp:7077`.
>   - `custom/sql/aotv4_sinew_line.sql` — a **damage tap that returns ENDURANCE**, weighted **75
>     percent damage / 25 percent endurance**, ids **43318-43323** (Sinewbite → Sinewreave, levels
>     8/18/28/38/48/58, damage 27/90/120/225/360/525, return = **damage / 3**). The structural
>     mirror of the Moonfire line, which splits the same way but returns health. **It exists because
>     of §22**: specials now cost endurance in proportion to their damage, and this refills the bar.
>     Damage sits just under the average native single-target nuke for the level (averages read out
>     of the DB, not from memory) because the cast also returns a resource; every damage value is
>     divisible by 3 so the return is a clean integer.
>     ⚠️⚠️ **NOT a lifetap, and must not be "fixed" into one.** `targettype` stays **5**. Setting it
>     to 13 is what `IsLifetapSpell` keys on (`common/spdat.cpp:108`) and makes `Mob::Damage` heal
>     the caster 1× the damage in **health** (`int64 healed = damage;`, `zone/attack.cpp:4287`) —
>     free HP nobody asked for, on top of the endurance. Moonfire *wants* that engine-paid 1× and is
>     a genuine tap; this line does not, so Lua pays **the whole** return.
>     ⚠️⚠️ **The return is not expressible in the DB at all** — there is no endurance-tap SPA. SPA
>     189 `SE_CurrentEndurance` applies to the spell's TARGET, and the target of a nuke is the thing
>     you are hitting, so it would hand the **monster** the endurance; a recourse could aim it at the
>     caster but only for a flat amount unrelated to the tier. Hence plain nukes +
>     `quests/global/spells/433xx.lua` → `lua_modules/aotv4_sinewtap.lua`. **Change a damage value in
>     the SQL and you MUST change that tier script's return (= damage / 3).**
>     ⚠️ **Clients only** — that is the shape of the mechanic, not a rare-case guard. `Mob::GetEndurance`
>     is a virtual returning 0 and `Mob::SetEndurance` a no-op (`zone/mob.h:674`, `:677`); only
>     `Client` overrides them (`zone/client.h:751`, `:756`). Resolve the caster with
>     **`GetClientByID`, not `GetMobID`** — it resolves invalid for a pet or NPC caster, which is
>     exactly the case to skip. ⚠️ There is **no `AddEndurance` binding** (only `Get`/`Set`,
>     `zone/lua_client.cpp:1304-1321`), so the max-endurance clamp is applied by hand or a cast at a
>     full bar sets endurance above max.
>     ⚠️ Cloned from stock **466 Lightning Shock**; only `new_icon` is overridden, to **47** — the
>     icon 44 native lifetaps use, so it reads as draining rather than lightning. Named "Sinew"
>     because it collides with nothing; deliberately **not** "Marrow" (stock 8608 Marrow Drain / 8803
>     Marrow Bend would sit beside it in the picker).
>     📌 Damage/mana is untuned until it is played (dpm 1.13 → 1.59 across the tiers).
>   - ⚠️⚠️ **A re-runnable script's cleanup `DELETE` must only name the ids that script itself
>     creates.** `aotv4_moonfire_line.sql` opened with `DELETE … WHERE id BETWEEN 43312 AND 43323`
>     to sweep up leftovers from an earlier pass — which silently covers **43318-43323, the whole
>     Sinew line**. Re-running moonfire would have deleted six spells belonging to another script,
>     with no error. Narrowed to `43312 AND 43317` (2026-07-29).
>
> ⚠️⚠️ **CLONING A STOCK SPELL INHERITS ITS DAMAGE FORMULA — overriding `effect_base_value1` is NOT
> enough** (bit the Sinew line, 2026-07-29). `spells_new` carries `formulaN` and `maxN` beside every
> base value. **Formula 1-99 means `base + caster_level * formula`** (`zone/spell_effects.cpp`, the
> default branch of the formula switch); **100 means "= base"**, the static case. 466 Lightning Shock
> ships `formula1 = 5, max1 = 570`, so the cloned tier dealt **27 + 8×5 = 67** at level 8 instead of
> 27, heading for 277 by level 50, with the top tier clipped at 570.
> - ⚠️ For a hand-tuned tier ladder this breaks the **design**, not just the numbers: any Lua-paid
>   companion value (the Sinew line's endurance return, a Moonfire-style bonus heal) is a **flat**
>   amount, so damage that climbs with level while the return stands still destroys the ratio the
>   line exists to express.
> - Fix is `formula1 = 100, max1 = 0`. An audit of all of **43300-43347** found the Sinew line to be
>   the **only** offender — every other custom line is already `formula 100 / max 0`, because they
>   cloned either a static stock buff or (Moonfire) a custom that was already static. That is also
>   why this had never shown up before.
> - 📌 **Check `formulaN`/`maxN` on any future clone before trusting its numbers**, the same way the
>   proc-slot note below says to check the template.
>
> ⚠️ **Observed damage will still exceed the base — that is DC Overpower, not a bug.**
> `zone/mob.cpp:8860` adds bonus spell damage scaled off how far the caster's DC exceeds the
> target's resist (`AoT:DCOverpower*` rules), which against a weak mob was contributing ~24 at
> level 8. It also means a **tap's engine-paid 1× follows the INFLATED damage while the Lua bonus
> stays flat**, so Moonspark healed ~100 rather than its nominal 75. Expected, and the same class of
> approximation already recorded for partial resists.
>
> ⚠️ The proc SPA sits in **slot 4** on the reptile line but **slot 3** on the sloth line — check the
> template before overriding. ⚠️ SPA 11 base is the attacker's resulting melee **speed**, not the slow
> amount: 75 = "attacks at 75 percent" = a 25 percent slow, so **lower base is stronger**.
> `spells_new` **is in shared memory**: world down → `./shared_memory` → restart, then rerun both
> generators.
>
> ⚠️⚠️ **THE BARD PIVOT SILENTLY BROKE ALL-CLASS SPELL ACCESS (fixed 2026-07-27).** While every
character was forced to Bard, "any class can use anything" was free -- the pool only had to be legal
for Bard. Opening the server to 16 classes removed that and the STOCK set was never reopened: of
2,707 pool spells, **ZERO** were castable by all classes, and **bard songs carry a real level only
in `classes8`** so 15 of 16 classes could be offered a song and then simply never use it. Three gates
all had to be reopened, and any one left shut makes the reward useless:
1. `spells_new.classes1..16` -- can the class scribe/cast it
2. `skill_caps` **Singing 12, Percussion 41, Stringed 49, Wind 54, Brass 70** -- only Bard had ANY
3. `skill_caps` for the twelve activated combat specials -- without a cap `CanHaveSkill` refuses and
   a picked reward silently does nothing
All three are in `custom/sql/aotv4_open_spells_and_skills.sql`. ⚠️ Running step 3 DESTROYS the record
of which classes had a special natively -- the only surviving copy is `skill_pool.NATIVE`.

⚠️ **The generator writes `spell_blacklist.lua` too — it MUST be regenerated with the pool.** When
> the custom set took over, the blacklist was emptied ("empty by design: the pool is hand-authored"),
> so re-pointing at the stock set without rebuilding it silently reopens every port, rez and
> Enchant-material spell. Current prune = **473** (travel 169 / summon-item 199 / illusion 23 /
> vision 19 / enchant 20 / rez 15 / cure-curse 7 / LDoN 18 / summon-corpse 3), leaving **2174
> offerable**. Travel is pruned partly because ports **defeat region locking**.
>
> ⚠️⚠️ **`teleport_zone` is NOT "destination zone" — it is "names an NPC *or* a zone".** Every
> pet/familiar/warder/Eye-of-Zomm spell stores its **summon type** there (SPA 33/71/152 pets, 106
> warders, 108 familiars, 67 eye). Keying the travel filter off `teleport_zone <> ''` pruned **95
> stock pet spells** — every Magician elemental, Necro pet, Shaman/Beastlord warder and Enchanter
> animation. Detect travel by **SPA** instead: 25 Bind Affinity, 26 Gate, 83 Teleport, 88 Evacuate,
> 104 Translocate. No travel spell lacks one.
>
> ⚠️ **`targettype = 3` was ALSO in the travel rule and is now REMOVED — do not add it back.**
> ST_Group is 3, which is *every* group-target spell, not a group teleport: it was pruning **78
> ordinary group buffs** (Elixir of Divinity, Wave of Marr, Eriki's Psalm of Power, the group heal
> lines) as if they were ports. Of the 150 `targettype=3` spells in the pool, the **72 that really
> are travel all carry one of the SPAs above** and are caught anyway — the clause bought nothing.
>
> ⚠️ **The junk rules must test PURITY, not presence** (2026-07-27). `illusion` (SPA 58) and `vision`
> (SPA 13/65/66/87) prune only when **every populated slot** is that effect or 254 — a spell is kept
> if the cosmetic bit rides along with something real. Written as "has the SPA" they took **31
> illusions that carry procs/stats/haste** (Boon of the Garou, Night's Dark Terror, Illusion:
> Werewolf) and, once illusion was narrowed, the vision rule inherited the same failure and swallowed
> the wolf/hunter forms (427, 1562, 1563, 3579, 4107) purely for their ultravision. Any new rule here
> gets the same treatment: prune the spell that does *only* the junk thing.

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
- ⚠️ **Combat specials: NATIVE ones are auto-granted, the rest are picker rewards.** Every class now
  has a `skill_caps` entry for all twelve (it must, or a picked reward cannot be granted), so
  `CanHaveSkill` is no longer a useful filter and the DB can no longer say what was native.
  `skill_pool.NATIVE` is the only record. `global_player.grant_native_combat_skills` grants a class
  its own on connect AND on level up (some cap curves only open above level 1);
  `spell_choice.gather_skill_candidates` offers only `skill_pool.rotatable_for(class)`.
- ⚠️ **`skill_pool.lua` is now a MODULE, not a bare table.** It exposes `.SKILLS`, `.NATIVE`,
  `.is_native()` and `.rotatable_for()`. `pairs(skills)` no longer yields skill ids -- use
  `pairs(skills.SKILLS)`. A `__index` metatable keeps `skills[id]` working for existing callers, but
  `pairs` does NOT follow it.
- **Bard `skill_caps`** (`class_id=8`) raised so skills scale; client also needs the exported
  `SkillCaps.txt` (`export_client_files`) installed in the EQ root.
- **Expansion lock:** `rule_values Expansion:CurrentExpansion = 0` (Classic).
- Test char `Ashrem`: charid **1**, GM — **Warrior (class 1)** as of 2026-07-29, not Bard; it was
  remade after the all-classes pivot (§14), so it also exercises the melee-as-caster path.
  (Reset a char's combat skills via
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
- ⚠️⚠️ **A Lua SPELL SCRIPT GETS AN EMPTY `e` WHEN THE TARGET IS AN NPC — fixed 2026-07-29,
  `zone/lua_parser.cpp`.** `SpellArgumentDispatch` is filled with `handle_spell_null` for every
  event (`:240`) and then only **four** are given a real argument builder. `EVENT_SPELL_EFFECT_NPC`,
  `EVENT_SPELL_EFFECT_BUFF_TIC_NPC` and the two BOT equivalents were **not among them**, so they
  fell through to `handle_spell_null` — a function with an **empty body**
  (`lua_parser_events.cpp:2182`). The event table therefore arrived with **no fields at all**: no
  `caster_id`, no `target`, no `spell_id`, no `tics_remaining`.
  - ⚠️⚠️ **The script still RUNS, which is why this hides so well.** `event_spell_effect` fires
    exactly as expected when you nuke a monster; every field on `e` is just nil, so a script that
    reads `e.caster_id` silently does nothing. The symptom is "the spell did not fire" with **no
    error anywhere** — and it is invisible to luacheck, to the zone log, and to reading the script.
  - **It only ever worked when the target was a PLAYER.** Beneficial/self spells land on a Client →
    `EVENT_SPELL_EFFECT_CLIENT`, which *was* registered. Of the 56 scripts in
    `quests/global/spells/`, **34 beneficial ones were fine and 22 detrimental ones were dead**:
    982, the retired customs 43010/43011/43015/43030/43052/43054/43055, helpers 43150/43155, **the
    whole Moonfire line 43312-43317** and the Sinew line 43318-43323.
  - ⚠️ **Moonfire had never worked in play.** Its 2× bonus heal (§5) reads `e.caster_id`, so against
    a monster it returned early every time and the line was a plain 1:1 lifetap paid entirely by the
    engine — for as long as it has existed. Nothing reported it.
  - The fix is registration only: `handle_spell_event` was **already written** for this case (its
    first branch is `if (mob)`, the NPC one), it was simply never wired to those four events.
  - 📌 **Before writing any new spell script, confirm the event you need has a real builder in
    `SpellArgumentDispatch`** — a missing entry is silent, not an error.
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
- ⚠️⚠️ **A custom AA must REUSE a native row — a new id never reaches the client.** The server
  resolves it, passes every check in `CanUseAlternateAdvancementRank` and sends the packet (proven
  with logging); the client silently discards it. Ability ids above the native max **30195** and
  rank ids above **65535** both fail this way, no error anywhere. Take over an existing ability
  instead: keep its id, rank ids, `title_sid` and rank chain, replace only `aa_rank_effects` and the
  `db_str` strings (`custom/sql/aotv4_aa_tank_hosted.sql`).
- ⚠️⚠️ **`aa_rank_prereqs` IS A SEPARATE TABLE AND MUST BE CLEARED TOO.** A hosted AA inherits its
  host's prerequisites, so it appears in the window but **refuses to train with no message**.
  Clearing `aa_rank_effects` does nothing to it. Eight AAs were untrainable this way.
- ⚠️ **Walk the rank chain via `next_id` — rank ids are NOT contiguous.** Natural Durability's chain
  is **107 → 108 → 109 → 7541 → 7542 → 7543**; assuming `first_rank_id + 0..4` wrote costs and levels
  onto ranks 110/111, which belong to a different ability. Terminate a chain early with
  `next_id = -1` to control how many ranks show (otherwise the window reads e.g. "0/15").
- ⚠️⚠️ **There is NO usable native "survive a killing blow" effect.** All three death hooks were
  checked (2026-07-26, Borrowed Breath) and every one fails for a different reason:
  - **SPA 232 `DivineSave`** (`TryDivineSave`) is the only one that runs inside the `HasDied()`
    branch, but it unconditionally casts **4789 Touch of the Divine** — SPA 40 Divine Aura for up to
    **36 s**. Under it you cannot attack (`attack.cpp:1743`), cannot cast (`spells.cpp:566`), and
    **no other caster can land any spell on you, heals included** (`spells.cpp:4147` — *"invuln mobs
    can't be affected by any spells, good or bad"*). You are also unattackable, so the boss drops
    the tank and turns on the healer. It keeps you alive by removing you from the fight. This is
    also exactly what the **"Second Chance" tribute** is (worn effect 5612/5613/5614 *Divine Res*).
  - **SPA 150 `DeathSave`** (Divine Intervention / Death Pact) is **not a death save here** —
    `TryDeathSave` is called from the *else* branch, only on crossing below 16 % health **alive**.
    Its chance also comes off the *saved player's* Charisma, which is backwards for a role AA.
  - **`SpellOnDeath`** (`TrySpellOnDeath`) always returns false on purpose; the comment
    (`mob.cpp:6636`) says a heal placed there cannot register before death kills you.
  - ⚠️ **Tributes are not a separate mechanism.** `CalcBonuses` resolves a tribute to an ITEM and
    runs it through `AddItemBonuses(..., is_tribute=true)` (`bonuses.cpp:177`), so a tribute can
    only ever deliver the same SPAs.
  - The working answer is a hand-rolled check in `Mob::Damage`'s `HasDied()` branch before the
    native saves — `Mob::AoTv4TryBorrowedBreath` (`zone/aotv4_healer_aa.cpp`). Return at a share of
    max HP, not 1 HP: at 1 HP the next tick of anything undoes the save.
- **The AA window tab is `aa_ability.type`**, not `category` (category is a sub-grouping header
  *within* a tab). 1 General / 2 Archetype / 3 Class / 4 Special, relabelled **Tank / Healer /
  Ranged / Melee** in `aotv4_client_install/uifiles_default/EQUI_AAWindow.xml` (a stock file we
  overwrite; no `<Include>` needed). Labels are XML-driven — the stock strings aren't in
  `eqgame.exe` — but page/listbox `item=` names resolve positionally, so rename only `<TabText>`.
- **AA names come from `db_str`** (type 1 title, type 4 description) resolved by the CLIENT out of
  its own `dbstr_us.txt`, so a `db_str` change needs `./export_client_files` + reinstalling that
  file. `title_sid = -1` renders **no row at all** — it does not fall back to `aa_ability.name`.
- ⚠️⚠️ **AN AA HAS *THREE* NAMES IN `db_str`, AND THE HOTKEY ONE IS SPLIT ACROSS TWO ROWS.**
  `aa_ranks` carries `title_sid` **and** `upper_hotkey_sid` **and** `lower_hotkey_sid`, and the
  client selects on `(id, type)`: **type 1** the window title, **type 2** the hotkey's UPPER line,
  **type 3** its LOWER line, **type 4** the description. Renaming an AA by writing types 1 and 4 —
  which is what `aotv4_aa_rename.sql` did — leaves the **host's** hotkey text in place, so the AA
  window reads "Iron Will" while a hotkey made from that same ability reads **"Frenzied Burnout"**.
  Fixed for all 10 activated abilities by `custom/sql/aotv4_aa_hotkey_names.sql` (first word to
  type 2, remainder to type 3; passives carry `-1` and are skipped).
  - ⚠️⚠️ **The split is why this is nearly unsearchable.** The string on screen never exists in the
    data: grepping `db_str`, `dbstr_us.txt` **and** `eqgame.exe` for `Frenzied Burnout` all return
    **zero**, because it is stored as `Frenzied` + `Burnout`. That produced three separate wrong
    conclusions (a stale `spells_us.txt`, a clone-source name, a hardcoded client string) before
    `SELECT id, type, value FROM db_str WHERE id = <sid>` — **dump every type for the sid** — showed
    it instantly. Search a single word, or dump the sid; never grep the rendered string.
  - ⚠️ In our pool `title_sid`, `upper_hotkey_sid` and `lower_hotkey_sid` are all the **same sid**,
    so these are three rows of one id. That is exactly what makes it look like fixing the title
    should have fixed the hotkey: the id matches, only the **type** differs.
- ⚠️⚠️ **`title_sid` and `desc_sid` are INDEPENDENT, and neither is guaranteed to equal
  `first_rank_id`.** 22 of our 23 hosts have `title_sid = desc_sid = first_rank_id`, which makes it
  look like a rule — **Quick Damage (44) does not**: `title_sid 141`, `desc_sid` **12863**. Writing
  the description to the title sid gave an AA with the right NAME and the host's ORIGINAL
  description, with no error anywhere. Always
  `SELECT title_sid, desc_sid FROM aa_ranks WHERE id = <first_rank_id>` before writing `db_str` for
  a new host, or repoint `desc_sid` to match the title sid as `aotv4_aa_ranged.sql` now does.
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
  - ⚠️ **Tier rows ALREADY inherit `price`/`sellrate` from the native — do not "fix" this** (checked
    2026-07-28: 0 of 54,292 tier rows differ from their base on either column). The clone is
    `INSERT … SELECT *` and nothing in the script touches those columns. 33,720 tier rows show a
    price of 0 only because **the base item's price is 0 too**; there is nothing to inherit.
  - ⚠️⚠️ **"Mythic won't sell" is NO DROP, not price.** `Handle_OP_ShopPlayerSell` bails at
    `if (!item->NoDrop) return;` (`client_packet.cpp:15221`) *before* price is read, and
    `Client::AdvLootSellValue` returns 0 for the same reason (`:10674`). A Mythic carries the native's
    full price and still cannot be sold to a vendor — that is the design, not a bug. The 17 Hallowed
    rows that also refuse are the **epics**, forced Lore + No Drop at the end of the tier script.
  Two C++ hooks (tracked patches): `NPC::ResolveTierDrop` (loot.cpp) decides **which tier** a rolled
  item drops as — **5% Mythic / 25% Hallowed / else base** (mode-independent, since
  `lootdrop_entries.chance` is a weight in the dominant weighted loot mode); `AoTv4MythicReward`
  (questmgr.cpp + lua_client.cpp) upgrades **quest-reward** gear to its Mythic tier (epics never hand
  out native). Re-run the SQL → rebuild shared memory → restart.
  - ✅ **CRAFTING ALWAYS YIELDS MYTHIC** (`ZoneDatabase::GetTradeRecipe`, tradeskills.cpp ~:1746). Every
    combine output is swapped for its Mythic tier when one exists, so crafted gear is top-tier and stays
    relevant as later expansions unlock better base items. Epics are tiered, so they are covered too.
    - ⚠️⚠️ **ONE choke point serves BOTH overloads.** `GetTradeRecipe(container, …)` resolves the recipe
      id from the container's contents and then **delegates** to `GetTradeRecipe(recipe_id, …)` (`:1600`,
      `:1639`), so the trade window and the world-container path both pass through here exactly once.
      Never copy the upgrade into a caller — two copies drift, per §22's reasoning.
    - ⚠️ It normalises through **`AoTv4TierBaseId` first**. Adding the step to an id that is *already* a
      tier would land outside the reserved band where no item exists, so the swap would silently no-op;
      normalising makes a Hallowed output upgrade and a Mythic output idempotent. (No stock recipe
      outputs a tier id today — 0 of 19,464 — so this is a guard, not a live path.)
    - 📌 **Only WEARABLES are affected, and that falls out of the data rather than a test.** The gate
      used to be `Slots > 0`, which was redundant: `aotv4_gear_tiers.sql` only generates tiers for
      slots>0 items, so the "does a Mythic exist" lookup already *is* the wearable test. Of the enabled
      recipes, **12,323 produce a wearable** (all 8,889 distinct outputs have a Mythic row) and **11,031
      produce a non-wearable** — 5,112 plain components, 728 food/drink, 71 containers, and 5 junk
      outliers. **Do not tier those**: a component has no stats to scale, so a "Mythic" one would be
      byte-identical to its base and would only split **2,556** component stacks across two ids.
    - ⚠️ Only the **success** list is upgraded — `onfail` and `salvage` are untouched, and quest recipes
      (5 of 19,464) hand out items from Lua/Perl and bypass this entirely.
  - ⚠️⚠️ **A TIER REPLACES THE DROP, IT DOES NOT ADD ONE (fixed 2026-07-29).** The original
    `AddTierUpgrades` rolled the two tiers **independently** and called `AddLootDrop` for each *on top
    of* the base, so one base item could put **three items on the corpse at once** (base + Hallowed +
    Mythic). A lucky roll meant **more** loot rather than **better** loot, and every upgrade dragged
    its own inferior copies along with it. Now one item goes in and one comes out.
  - ⚠️ The 5/25 rates are unchanged but are now **mutually exclusive bands of a single roll**, so
    **Mythic is tested first** — it is the rarer band and testing Hallowed first would swallow it
    entirely. Expect marginally fewer Hallowed than before (25% of drops, not 25% plus the 5% that
    also rolled Mythic).
  - **The tiers are always created in PAIRS** — `aotv4_gear_tiers.sql` generates Hallowed and Mythic
    from the same base set, and the DB confirms it: 27,146 rows each, **zero** Hallowed without a
    Mythic and zero Mythic without a Hallowed (checked 2026-07-29). So the split really is a clean
    5 / 25 / 70, and the only item that is native 100% of the time is one with **no tier rows at all**
    (quest paper, whatever the tier script skipped), which returns the base untouched.
  - 📌 The Mythic branch **falls through** to the Hallowed test instead of returning, so a half-tiered
    item would degrade to Hallowed rather than to base. Given the pairing invariant above that branch
    is **unreachable** — it is a safety net for a hand-edited or partially-regenerated tier set, not a
    live code path. Do not read it as evidence that half-tiered items exist.
  - ⚠️ In the weighted path the **`multiplier`/charges loop rolls its own tier per copy** — it
    previously got no upgrade at all, and sharing one roll would hand out one upgraded item plus N
    base duplicates.
  - 📌 loot.cpp now uses `zone/aotv4_tiers.h` (`AOTV4_TIER_STEP` / `AoTv4IsTierId`) instead of literal
    `300000`/`600000` — it was touched for another reason, which is exactly the condition that header
    names for migrating one of the five hardcoded copies. Four remain (npc/questmgr/attack/tradeskills).
  - ✅ **QUESTS ACCEPT ANY TIER — do not "scrub" the 8,000 quest scripts, it is already handled in
    five C++ choke points** (audited 2026-07-28). `zone/aotv4_tiers.h` holds the id maths now
    (`AoTv4TierBaseId`, `AOTV4_TIER_STEP`); the five OLDER hardcoded copies (loot/npc/questmgr/attack/
    tradeskills) were deliberately left alone — migrate one only if it is touched for another reason.
    | Path | Covers |
    |---|---|
    | `NPC::CheckHandin` (npc.cpp, pre-existing) | **all turn-ins** — Lua `check_turn_in` (1,039 files) *and* Perl `plugin::check_handin` (1,281) both funnel here |
    | `Lua_Client::CountItem` / `Perl_Client_CountItem` / `QuestManager::countitem` | possession checks |
    | `Lua_Client::HasItemOnCorpse` | the corpse fallback inside `Client:HasItem` |
    | `Client:HasItem` (`lua_modules/client_ext.lua`) | ~230 Lua files — it **scans slots itself**, never calls CountItem, so it needs its own `aotv4_same_item` |
    | `Perl_Client_HasItemOnCorpse` (**added 2026-08-01**) | the corpse fallback inside `plugin::check_hasitem` |
    | `plugin::check_hasitem` (`plugins/check_hasitem.pl`, **added 2026-08-01**) | **133 Perl files** — the Perl twin of `Client:HasItem`, and it scans slots itself the same way |
    - ⚠️ **Scoped to the QUEST bindings, never `Client::CountItem` itself** — the engine calls that for
      merchants, corpses and NPC inventories, where "a Mythic Rusty Dagger is a Rusty Dagger" is the
      WRONG answer.
    - ⚠️ 7 Perl files compare `$itemN ==` directly and bypass all of it. Audited: 5 of the 6 ids are
      untiered quest paper, and the 6th (26015 Pulsing Goo) only ever appears as a
      `quest::ChooseRandom` reward roll — a script-local, never a hand-in. **No real gaps.**
    - ⚠️⚠️ **THE 2026-07-28 AUDIT MISSED THE WHOLE PERL POSSESSION PATH** (found and fixed 2026-08-01).
      It reasoned in terms of the *Lua* call graph, fixed `Client:HasItem` when it turned out to scan
      slots itself — and never asked whether Perl had the same shape. It does: `plugin::check_hasitem`
      does a raw `GetItemIDAt($slot) == $item_id` (plus the same on every augment socket), reaching
      none of the C++ choke points, so **133 Perl quest scripts told a player holding only the Mythic
      copy that they did not have the item.** Its corpse fallback `Perl_Client_HasItemOnCorpse` was
      unnormalised too, while `Lua_Client::HasItemOnCorpse` had been fixed. The tell was in plain
      sight: the comment in `client_ext.lua` lists the covered paths and names `Perl_Client_CountItem`
      but not `Perl_Client_HasItemOnCorpse`.
      📌 **`aotv4_same_item` now exists TWICE — `lua_modules/client_ext.lua` and
      `plugins/check_hasitem.pl`. Fix them together.** And when auditing a rule like this, enumerate
      by *mechanism* (who scans slots directly?) rather than by language, or one interpreter's half
      gets checked and the other does not.
    - 📌 **Crafting makes this load-bearing rather than cosmetic.** Every combine output is swapped for
      its Mythic tier (§ the crafting note above), so **25 quest turn-in requirements are craftable
      wearables** whose only obtainable form is now Mythic — if the normalisation broke, those quests
      would be uncompletable rather than merely awkward. Verified 2026-08-01: all 25 have Mythic rows,
      none is touched by the 7 direct-comparison files, and the Lua chain resolves
      `items.check_turn_in` → `trade.self:CheckHandin` → `NPC::CheckHandin` (`npc.cpp:4376`).
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
      relative "when" via `ShopWhenStr`. EQUI_ShopWnd.xml is CX564×CY600. **Needs a dll rebuild.**
      - ⚠️ **The "keep it to ONE listbox" and "no native tabs" rules recorded here were WRONG** — see
        §16's corrections. A second listbox failed once because that XML was malformed, not because
        the client forbids it (`EQUI_AoTSpellBook.xml` ships four), and the hand-rolled toggle-button
        tabs were unnecessary: `TabBox`/`Page`/`TabText` are stock and are what the AA window uses
        (§6). Both windows are worth rebuilding on the real widgets when they are next touched.
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
- ✅ **Loot / Loot All are HIDDEN while the group owns the decision** (2026-07-28). **Loot** goes when
  the *selected* row is still votable (`RowIsVotable` — a "Rolling N" status, not `Yours`/`Free Grab`/
  `Won by`); **Loot All** goes for the whole session under any non-FFA `g_lootMode`, since it would
  sweep contested rows along with settled ones.
  - ⚠️⚠️ **This is PRESENTATION, not enforcement.** `AdvLootManager::CanLoot` (`zone/advloot.cpp`)
    already refused both cases server-side *with a reason* — racing the button never worked, it just
    LOOKED like it should, and a live button that silently does nothing reads as a broken window
    rather than as a rule. **Never move the check to the client**: a modified dll would just un-hide.
  - ⚠️ **HIDDEN, not greyed.** There is no address-mapped enable setter on this build — `eqgame.h`
    maps nothing for it and `EQClasses.h` only *declares* `CXWnd::IsEnabled` — and writing a raw
    `->Enabled` member offset is the unreliable-struct-offset trap from §13. `Show` **is** mapped.
  - ⚠️ **FFA is deliberately untouched**: there the server permits looting and first-come *is* the
    rule, so hiding the button would break ordinary group play rather than protect it.
  - ⚠️ `ApplyVis` now takes the selected row as a parameter and the list's click handler passes
    `m_sel` — `GetCurSel()` is one click behind inside that notification, so a live read would briefly
    offer Loot on a row that is still rolling. Safe to call from there because `ApplyVis` touches only
    buttons and labels, never the listbox (rebuilding a list inside its own click is the §3 Death Book
    crash).
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

## 17c. Roaming world boss — `#worldboss` — 2026-07-26

A boss armed for a random classic dungeon; the server is told; everyone who damages it can roll on
the corpse. **No raid** — loot rights are granted directly, which disturbs nobody's group and covers
latecomers. Pure Lua + SQL, no C++. **Test plan + known failure points: `WORLD_BOSS.md`.**

- **Files**: `lua_modules/aotv4_worldboss.lua` (core), `lua_modules/commands/worldboss.lua` (`#worldboss`
  / `status` / `clear`, access **200**), `custom/sql/aotv4_worldboss.sql`
  (npc **2000200 `#The_Nameless`**, loottable 200020). Hooks: `global_player.event_enter_zone` →
  `on_enter_zone`, `global_npc.event_death_complete` → `on_death`.
- ⚠️⚠️ **A LUA COMMAND IS REGISTERED IN `lua_modules/command.lua`, NOT in `command_settings`.** That
  table is for **C++ commands only** — the zone deletes any row whose command has no C++ handler,
  logging *"Command [x] no longer exists. Deleting orphaned entry from `command_settings`"*. A file
  in `lua_modules/commands/` that is not listed in `command.lua`'s `commands` table is simply never
  reachable. **`#worldboss` was unregistered and therefore did nothing from the day it was written
  until 2026-07-27**, which is also why it was never caught: it was only ever going to fail the
  moment somebody typed it. `commands["name"] = { access, require("commands/file") }`.
- ⚠️ **ARMING AND SPAWNING ARE SEPARATE, and must be.** You cannot spawn into a zone nobody occupies:
  zones are dynamic (idle ones self-terminate) and `eq.spawn2` is zone-local. So the command records
  `aotv4_worldboss = "zone|npcid|expiry"` in a GLOBAL bucket + announces, and the **first player to
  enter that zone spawns it** — on their own coordinates, which guarantees valid placement in any
  zone with no per-zone spawn-point table. The bucket is cleared BEFORE spawning so two simultaneous
  arrivals can't both spawn one. Lapses after `WINDOW_MINS`.
- ⚠️ **Raid is READ-ONLY from Lua** — `Lua_Raid` has no `AddMember` binding (the C++
  `Raid::AddMember` exists). Auto-raid would need a new binding; that's why loot rights were chosen
  instead. `corpse:AddLooter(mob)` + `mob:GetHateList()` are both bound, and `MAX_LOOTERS = 72`.
- ⚠️ **Region locking still applies** — a boss in a region nobody has unlocked is unreachable. Keep
  `M.ZONES` inside open regions.
- Zone list = expansion 0, ≥25 spawn points, **zero merchant NPCs** (good "not a city" proxy), minus
  the gated planes → 14 classic dungeons.
- 📌 Tuning (level 50 / 120k hp) and the 5-item placeholder loot pool are **guesses until it is
  fought**. Scheduling is deliberately not built yet — GM command only.

## 17b. Shield Wall — `/shield` damage splitting — 2026-07-26

Several players share one monster's melee damage. **Player-facing doc: `SHIELD_WALL.md`.**
Built by opening up the native `/shield` (Warrior-only, 12s, one shielder) rather than writing a
parallel system — the native path already does the pairing, the distance break and the teardown.

- **Split maths** (`Mob::ApplyShieldWall`, `zone/attack.cpp`): `share = damage * (100 + penalty*(N-1))
  / (N*100)`. Penalty 20 → 2 sharers take **60% each** (120% total), 3 take ~47% (140%). Sharing is
  deliberately **lossy**: it converts one spike into several survivable hits at a cost in total HP,
  so stacking bodies is progressively worse value. Everyone takes the **same** share incl. the aggro
  holder — the stock shielder mitigation is NOT applied on top, or the tank would take more than the
  helpers. ⚠️ Multiply before dividing or small hits truncate to nothing.
- ⚠️ **Stock allows exactly ONE shielder** (`m_shielder_id`; `ShieldAbility` rejects a second with
  `ALREADY_SHIELDED`). Added `m_shield_wall` (vector on the TARGET) as the source of truth, with
  `m_shielder_id` kept in sync with its head so every native teardown path still works. Distance is
  re-checked **per hit**, which is what makes a permanent pairing safe.
- ⚠️ **`/shield` is now a TOGGLE** (`Handle_OP_Shielding`). With a permanent duration there was
  otherwise no way to stop short of walking away or dying. Toggling off costs no recast.
- "Permanent" = an 86400000 ms timer, not a disabled one — `ShieldAbilityFinish()` keys off
  `shield_timer` to tear down cleanly.
- Rules: `AoT:ShieldAnyClass` / `ShieldMinLevel` / `ShieldPermanent` / `ShieldRecastSeconds` /
  `ShieldDistance` / `ShieldWallPenaltyPercent` / `ShieldWallMaxSharers`. **Needs a zone rebuild.**
- **Visible "Shielded" buffs** = spells **43380-43382** (`custom/sql/aotv4_shield_wall_buff.sql`),
  inert markers rebuilt by `ShieldWallRefreshBuff()` on every wall change.
  ⚠️ **One spell id PER SHIELDER, not one for the wall.** EQ will not stack the same spell id from
  two different casters — a second cast of 43380 overwrites the first — so three identical rows with
  distinct `spellgroup`s are needed for three shielders to each show as their own buff. Count must
  track `AoT:ShieldWallMaxSharers` minus 1; extra shielders beyond the row count go uncredited.
  ⚠️ It lives at **43380 (helper band 43350+)**, NOT 43300-43349 — the pool generator pulls that band
  wholesale and would offer "Shielded" as a level-up reward.
  ⚠️ **Cast it FROM THE SHIELDER, never via `ApplySpellBuff`.** That helper does
  `SpellOnTarget(spell_id, this)` on the *buff holder*, so the buff records the shielded player as
  its own caster and inspecting it reads "cast by yourself". Buffs carry `casterid` + `caster_name`
  (`Buffs_Struct`, `zone/common.h:228`) and the client shows them on inspect — casting from the
  shielder is what puts a name there. (The buff's *name* is still fixed by the spell row; the caster
  field is the part that identifies who.) `ShieldWallRefreshBuff()` re-applies from the head of the
  wall whenever it changes, so it never credits someone who has already stopped.
  ⚠️ Three places mutate the wall — `ShieldWallRemove`, the per-hit distance prune in
  `ApplyShieldWall`, and `ShieldAbilityClearVariables` — **all three must fade/refresh the buff** or
  it strands on the player.
- ⚠️ **UNTESTED at runtime** as of writing — it compiles and links, but nothing has swung at a
  shielded pair yet.
- 📌 **TODO — the Shielded buffs are PLACEHOLDERS.** Using real buff slots to display transient state
  works but is the wrong long-term shape: it burns a buff slot per shielder, caps at however many
  spell rows exist (43380-43382), and every future short-lived effect would need its own throwaway
  spell row the same way. **Build a proper tracker for short buffs / autoskills** — a purpose-made
  display fed from server state rather than the buff bar — and move Shield Wall onto it. Same problem
  already shows up in the inert-marker buffs elsewhere (43022 Divine Aura, 43035 Blade Turn, 43056
  Counterattack, the Thirst line): they exist to be *seen*, not to do anything.

## 17. Database access from Windows + container recovery — 2026-07-24

> ⚠️⚠️ **See §25 first.** This section's "a rebuild does NOT destroy the DB — the old container still
> has it" is correct but incomplete: the same root cause also produces a **stale-but-present**
> database that looks entirely healthy. `db_sanity.sh` detects it in one second.

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

## 18. Server-wide buffs — `#worldbuff` — 2026-07-27

`#worldbuff <spellid> [minutes]` puts any spell on everybody, everywhere. Also `status` and `clear`.
Access **200**. Pure Lua, no C++.

- **Files**: `lua_modules/aotv4_worldbuff.lua` (core), `lua_modules/commands/worldbuff.lua` (the
  command), registered in `lua_modules/command.lua`. Hooks: `global_player.event_connect` and
  `event_enter_zone` → `on_player`, `event_timer "worldbuff"` → `on_sweep`.
- ⚠️ **THERE IS NO SINGLE CALL THAT REACHES THE WHOLE SERVER.** Lua can only enumerate clients in its
  OWN zone (`eq.get_entity_list():GetClientList()`), and every zone is a separate process. The
  cross-zone bindings are all targeted — `cross_zone_cast_spell_by_char_id` / `_client_name` /
  `_group_id` / `_guild_id` / `_raid_id` / `_expedition_id` — there is no "by everyone". So coverage
  is assembled from four places: the command buffs its own zone at once, `event_connect` catches
  logins, `event_enter_zone` catches zoning, and a per-client repeating timer catches the player who
  is parked somewhere and does neither.
- State is ONE global bucket, `aotv4_worldbuff = "spellid|expiry_epoch"`, so every zone reads the
  same thing and it survives restarts. Same shape as `aotv4_worldboss`.
- ⚠️ **`ApplySpellBuff`, not `SpellFinished`.** `SpellFinished` runs a real cast: it can be resisted
  and it honours target type, so a beneficial spell with the wrong `targettype` will not land on an
  arbitrary player. `ApplySpellBuff` puts the buff straight on, which is what "the GM said everyone
  gets this" has to mean — and it is why **any** spell id works rather than only self/single-target.
- ⚠️ The sweep re-checks with `FindBuff` before applying, so it does not reset the duration every
  tick; a 10-minute buff lasts 10 minutes, not as long as the window.
- ⚠️ **The timer is PER CLIENT**, so `on_sweep` does per-client work. Calling `apply_zone()` from it
  would walk the whole client list once per player per sweep.
- `clear` stops new grants; buffs already handed out run out on their own.

## 19. Autoskill window — `/autoskill` — 2026-07-27

⚠️ **The autoskill SYSTEM predates this and was NOT changed.** It was already: `GetAvailableAutoSkills`
(the ten activated specials), `GetAutoSkillsList` (those you have), `Get/SetAutoSkillStatus`
(per-skill on/off in `autoskill.<id>` data buckets), and the auto-fire loop in `Client::Process` that
runs enabled specials while auto-attacking a **non-client** target. `#autoskill [skill]
[enable|disable|status]` / `#autoskill list` still work untouched. Section 19 is only the **window**.

- **Server**: `Client::SendAutoSkillData` / `HandleAutoSkillSay` / `GetAutoSkillCooldown` /
  `GetAutoSkillReuse`, all in `zone/special_attacks.cpp`; say intercept in
  `Client::ChannelMessageReceived` beside the AdvLoot one. **Needs a zone rebuild.**
- **Client**: `core_autoskill.cpp/.h` (own TU, no detours) + `EQUI_AoTAutoSkillWnd.xml`, flag
  `areAutoSkillWindowEnabled`. `/autoskill` from `core_bazaar.h`. Install:
  `aotv4_client_install/AUTOSKILL_WINDOW_INSTALL.md`.
- **Protocol**: `ASKILLDATA <n>^skillid|name|enabled|cooldown_secs|reuse_secs^…` out;
  `/say askset <skillid> <0|1>` and `/say askrefresh` in.
- ⚠️ **`GetAutoSkillCooldown` lives in `special_attacks.cpp` on purpose** — `AOTV4_SKILL_TIMER_BASE`
  is file-scope there, and duplicating it elsewhere is how the two silently drift apart.
- ⚠️⚠️ **TAUNT USES A DIFFERENT TIMER.** Every other special uses `AOTV4_SKILL_TIMER_BASE + skill`;
  Taunt runs on the stock `pTimerTaunt` (the auto-fire loop special-cases it for the same reason).
  Reading the wrong one reports a permanently-ready Taunt.
- ⚠️⚠️ **`PersistentTimer::GetRemainingTime` returns `0xFFFFFFFF` when the timer exists but is
  DISABLED**, not 0. Unguarded, a ready skill displays a 49-day cooldown. (`PTimerList` returns a
  plain 0 for a timer that was never created at all, which is fine.) Reuse times are **seconds**
  (`BashReuseTime = 5`, `common/features.h`).
- ⚠️ **The countdown runs CLIENT-side.** The server sends remaining seconds; the dll ticks them down
  per frame and resyncs every 2.5 s **only while the window is on screen**. Streaming a line per
  second per player is exactly the chat burst RoF2 drops (§15). Closed window, zero traffic.
- ⚠️⚠️ **THE TIMERS TAB IS GONE (2026-07-28) and should not come back in that shape.** It listed the
  firing skills with Ready In and Reuse columns, and that does not work: **Bash and Kick reuse in about
  FIVE SECONDS** (`BashReuseTime = 5`, `common/features.h`), so the number was ready again before the
  eye found it and the page looked permanently broken. A grid of buttons with the countdown drawn on
  each face was tried next and was not wanted either. The window is now a SINGLE page, no `TabBox`.
- ✅ **The real lever is a CAP, not a display.** `AOTV4_AUTOSKILL_MAX = 4` (`zone/special_attacks.cpp`)
  limits how many abilities may be enabled at once, enforced in `Client::HandleAutoSkillSay` -- the
  only path that turns one on. Without it the correct play is to enable everything and the choice of
  which specials to run stops being a choice. ⚠️ Enforced SERVER side on purpose: a client-side cap is
  bypassed by the `/say askset` the window sends, or by typing `#autoskill`. Turning one OFF is always
  allowed. The dll mirrors it in `AUTOSKILL_CAP` only so "All On" does not ask for a fifth and collect
  a red refusal per extra skill.
- Cooldowns are still parsed and ticked locally even though nothing shows them, so a future short-buff
  / autoskill tracker (§17b) finds them already running.
- ⚠️ **The list is rebuilt only when the rows change** (skill gained, or one toggled) — `Refresh()`
  calls `DeleteAll`, which clears the selection, so rebuilding on every resync would yank it out
  from under the player. Otherwise only the timer column is rewritten.

## 20. Spell window tabs — Choose / Known / Pool — `/journal` — 2026-07-27

⚠️ **This began as a SEPARATE window and is not one any more.** `EQUI_AoTSpellJournalWnd.xml` and a
`SpellJournalWnd` class existed for one iteration, then the two panels were folded into the EXISTING
reward picker so there is a single spell window. The standalone XML is kept as a backup and is
**unused** — do not copy it or `<Include>` it. `core_spelljournal.cpp` now owns **no window**: it is
the data and rendering service behind two tabs whose widgets belong to `core_spellchoice_native.cpp`.

**Three tabs**: **Choose** (the reward cards, icons and two-step Confirm — UNCHANGED),
**Known**, **Pool**. **Known** = what this character has scribed, grouped under
level-band headers. **Pool** = what can still be OFFERED at a chosen level, with a `<<`/`>>` stepper.
The Pool tab is the point: the pool is **2,154 spells over 78 levels** and was previously invisible —
a player could only ever see what they had been handed, never what they might be.

- **Files**: `core_spelljournal.cpp/.h` (own TU, no detours, no window) + the Known/Pool widgets in
  the GENERATED `EQUI_AoTSpellChoiceWnd.xml`, flag `areSpellJournalEnabled`. `/journal` and
  `/spells` from `core_bazaar.h` call `SpellChoiceOpen()`. Server: `spell_choice.send_pool` /
  `handle_journal_say`, routed from `global_player.event_say`. Install:
  `aotv4_client_install/SPELL_JOURNAL_INSTALL.md`.
- **Protocol**: `SJPOOLDATA <level> <chunk> <chunks>^id:known,id:known,…` out; `/say sjpool <level>` in.
- ⚠️⚠️ **The Pool tab lists TWO pools, and combat abilities are the second one.** `spell_pool.lua` is
  spells only; the activated combat abilities live in `skill_pool.lua` and were invisible on the tab
  until 2026-07-28, which read as "not obtainable" — wrong, the picker spends a slot on one roughly
  every eighth level. They ride the same line as **`k<skillid>:<known>:<name>`** and the client lifts
  them to **`SJ_SKILL_BASE (1000000) + skillid`** so one `sjinfo <id>` can ask about either kind.
  ⚠️ Without the offset a skill id (0-76) *is* a valid spell id — `sjinfo 8` would describe spell 8.
  ⚠️ The **name is on the wire** because this dll has no skill-name table; hardcoding one is the
  `kIcons[]` drift trap again (§3). ⚠️ `SKILL_ID_BASE` (spell_choice.lua) and `SJ_SKILL_BASE`
  (core_spelljournal.cpp) must match. Abilities are **not level-banded** (`rotatable_for(class)`
  ignores level), so they list once at the top for every level; a class never sees its own natives.
- ⚠️ **Bard songs ARE in the pool** (150 of them, ~1-2 per level, only 3 blacklisted) — they are just
  sparse and carry no marker, so "no songs" is nearly always "did not spot one" rather than a filter.
- **Adapted from `New folder (2)/AoT_Spell_Book.cpp`** (an earlier AoT generation). That window is
  where the two-native-tabs + four-listboxes layout was first proven on this client — it is the file
  §16 cites. What was deliberately NOT taken:
  - its **progression model** (one spell slot per character level, tiers of 5, a Roll button, mastery
    ranks bit-packed into synthetic spell ids: `15-13` mastery / `12-8` tier / `7-0` index). Ours
    scribes real `spells_new` rows chosen three at a time on level-up; none of that machinery applies.
  - its **transport** — it talks over opcodes (`0x196A`/`0x196B`, range 6500-6504). This dll has **no
    raw-packet send at all** and every window it owns rides the shared `dsp_chat` detour.
- ✅ **Descriptions are built CLIENT SIDE** from the client's own spell record (`GetSpellByID`, then
  `Attrib[]`/`Base[]`/`DurationType`). This is the technique worth having from that reference. It
  removes the standing two-source problem in §14: the picker's text comes from `spell_desc.lua` over
  `SPELLDESCDATA` while the spellbook's comes from `db_str` via an exported `dbstr_us.txt`, and those
  must be kept in sync by hand. Text derived from the spell cannot drift from it and costs no traffic.
  ⚠️ The SPA label table is deliberately NOT exhaustive — an unknown SPA falls through to a generic
  `SPA n: base` line so it is visibly unlabelled rather than silently absent.
- ⚠️ **IDS ONLY on the wire; the client resolves names.** Sending names would roughly quadruple the
  payload (level 70 holds 125 pool spells: ~1.1 KB as `id:known` vs ~4 KB with names).
- ⚠️ **And it is CHUNKED anyway (60/line).** An oversized chat line is silently TRUNCATED, which looks
  like a short pool rather than an error — and §15 already records RoF2 dropping/reordering bursts.
  Each line carries its own chunk index, and a chunk whose level ≠ the level on screen is **dropped**
  (chat can arrive out of order; without that guard two levels would merge into one list).
- ⚠️ **Row index is NOT a data index on either list** — Known inserts a band header every 10 levels,
  so both lists keep an explicit row→spell-id map. Same trap as the Autoskill Timers page (§19).
- ⚠️ **An all-254 spell shows "Handled by the server; no client-visible effects."**, not "no effects".
  Several of ours are deliberate inert marker buffs paid by Lua (the Thirst line 43342-43347, the
  Shield Wall buffs). The wording is deliberate.
- The Known tab costs **zero** server traffic — it reads `CHARINFO2::SpellBook` directly.

### ⚠️⚠️ 43000-43112 is DELETED (2026-08-01) — retired from the pool 2026-07-27, rows removed now
That band was the 113-spell custom reward set (Ember, Zap, Kick, Strike, Counterattack, Moonfire…):
mostly redundant with native spells, and being all-slots-254 inert markers they rendered as "no
effects" in any client-side description. The generator's clause that offered them was removed on
2026-07-27 (pool 2,174 → 2,154); **the rows themselves went on 2026-08-01** via
`custom/sql/aotv4_retire_43000_43112.sql`, together with everything that called them:
- deleted `lua_modules/aotv4_pets.lua` and `lua_modules/aotv4_summon_table.lua`
- deleted the 14 scripts `quests/global/spells/{43010,43011,43015,43017,43025,43027,43028,43030,
  43035,43052,43054,43055,43056,43059}.lua`
- `global_player.lua` — the pets require, the **repeating 5-second `aotpet` timer** and its branch
- `global_npc.lua` — the pets require, `on_pet_spawn`, and the **whole `event_damage_given`**, which
  existed only to drive the summoned-pet behaviours. ⚠️ The player-side damage hooks (Thirst, Sinew,
  reactions) are in `global_player.lua` and are untouched — do not re-add it looking for them.
- `aotv4_reactions.lua` — the four reaction branches (Divine Aura 43022, Blade Turn 43035,
  Counterattack 43056, Vengeful Aura 43059), their charge tables, tuning and `arm_`/`clear_` API.
Full restore of all 202 original custom rows is `custom/sql/aotv4_custom_spells_backup.sql`; a
targeted pre-delete dump of 43000-43199 is `peq_pre_43xxx_purge.sql.gz`.
- ✅ **`on_damage_taken` got measurably cheaper**, which matters because it runs on **every damage
  event for every player**. It used to resolve `CastToClient` + `CharacterID` **and an
  `eq.get_entity_list():GetMobID()` lookup — an entity-list walk per hit** — before its first
  early-out, purely to feed those four branches. All three are gone; only the duel block remains.
- ⚠️⚠️ **42 STOCK ITEMS REFERENCE THE DELETED BAND, AND THAT IS EXPECTED — LEAVE THEM ALONE.** The
  50xxx → 43xxx renumber (§14) landed on ids **stock EQ already used**, so items 143000+ (*Tome:
  Breather*, *Tome: Cyclone Roar*, *Tome: Insult*, *Tome: Warrior's Bulwark*…) carry a `scrolleffect`
  pointing into it. Those stock disciplines were overwritten and no longer exist anywhere in
  `spells_new` — checked by name. The tomes are **unobtainable here** (0 rows in `lootdrop_entries`,
  0 on any merchant; level 70+ content behind a Classic lock and a level 30 cap), so the collision
  was never reachable. They now point at nothing rather than at the wrong spell, which is strictly
  better. 📌 Revisit only if a future expansion unlock makes them obtainable.
- ⚠️ **The 11 helpers at 43150-43199 are now ORPHANED and were deliberately kept.** 43150 (Open
  Wounds mark) was applied by 43052 and 43155 (Duel lock) by 43030, both in the deleted 113, so
  nothing applies them. `43150.lua`, `43155.lua` and the `aotv4_reactions` code that reads them still
  compile and still work — they simply never fire. Removing them is a separate decision.
- ⚠️ `aotv4_abilities.lua` **survives**: `43150.lua` requires it, and `aotv4_reactions.bleed_tick`
  still uses `ab.SKILL_OFFENSE`. It is no longer only-for-the-retired-set.
- ⚠️ Two names survive in shared memory and are **not** leftovers: `Smolder` and `Counterattack` are
  genuine **stock** spells (917 and 42175) that happened to share names with the customs.

⚠️⚠️ **"Get rid of the 43xxx spells" must NEVER be read as the whole range.** The band is not
homogeneous and everything from 43150 up is live or still referenced: **43150-43199** the helpers
(11, now dormant — see above), **43300-43349** the custom spell lines (reptile, sloth, moonfire,
promised, kindred, mark, thirst — all still offered), **43350-43399** their triggers plus the Shield
Wall buffs, **43400-43454** the AA-tree buffs and pet wards, **43500-43565** the class auras,
**43576-44327** the 752 spell-rank rows (§29). Deleting the range would destroy all four AA trees,
Shield Wall, the pet wards, the achievement auras and every spell rank.
📌 The 2026-08-01 purge above is the *only* part of the band that was safe to remove, and it took a
five-way reference check first — pool, other spells, AA, items and the Lua call graph.

### The tabs live in the GENERATED picker XML (`gen_choice_xml.pl`)
⚠️ `EQUI_AoTSpellChoiceWnd.xml` is generated, so the Known/Pool widgets are emitted by
`aotv4_client_install/gen_choice_xml.pl`, NOT hand-written. Rerun it after any pool regen (it already
had to be rerun for the icon set) and copy the result. The generator now emits three `<Page>`s inside
one `<TabBox>`.
⚠️ **The `<Screen>` lists ONLY `ASC_Tabs`.** Every other piece belongs to a page; naming one on the
screen as well places it twice and it draws outside its tab.
⚠️ The card coordinates were NOT moved — each page sits at `Y=TAB_Y` (22) and its pieces are
positioned relative to the page, so the Choose layout is byte-for-byte the old one. The window grew
450 → 500 tall to pay for the tab strip.
⚠️⚠️ **THAT `Y=TAB_Y` IS LOAD-BEARING — A `Page` IS NOT AUTOMATICALLY PLACED BELOW ITS TAB STRIP.**
A page's origin is the TabBox's top-left corner, **tabs included**, so a page at offset 0 puts its
first row of pieces directly ON TOP of the tab captions and both texts overprint into an unreadable
smear. The strip has to be paid for by hand on every page. Cost a round trip on
`EQUI_AoTLostWnd.xml` (§6) when its flat layout was restructured into tabs and the offset was left
at 0; 24 clears it on this build.
⚠️⚠️ **CONVERTING A FLAT WINDOW TO TABS DROPS ITS ANCHORS IF YOU ONLY MOVE THE PIECES.** Anything
that should grow with the window needs `AutoStretch` **plus** its four anchor tags; a piece without
them keeps its fixed `Size` forever, so dragging the window larger just adds empty space around a
small list. Same file, same session, immediately before the offset bug — the two together made a
resizable window look completely broken.
⚠️ A **sizable** window has its size AND position **saved per character** by the client, so an
edited `<Size>` only applies to a character that has never opened it. If it comes back wrong, drag
it once or delete its entry from `UI_<char>_<server>.ini` — the XML is not being ignored.
⚠️ `validate_ui_xml.sh` counts tags **inside comments**, so prose like `UI_<char>_<server>.ini` or
a bare `<Size>` in an explanatory note fails the tag-balance check. Write those without angle
brackets; the failure names a tag that does not exist in the markup at all, which is confusing.
- `SpellChoiceShow()` still refuses to open with nothing pending (Ctrl+Q's "is a reward owed" test
  keeps its meaning); **`SpellChoiceOpen()`** is the new browse entry point that ignores that.
- The transport calls back via `SjSetPoolReadyCallback` rather than touching the window, so
  `core_spelljournal` stays ignorant of who owns the widgets.

### ⚠️⚠️ Forward-declaring a client widget type in one of OUR headers
`core_spelljournal.h` was the first header in this dll to take a `CListWnd*`/`CXWnd*` in a signature,
and the obvious way to do it is wrong:
```cpp
class CListWnd;   // WRONG -- declares a second, unrelated ::CListWnd
```
The client classes live in **`namespace EQClasses`**, and `EQClasses.h` ends (line ~6973) with
`using namespace EQClasses;`. A global forward declaration therefore creates a *different* type, and
every later mention becomes **`'CListWnd': ambiguous symbol`** plus
**`cannot convert from 'EQClasses::CXWnd *' to 'CXWnd *'`** — over a hundred errors from one line,
almost all of them cascades that point nowhere near the cause.
```cpp
namespace EQClasses { class CListWnd; class CXWnd; }   // RIGHT
void SjRefreshKnown(EQClasses::CListWnd* list);        // and QUALIFY the signatures
```
Qualify rather than adding a global `using`, so the header works whether it is included before or
after `EQClasses.h`. Inside a `.cpp` the bare names are fine — `MQ2Main.h` pulls the using-directive
in first.

## 21. Launcher — `/aot` — 2026-07-27

One button per custom window, so none of them needs a remembered slash command. A narrow vertical
strip meant to sit under the stock **SC / EQ** buttons so it reads as part of that bar; draggable,
and the client persists its position, so it is placed once.

- **Files**: `core_aotmenu.cpp/.h` (own TU, no detours) + `EQUI_AoTMenuWnd.xml`, flag
  `areAoTMenuEnabled`. `/aot` and `/menu` from `core_bazaar.h`.
- **Buttons**: Spells, Autoskill, Adv Loot, Trader, Achievements, Search, Last Death.
- ⚠️ **Two different open mechanisms, and the split is deliberate.** Windows whose module exposes a
  show function are called directly (`SpellChoiceOpen`, `AutoSkillShow`, `AdvLootShow`,
  `ShowSearchWindow`, `LostWindowShow`). Windows whose CONTENTS come from the server are opened by
  **queueing their command** instead — Trader (`/trader` → `SHOPINVDATA`) and Achievements (`/ach` →
  `ACH|window|show`). Calling their internal show directly would put an EMPTY window on screen, and
  for the shop it would show the last stale listing.
- ✅ **It DOCKS to the left of the stock SC / EQ bar** on every open, so both are usable and neither
  covers the other. The bar is `EQMainWnd` and the client keeps a pointer at **`pinstCEQMainWnd`**
  (`eqgame.h` 0xF713C0), already in the address table -- so nothing is found by name and nothing stock
  is hooked. `CXWnd::GetScreenRect` (0x8638D0) and `CXWnd::Move(CXRect)` (**`CXWnd__Move1_x`**,
  0x8686F0) are both mapped on this build. ⚠️ `CXWnd__GetClientRect_x` is **NOT** defined here -- use
  GetScreenRect.
  ⚠️⚠️ **`CXRect::A,B,C,D` are left/top/right/bottom and are `DWORD` -- UNSIGNED.** Cast to `int`
  before any arithmetic: `bar.A - width` wraps to ~4 billion when the bar sits near the left edge,
  flinging the window off screen instead of clamping. If there is no room on the left it flips to the
  bar's right.
  ⚠️ Docking runs on OPEN, not per frame -- a per-frame reposition would fight the player dragging the
  window. Move the EQ bar, then reopen the launcher to re-dock.
- ⚠️⚠️ **DO NOT PUT BUTTONS INTO THE STOCK SC / EQ BAR. IT WAS TRIED AND IT CRASHES.** An `EQM_AoTButton`
  was added to `EQUI_EQMainWnd.xml` and its click caught by copying `EQMainWnd`'s vtable and
  overwriting **entry 34** (the slot `SetWndNotification` writes) -- necessary because
  `CCustomWnd::ReplacevfTable` only ever operates on `this`. **It crashed on the first click.** The
  code and the XML edit are both reverted; `EQUI_EQMainWnd.xml` is back to stock and is NOT shipped.
  Do not re-attempt: it patches a window this dll did not construct, on a build whose UI structs are
  already documented as not matching the headers (§13), and it can only fail at runtime -- there is no
  static check that would have caught it. The list is our OWN window, which gets notifications the
  ordinary way and cannot corrupt anything stock.
- **It auto-shows once per session** on entering the world (`AoTMenuTick` watching `pinstLocalPlayer`),
  then stays closed if closed. `AoTMenuOnUiReset` re-arms it so it survives a `/loadskin`.

### ⚠️⚠️ Listbox text colours are `0xAARRGGBB` — the ALPHA byte is mandatory (2026-07-28)
A colour written **`0x00RRGGBB` is fully transparent**, so the text is drawn and is completely
invisible. The Autoskill window showed an empty red selection bar and no skills at all for this
reason, and the spell Known/Pool tabs had the same fault. Use **`0xFF……`**. `core_advloot.cpp` and
`core_achievements_native.cpp` were already correct (`0xFFE0DCCD` etc.), which is why they looked fine
and made the others look like a data problem rather than a colour one.
⚠️ Second, separate trap in the same place: **`CListWnd::AddString` colours COLUMN 0 ONLY.** A cell
written afterwards with `SetItemText` has no colour and draws **black**. Every extra column needs an
explicit `SetItemColor(row, col, colour)` — the `Cell()` helpers in `core_autoskill.cpp` and
`core_spelljournal.cpp` now take a colour and do both.

### Blacklist as of 2026-07-29 — 637 pruned, **1996 offerable**
```
travel 170 · discipline 157 · summonitem 183 · illusion 23 · vision 19
enchant 20 · rez 15 · ldon 36 · curecurse 7 · corpse 3 · sense 3 · truenorth 1
```
⚠️ These counts are only true as of the last `gen_stock_pool.pl` run — **re-read them off the
generator's own output rather than trusting this block**, which drifts the moment any SQL touches
`spells_new`. The 2026-07-28 figures recorded here (619 pruned / 2008 offerable / `ldon 18`) were
already stale before the Sinew line was added: `ldon` had moved 18 → 36 on its own.
Pool is **2633** spells (2585 stock + 48 custom) over 78 levels before the blacklist is applied.
- ⚠️ **`discipline` MIRRORS `common/spdat.cpp IsDiscipline` EXACTLY** — `mana = 0 AND (EndurCost > 0 OR
  EndurUpkeep > 0)`. Do not simplify it to "has an endurance cost": that would misclassify anything
  carrying both, and the rule must agree with what the ENGINE calls a discipline or the picker and the
  grant path disagree.
  Disciplines are excluded because they are not spells and do not behave like one: a picked discipline
  trains into the **Combat Abilities** window, not the spellbook, and autoskill cannot fire it either —
  that handles only the ten activated specials in `Client::GetAvailableAutoSkills`. A reward that lands
  somewhere the player is not looking, driven by none of our systems, is not a reward. Throw Stone,
  Leg Strike, Head Strike and Provoke were all being offered this way.
- ⚠️ **Rule ORDER decides which bucket a spell is counted in.** Adding `discipline` moved 16 spells out
  of `summonitem` (199 → 183): they matched both. The counts are per-rule-first-match, not disjoint
  categories — a shifting count in one bucket after adding a rule is expected, not a regression.
- ⚠️ `truenorth`, `sense`, `illusion` and `vision` all test PURITY: pruned only when the junk effect is
  the ONLY thing the spell does. A spell that senses/disguises/points north AND does something real is
  kept. Any new rule of that kind gets the same treatment.

### ⚠️⚠️ Syntax-check Lua before restarting zones — `.devcontainer/custom/tools/luacheck`
The container ships **no `lua` or `luac` binary**, so for most of this project's life a broken quest
module could only be found by restarting zones and reading a player's CHAT log. That is how a mangled
string literal in `spell_choice.lua` took down the **entire** level-up picker: `global_player.lua`
`require`s it at line 4, so the failed module aborted the whole global player script, and the only
symptom was `error loading module 'spell_choice'` in game.

`luacheck.c` builds against the system Lua 5.1 and calls `luaL_loadfile`, which parses and compiles
but never RUNS the file — so quest scripts full of `eq.*` calls are safe to check.

```bash
cc -O1 -o .devcontainer/custom/tools/luacheck .devcontainer/custom/tools/luacheck.c \
   -I/usr/include/lua5.1 -llua5.1
find .devcontainer/repo/quests -name '*.lua' -print0 | xargs -0 .devcontainer/custom/tools/luacheck
```

⚠️⚠️ **DO NOT EDIT LUA WITH `perl -0pi -e`.** Both corruptions came from that: perl interpolates
`$"`, `$+` and friends inside the replacement, and a Lua pattern is *full* of them —
`"^sjinfo%s+(%d+)%s*$"` became `"^sjinfoH  PA u+(1990222272+)  +z * )`. It also silently inlined the
whole slurped file into `gen_stock_pool.pl` when a `$_` appeared in a replacement. Use the Edit tool,
or single-quoted `sed` with explicit line numbers, and run luacheck afterwards either way.

## 24. Scaling dungeon — "Delve" — `/delve` — 2026-07-29

Pick a level in a window, get dropped into a private **instance** of a Dragons of Norrath zone with
every mob scaled to that level and a real quest in the journal. Finish it and a chest drops where you
finished it; loot the chest or press Exit and the instance closes. **Pure Lua + SQL + one window —
no C++ server change at all**, because all four primitives are already native and Lua-bound:
`eq.create_instance` / `assign_to_instance_by_char_id` / `destroy_instance`, `client:MovePCInstance`,
`npc:ScaleNPC(level)`, and the task system for the journal.

- **Files**: `lua_modules/aotv4_dungeon.lua`, `custom/sql/aotv4_dungeon.sql`,
  `eq-core-dll/src/core_dungeon.cpp/.h`, `EQUI_AoTDungeonWnd.xml`. Hooks:
  `global_player` `event_say` + `event_task_complete` + `event_enter_zone` + `event_death` +
  `event_connect` + **`event_disconnect`**, `global_npc` `event_spawn` + `event_death_complete`.
  ⚠️ `event_disconnect` is **not** only a camp hook — see the trap below; it fires on every zone
  change, including the one that carries the player INTO the delve.
- **Protocol**: `DUNGDATA <unlocked>^level|name|cleared^…` out; `/say delve`,
  `/say delveenter <level>`, `/say delveexit` in. Command is `/delve` (alias `/dungeon`), plus a
  **Delve** button on the `/aot` launcher (§21; `EQUI_AoTMenuWnd.xml` grew 232 → 262 tall for it).
  Install: `aotv4_client_install/DUNGEON_WINDOW_INSTALL.md`.
- ⚠️ On the launcher, Delve sits in the **"open directly"** group, not the "queue the command" group,
  even though its contents come from the server — `DungeonShow()` opens the window *and* queues
  `/say delve` itself, so it is never shown empty. That distinction is the whole point of §21's split.
- Debug trace: `<EQ>\aotv4_dungeon.log`, which is what tells an unbuilt dll (no file) apart from a
  missing `EQUI.xml` `<Include>` (`pXWnd=NULL`) — otherwise identical symptoms.
- **Fifteen rungs, level 1 → 70** in fives, cycling the six DoN zones (`delvea` 341, `delveb` 342,
  `stillmoona` 338, `stillmoonb` 339, `thundercrest` 340, `thenest` 343), each at its own mission
  version. ⚠️ It **started at 50** originally, which meant a new character could not reach the first
  delve at all and clearing it jumped straight to 55 — the top of a ladder with no ladder under it.
- ⚠️ Six zones over fifteen rungs means a zone is **reused at several difficulties**, which is why the
  tasks are per **ZONE** (six of them) with **no level in the title** and `min_level 1`: a title of
  "[50]" would be wrong on 11 of the 15 rungs. The authoritative rung table is `M.LAYERS`.

- ⚠️⚠️ **DoN is the ONLY expansion past OoW with instanced content.** `dynamic_zone_templates` holds
  215 rows: Classic 6, LDoN 24 (14 zones), GoD 16 (9), OoW 28 (16), **DoN 141 (6 zones)** — and
  **zero** for DoDH through RoF. The 141 sit on 6 zones because each ships many *versions* with
  genuinely different populations (delvea 9, stillmoona 9, thenest 14…), i.e. ~59 ready-made layouts.
  Any zone can still be instanced by `create_instance`; the templates only say what stock content
  instances.
- ⚠️⚠️ **An EMPTY `npc_match_list` matches EVERY npc** — that is the mechanic, not an oversight.
  `task_client_state.cpp:528` wraps the whole name test in `if (!activity.npc_match_list.empty() &&
  …)`, so an empty list makes any kill in the listed zone count. `zones` holds the **base** zone id
  (instances keep it), so one task row serves every instance.
- ⚠️ **Layers unlock in order and only unlocked ones are ever SENT.** With loot out of scope the
  reward is XP, so a freely pickable level would be a power-level machine. The window cannot show or
  ask for a layer it was never told about, and `M.enter` re-checks anyway.
- ⚠️ **Gate order in `M.enter` is load-bearing**: validate → create instance → assign → task → move.
  Creating first leaks an instance on every refusal; moving before `AssignTask` puts the player in the
  dungeon with no journal entry, which reads as the quest having failed.
- ⚠️⚠️ **THE VERSION IS NEVER 0 (fixed 2026-07-29).** Version 0 is the **open world** spawn set, so
  instancing it produces a private copy of the ordinary zone — genuinely instanced (the zone boots
  with its own `inst_id`, verified in the log) but identical in layout and population to just walking
  in, which reads as "it opened the open world, not a DZ". The non-zero versions are the real DoN
  mission layouts. Entry coordinates come from **`dynamic_zone_templates.zone_in_*`** for that exact
  (zone, version) pair — the point DoN itself uses — **not** `zone.safe_x/y/z`, which can sit in a
  part of the map the mission layout does not populate. Mission versions are **smaller** (delvea 154
  points against version 0's 262) and the kill goals are set against them, not against version 0.
- ⚠️⚠️ **`eq.get_zone_id()` TAKES NO ARGUMENTS** — it returns the *current* zone. `eq.get_zone_id(name)`
  is a hard Lua error (`int get_zone_id()` + stack traceback) and it silently broke Exit, which had
  already printed its "you step back out" message before dying. Use `eq.get_zone_id_by_name()`, or
  better, record the numeric zone id on the way IN so no lookup is needed on the way out.
- ⚠️ **A FRESH instance every time.** `eq.get_instance_id` would hand back the one this character used
  last, still full of corpses and an emptied spawn table.
- ⚠️⚠️ **`e.other` IS A `Lua_Mob`, NOT A `Lua_Client`.** `IsClient()` returns true on it, but
  **`CharacterID` is only defined on `Lua_Client`**, so passing it straight into anything that keys a
  data bucket is *"attempt to call method 'CharacterID' (a nil value)"* — which fired on **every kill**
  in a delve. `CastToClient` is not reliably bound either; resolve through
  **`eq.get_entity_list():GetClientByID(mob:GetID())`** (what `aotv4_sinewtap.lua` already does).
  `M.as_client()` is the shared helper.
- ⚠️⚠️ **DoN zones have real zone lines out of them**, and walking one drops you into the ordinary
  world with the run bucket set, the task live and the instance open. `M.on_enter_zone` (from
  `global_player.event_enter_zone`) **fails the run** and returns you to where you entered from. ⚠️ The
  test is *"am I somewhere other than my delve"* (zone short name **and** instance id), not "did I
  zone" — the same hook fires on the way **in**. ⚠️ Unlike the death path this one **does** move the
  player, because the zone change has already taken them out of the instance.
- ⚠️ **The chest has to be findable.** It lands wherever the last blow happened — a corridor, a corner,
  behind a corpse pile — and a default chest on a dark DoN floor is easy to walk straight past. Four
  things stack: `texture 1` (gilded), `size 12`, `light 10` on the npc row, plus `AddNimbusEffect(412)`
  at runtime and a message. ⚠️ **412 is the only nimbus id proven to work in this codebase**
  (`guildhall/#Healer.lua`); a wrong nimbus id fails silently.
- ⚠️⚠️⚠️ **`EVENT_DISCONNECT` FIRES ON EVERY ZONE TRANSFER, NOT JUST CAMP / LINK-DEAD — and this
  single fact made the Delve un-enterable for an entire session (fixed 2026-07-30).**
  `zone/client_process.cpp` tests `bZoning` for other work in the very same function —
  `if (!bZoning) { SetDynamicZoneMemberStatus(Offline); }` (**:735**) — and then fires the event
  **unconditionally** thirty lines later (**:765**). So the instant `M.enter` moved a player into
  their delve, the source zone raised `event_disconnect`, `M.on_disconnect` concluded the run was
  over, and it deleted the run bucket, the back bucket and the `instance_list_player` row **while the
  player was still on the loading screen**. World then failed its own entry check (next bullet) and
  redirected them out. Guarded by a **transit flag** (`delve_transit_<charid>`): set immediately
  before the move, cleared on arrival *and* on landing anywhere else, expiring after 60 s so a real
  camp mid-transit still tears down on next login. `Client::IsZoning()` exists (`zone/client.h:855`)
  but is **NOT Lua-bound**, hence the flag.
  - ⚠️⚠️ **The tell is which teardown path DIDN'T run.** `on_disconnect` is the only one that
    deliberately leaves the journal entry alone, so the fingerprint was buckets + membership row
    vanishing while the **task survived**. Polling the DB five times a second is what found it:
    `.934` everything correct → `.368` buckets and ILP row gone → `.029` evicted. **Reach for a
    DB poller early on anything that fails this silently** — three separate "fixes" were aimed at
    downstream symptoms before anything was actually measured.
  - 📌 Any future handler that tears state down on `event_disconnect` needs the same guard.
- ⚠️⚠️ **A CHARACTER MUST BE REGISTERED ON AN INSTANCE OR WORLD SILENTLY REDIRECTS THEM.**
  `world/client.cpp:929` runs `VerifyInstanceAlive` → `CheckInstanceByCharID` (an
  `instance_list_player` row); on failure it calls `MoveCharacterToInstanceSafeReturn` and zeroes
  the instance. Symptom is **"it opens the delve and then ports me to the bazaar"** — no error, no
  log line, and nothing wrong in the zone that booted. `eq.create_instance` creates the instance but
  **adds nobody to it**; `eq.assign_to_instance_by_char_id(instance_id, character_id)` is what
  registers them (⚠️ that argument order, not the reverse). The stock `#zoneinstance` does exactly
  this before its move (`zone/gm_commands/zone_instance.cpp`), which is why the GM command worked
  when `/delve` did not — the cheapest way to bisect this class of bug.
  - 📌 `Client::SendToInstance` (`zone/client.cpp:11003`, Lua-bound, wrapped by
    `quests/plugins/Instances.pl`) is the native one-call form: `AssignToInstance` + `MovePC`. It is
    exactly the two calls above, so it is a tidier spelling rather than a different mechanism — but
    it caches the instance id in a data bucket keyed by type/name/identifier/zone, so it **reuses**
    an instance unless the identifier is varied per run.
- ⚠️⚠️ **DO NOT CREATE AN EXPEDITION AND ZONE INTO IT IN THE SAME BREATH — world reaps it first.**
  `world/dynamic_zone.cpp:104`: `if (!HasMembers() || IsExpired())` and the dz zoneserver has
  `NumPlayers() == 0`, the DZ is `ExpiredEmpty` and **deleted**, taking its `instance_list_player`
  rows with it. Creating and moving together tears the source-zone `Client` down mid-handshake,
  before world's copy of the DZ has its member list, so it looks memberless — measured at ~270 ms
  from create to delete. Expeditions themselves are **fine** on this build: AoTv3's Progressive
  Dungeons uses them (`server/quests/draniksscar/302093.lua`, repo `Twochords/AoTv3`) but creates the
  expedition and then **stops**, telling the player *"Click the dungeon portal to enter your active
  mission"* — entry is a separate act through the engine's own DZ portal, exactly as live EQ does it.
  The Delve therefore uses a **plain instance**; the cost is no Ctrl+Z entry, no compass and no
  engine-managed safe return. Revisit only by decoupling entry from creation (portal / confirm step),
  never by adding a delay.
- ⚠️⚠️ **CAMPING (`/q`) IS THE THIRD WAY OUT** and needs its own handling: the character is **saved
  standing inside the instance**, so if that instance is later gone the next login asks world to place
  them somewhere that no longer exists. The engine falls back gracefully (the world log shows
  `delvea(105)` → `tutorialb`), but every camp leaves an **orphaned instance** and a run bucket
  claiming the player is in a dungeon they left. `M.on_disconnect` (`event_disconnect`, which this
  server had **no** handler for at all) ends the run; ⚠️ it deliberately does **not** move the player
  or touch the task — the client is already leaving. `M.on_connect` drops the stale journal entry on
  the way back in, guarded on there being no live run so a camp-and-return mid-delve is not disrupted.
- ⚠️⚠️ **`DungeonOnUiReset` must `delete` the window, not just null the pointer.** `/q` tears the UI
  down through `CleanGameUI`; nulling alone leaks the `CCustomWnd` **and leaves it registered with the
  client's window manager** after its widgets are freed, so the next thing to walk the window list
  touches freed memory. Every other native window here deletes (`core_lostwindow`, `core_autoskill`,
  `core_advloot`, `core_allaclone`) — the delve window was the odd one out.
- ⚠️ **Dying ends the run** (`M.on_death`, from `global_player.event_death`): the roguelite reset puts
  the character at **level 1**, so the mission cannot continue. It uses **`FailTask`**, not
  `RemoveTaskByTaskID` — a death is a real failure and should read as one, where walking out
  voluntarily should not. ⚠️ It is the **one teardown path that does NOT move the player**: the engine
  is already sending them to their bind point as part of the death flow, and a competing `MovePC` from
  inside `event_death` risks breaking the respawn. Drop the task *before* destroying the instance.
- ⚠️ **Destroy the instance LAST**, after the player is out, or you strand a client in a zone that no
  longer exists. `M.leave` is the single teardown path for both the chest and the Exit button.
- ⚠️ **`AssignTask(id, 0, false)`** — `enforce_level_requirement` is false on purpose. The tasks carry
  DoN-style `min_level` tiers so they *read* native, but the unlock chain is the real gate.
- ⚠️ `RemoveTaskByTaskID` (custom binding, `lua_client.cpp`) is used on exit, not `CancelAllTasks`: it
  clears the DB rows too, so an abandoned delve does not leave a permanent journal entry.
- ⚠️ **`ScaleNPC`'s Lua binding passes `always_scale = true`** (`lua_npc.cpp:782`), so it scales mobs
  not flagged for scaling — every stock DoN mob. No `npc_types` editing needed. The chest is skipped
  explicitly or it would get 70th-level HP and become a fight.
- ⚠️ `M.on_npc_spawn` runs for **every NPC spawn on the server** — the instance-id early-out rejects
  the whole normal world before anything else is read.
- ⚠️ **`AddLooter` is on `Lua_Corpse`, not `Lua_NPC`**, so it cannot be applied to the living chest.
  Loot is out of scope for now, so the chest is an empty marker whose death ends the run.
- ⚠️ There is **no `MoveToSafeCoords`** binding on `Lua_Client` (only `MovePC` and the `MoveZone`
  family) — the bind point is the fallback exit.
- ⚠️ npc_types columns were hand-listed on a first pass and `aggro_spell_id` does not exist on this
  schema. The chest is cloned from stock 119179 `a_gilded_chest` **via a temp table**, same rule as
  the custom spell lines.
- 📌 Not built yet: loot in the chest, group entry (`assign_to_instance` is per character today),
  a floor/ascent chain, and any Torghast-style in-run power picker.

### Player-relative creature scaling — `aotv4_dungeon_scale.lua` — 2026-07-29
The layer level is only the **floor**. Creatures also scale by how far the player sits above the
**naked expectation** for their level, which is the gear-bloat correction. `/say delvepower` prints
the whole calculation axis by axis.

- **Why it can be measured at all**: in EQ, stats / AC / resists **do not grow with level** on their
  own — a naked level 70 has the same STR as a naked level 1, because all of it is gear. Only HP and
  mana scale with level. So gear bloat is directly "actual over naked expectation", and only the
  HP/mana axis needs a per-level curve.
- **Baseline** = the level 1 reference character (Warrior/Karana): HP 38, MP 40, AC **31**,
  stats sum **585**, resists sum **130**. ⚠️ Stats and resists are race/class dependent, so one
  universal baseline is an approximation — the axis that matters most (HP) comes from `base_data`.
- ⚠️⚠️ **`base_data` gives the exact per-level curve, so do not guess it.** `hp` for any class is
  `CLASS_HP[class] × shape(level)`, and the shape is **identical for all 16 classes** (warrior/magician
  HP is a constant 1.25 ratio at every level). The shape is **linear to 40 then double rate**:
  `shape(L) = L` for L ≤ 40, else `40 + 2×(L−40)`, hitting exactly 100 at level 70. Treating it as
  linear throughout understates level 70 by 30% (70 vs 100) and makes every high-level character look
  geared while naked.
- **Weights** hp .30 / mana .15 / ac .20 / stats .20 / resists .15, then `effective =
  layer + 12×(power−1)`, clamped to −5 / +20. The reference character scores **power ≈ 1.05**, i.e.
  baseline difficulty.
- ⚠️ **Renormalise by the weight actually used.** A Warrior has no `base_data` mana, so that axis is
  skipped; dividing by the full weight total would score every melee class 15% weaker than it is and
  quietly soften their delves. (Melee mana here is a flat `level × 40` from §14 and carries no gear
  signal at all, so skipping it is also correct on the merits.)
- ⚠️ The HP expectation uses the player's **current STA**, so stamina-granted HP lands in the stats
  axis instead of being counted twice.
- ⚠️ **`ScaleNPC` must come before any `ModifyNPCStat`** — it rewrites stats wholesale from
  `npc_scale_global_base`, discarding anything applied first. `ModifyNPCStat` takes **strings** and
  accepts `max_hp`, `min_hit`, `max_hit`, `ac`, `atk`, `accuracy`, `avoidance`, the resists and `level`.
- ⚠️⚠️ **Re-scaling mid-run is OUT-OF-COMBAT ONLY** (`M.rescale_zone`, a 10s per-client timer). This is
  what catches entering stripped and then gearing up. Re-scaling something you are *fighting* would
  refill it to a new max_hp mid-swing, and re-scaling **down** would be the exploit in the other
  direction (strip, soften the boss, re-gear). A pulled mob keeps the difficulty it was pulled at.
  ⚠️ A `RESCALE_DELTA` deadband (0.08) stops one buff landing or fading churning the whole zone.
  ⚠️ The tick guards on **zone and instance id**, not just the run bucket — that bucket survives a
  crash or GM port, and without the guard the sweep would rescale mobs in an ordinary zone.

#### ⚠️⚠️ Rewards must never be computed from gear at the END of a run
The ledger records **the effective level of every mob actually killed, at the moment it died**
(stamped on the NPC via `SetEntityVariable("delve_eff")` when it was scaled). Clearing the dungeon
naked and putting gear on before turning it in therefore *cannot* inflate anything — the number was
already banked. That makes the whole class of gear-swap exploits **structurally impossible rather
than merely detected**, which is why the ledger keys off the mob and not off the player.
- `M.ledger` returns `kills`, `avg`, `lo`, `hi`. **`avg` is what pays.** `hi − lo` is the *swing* — a
  naked clear followed by a gear-up reads as a low average with a high maximum, and the player is told
  as much on completion.
- ⚠️ The ledger is wiped in `M.enter` **before** entry; a stale one would credit this run with the
  last run's kills, which is exactly the accounting an exploiter wants.
- 📌 The scale reference is the **first client found** in the instance — fine for solo, a known
  simplification for groups (it should become the strongest member, or an average).

#### The score sheet — per-kill value + run history
- **`M.kill_value(eff, named)` is QUADRATIC**: `eff² / 25`, ×3 for a named. Level 25 → 25, 50 → 100,
  70 → 196. ⚠️ Linear scoring would make grinding a soft layer for volume beat clearing a hard one —
  the exact behaviour the scaling system exists to discourage. "Named" uses the same heuristic as
  §17c and `quest_difficulty.pl`: EQ trash is lowercase, named mobs are proper nouns (or lead with `#`).
- Each run also builds a **five-level band histogram** (`45-12,50-20,55-15`), so a run reads as a
  *shape* rather than one average. ⚠️ Bands use `-` internally because `|` and `^` are the wire
  separators and `,` separates the bands.
- **History** in `delve_hist_<charid>`, newest last, **capped at 20** with the oldest dropped — a data
  bucket is one string, and §20 records that an oversized chat line is silently **truncated**, which
  looks like a short list rather than an error. Outcome letters: `C` cleared, `F` died, `A` abandoned.
- ⚠️ Recorded in **three separate places** (clear / death / abandon), not one shared helper, because
  the outcome letter is what makes the row meaningful. ⚠️ In `M.leave` the "still active?" test must be
  taken **before** `RemoveTaskByTaskID` — after it, it always reads false and no abandon is ever
  recorded; and looting the chest also routes through `M.leave`, so without the test every clear would
  be double-counted.
- ⚠️ `M.send_history` resolves the layer **name** server side rather than sending an index: the client
  only ever learns the names of layers it has *unlocked*, so an old run in a now-invisible layer would
  render blank. It is also sorted newest-first server side rather than in the dll.
- **Client tab** = the Death Book pattern (`core_lostwindow.cpp`), and all three of its traps apply
  identically: the toggle is **deferred to `DungeonTick`** (`RefreshHistory` → `DeleteAll` would
  destroy the listbox's rows from inside its own click notification), the selection is read **after**
  `CSidlScreenWnd::WndNotification` (inside it `GetCurSel` is one click behind), and the row index is
  **not** a data index — every click goes through `g_histRowRun[]`.
- ⚠️ `DungeonHistTransport` **preserves which runs were open** across a rebuild, keyed on the run's
  timestamp. The server pushes fresh history after every finished run, and collapsing the sheet each
  time would fight a player reading it. Newest run defaults to open.
- ⚠️ Both server lines route through the single `DungeonParseTransport` entry point, so the `dsp_chat`
  chain needs one call, not two.

## 23. Quest difficulty scoring — `quest_difficulty.pl` — 2026-07-29

`.devcontainer/custom/tools/quest_difficulty.pl` scores every quest on the server as ONE float.
Read-only (database + quest scripts); no server restart, nothing generated into the game.

```
score = LEVEL BAND . AT-LEVEL DIFFICULTY
        floor(1 + level/10), clamped 1..8      0.00 .. 0.99
```
A level 30 quest bands at **4**, a level 60 quest at **7**, so sorting by score sorts by level first
— level being the dominant input. The decimal is how hard it is **assuming you are the right level**.

- **Terms** (weights): turn-ins **0.35** (distinct items 0.75 / extra hand-in steps 0.25), travel
  **0.35**, gear **0.30** (required content tier 0.70 / turn-in item quality 0.30). Each saturates
  via `sat(x, full)` so no single outlier can push a quest out of its band; the decimal is hard-capped
  at 0.99 for the same reason.
- ⚠️ **The gear term is LEVEL-RELATIVE, deliberately** — `over_level()` scores how far the required
  content sits *above* the quest's own level (at-level = 0, +15 levels = 1). Scoring it absolutely
  made every level 60 quest look hard purely for being level 60, which the integer part already says.
  Turn-ins and travel stay absolute: ten hand-ins is ten hand-ins at any level.
- **Both corpora, one model**: `SCRIPT` = 2,743 rows parsed from `quests/<zone>/<NPC>.lua|.pl`
  (`check_turn_in{item1..}` / `check_handin(…, id => n)`); `TASK` = `tasks` + `task_activities`
  (activitytype **1 Deliver / 2 Kill / 3 Loot**, `common/tasks.h`). Only extraction differs.
- Flags: `--zone= --source=script|task --limit= --min= --explain --tsv`.
- Current spread: bands 1→8 hold 351/111/134/312/366/470/785/214, mean decimal **0.313**.

### ⚠️⚠️ Three modelling traps, all of which produced plausible-looking but wrong numbers
1. **Travel is the number of zones you must GO to, not the union of everywhere an item exists.** A
   common drop comes from forty zones; you visit **one**. Unioning them reported quests spanning
   **185-217 zones**. Each item now costs one trip, to the cheapest source available, preferring
   ground spawn → merchant → loot (picking something up beats having to kill for it), and free if the
   quest's own zone can supply it.
2. **A zone that DEMANDS an item does not SUPPLY it.** The scanner registered every turn-in item as
   available in the very zone requiring it, so "can I get this without leaving?" was always true and
   script travel was **exactly zero on all 2,743 rows** — while tasks looked fine, which is what made
   it read like a data gap rather than a bug. Providers are summonitem/loot/ground/merchant **only**.
3. **MIN dropper level, not MAX.** A player farms the *easiest* source. MAX let one high-level
   dropper somewhere in the world pin the gear term at its ceiling — every quest scored exactly
   0.700 — which looks like a constant, not a bug.
- ⚠️ Also: match summonitem **on the actual call**. An earlier regex matched `item_id`/`itemid`
  anywhere in a file and swept up merchant tables and config.
- 📌 **Unit of "a quest" is one NPC FILE**, so an NPC implementing several independent quests reads as
  one many-step quest (`abysmal/Rilwind_Sitnai` shows 19 steps). The steps term saturates at 4 extra
  to blunt this. Real chains spanning several NPCs are only partly captured — via the 1,604 of 5,592
  turn-in ids that some other script summons.
- 📌 Coverage is honest, not total: 5,589/5,592 turn-in ids resolve to real items, but only 57% have
  any loot source, so gear scores 0 where nothing is derivable rather than guessing.

## 22. Combat specials COST ENDURANCE — 2026-07-29

Stock EQEmu charges nothing to Bash/Kick/Backstab/the monk strikes — the only brake is the reuse
timer. Here a damaging special costs endurance **in proportion to the damage it actually dealt**, so
the specials are a resource to spend rather than a rotation to mash. Pure C++, no Lua, no client
change. **Needs a zone rebuild** (and `ruletypes.h` is in `common/`, so expect a wide one).

- **Files**: `common/ruletypes.h` (two rules) + `zone/special_attacks.cpp`
  (`Mob::DoSpecialAttackDamage`, the gate at ~:235 and the charge at ~:333).
- **Rules**: `AoT:SpecialEndurancePct` (**50** — a 200-damage Backstab costs 100 endurance; `0`
  disables the whole mechanic, gate included) and `AoT:SpecialEnduranceMinToUse` (**1**).
- ⚠️⚠️ **`DoSpecialAttackDamage` IS THE ONLY PLACE THIS GOES.** Bash, Kick, Frenzy, Backstab and
  every monk strike (via `MonkSpecialAttack`) funnel through it, so one edit covers the
  player-pressed path **and** the autoskill auto-fire loop (§19). Per-ability copies would be five
  that drift — the same reasoning that put the skill-gating in one place in §4.
- ⚠️⚠️ **CHARGED AFTER THE HIT, and it has to be.** The cost is a share of the damage and the damage
  is not known until `DoAttack` has rolled it. That is *why* the pre-swing test is a cheap "have you
  got anything left at all" (`SpecialEnduranceMinToUse`) rather than a real affordability check —
  **you cannot price something before you know what it costs.** Don't "fix" the gate into a proper
  cost check; it can't be one.
- ⚠️⚠️ **The reuse timer STILL STARTS on a winded whiff.** The caller begins it after the function
  returns (`p_timers.Start`, e.g. `special_attacks.cpp:490` for Bash) across six-plus call sites, so
  gating there would be six copies. Deliberate as well as convenient: spending the swing is what
  stops a player mashing specials at zero endurance, which is the entire point of the cost. Setting
  `SpecialEnduranceMinToUse` to 0 removes the gate — and with it any reason not to mash.
- ⚠️ **Clients only.** `Mob::GetEndurance` is a virtual returning **0** (`zone/mob.h:674`) and
  `Mob::SetEndurance` a **no-op** (`:677`); only `Client` overrides them (`zone/client.h:751/756`).
  An NPC would silently "pay" nothing, so the `IsClient()` guard is there to stop a later reader
  believing NPCs are costed.
- ⚠️ **A miss costs nothing by construction** — 50 percent of zero damage is zero. No special case.
- ⚠️ Multiply before dividing (`damage * pct / 100`) or a small hit truncates to a free swing. Same
  trap as the Shield Wall split (§17b).
- 📌 Untuned: 50 percent is a guess until it is played. It interacts with the §19 autoskill cap —
  the cap limits how many specials may fire, this limits how long they keep firing.
- **The refill is the Sinew line** (§5, `custom/sql/aotv4_sinew_line.sql`, ids 43318-43323): a damage
  tap returning endurance equal to a third of its damage. This cost is what gave endurance a sink
  worth spending on in the first place, so tune the two together — at the default 50 percent, the
  top tier's 175 endurance buys roughly 350 damage worth of specials.

## 25. ⚠️⚠️ THE STALE-DATABASE TRAP — "a session's work vanished" — 2026-07-24, 2026-07-30

**The single most expensive recurring failure in this project.** It has struck at least twice and
cost most of a session both times. Nothing errors, nothing logs, and the server runs perfectly —
you are just connected to an **older copy of `peq`** than the one your last session's work went
into. Because every symptom points at "the data was lost", the natural reaction is to re-apply SQL
or import a snapshot, and **that is what actually destroys the work** — the real database is still
sitting on disk somewhere the whole time.

**Run `bash .devcontainer/custom/tools/db_sanity.sh` BEFORE starting the stack or applying any SQL.**
It exits 1 and prints why. On 2026-07-30 it reported the datadir as **138 hours older** than the
shared-memory blobs — the diagnosis that had just taken an hour of manual work.

### What it looks like
`git status` is clean and correct, every source file is where you left it, the server boots, and
the DB answers every query — but recent content is simply **absent**. In 2026-07-30's case: the
Sinew line, the Delver's Sigil, all delve tasks and the chest NPC were missing, while the class
auras and achievements (older work) were present, which made it read like a partial failure of one
feature rather than a whole-database problem.

### Why it happens — ⚠️⚠️ TWO DOCKER ENGINES, TWO VOLUMES, ONE NAME
**This is the confirmed 2026-07-30 cause, and it is not what it looks like.** Nothing was on a
container's writable layer, no mount failed, and no data was ever lost. There are **two Docker
engines**, each with its own `/var/lib/docker`, and each holding a volume named `aotv4-mysql-data`.

⚠️⚠️ **THE TWO ENGINES ARE *NOT* THE TWO `docker context ls` ENTRIES.** That was the first guess and
it is wrong: Docker Desktop publishes **both** `default` (`npipe:////./pipe/docker_engine`) and
`desktop-linux` (`npipe:////./pipe/dockerDesktopLinuxEngine`), and they reach the **same** daemon.
Proof — the dev container is invisible to both:
```powershell
docker --context default      stop <dev container id>   # No such container
docker --context desktop-linux stop <dev container id>  # No such container
```
⚠️⚠️ **THE SECOND ENGINE IS `docker-ce` RUNNING INSIDE THE WSL `Ubuntu` DISTRO, AND VS CODE USES IT
EVEN THOUGH VS CODE IS LAUNCHED FROM WINDOWS.** Confirmed from the Dev Containers log
(`Ctrl+Shift+P` → *Dev Containers: Show Container Log*), which is the fastest way to settle this:
```
Running Dev Containers CLI: read-configuration --workspace-folder /mnt/c/AoTv3/AoTv4
Start: Run in Host: /home/michael/.vscode-remote-containers/bin/.../node
docker version --> {"Client":{"Version":"29.4.0","Context":"default"},"Server":null}
Cannot connect to the Docker daemon at unix:///var/run/docker.sock
```
`/mnt/c/...` and `/home/<user>/` are **WSL** paths: the extension runs its CLI inside Ubuntu and
therefore uses **Ubuntu's** `docker-ce` daemon on `unix:///var/run/docker.sock` — a different engine
from Docker Desktop, with its own volumes. **"I always open VS Code from Windows" does not rule this
out**, which is exactly why it took so long to find.
- 📌 The same log shows VS Code helpfully launching `Docker Desktop.exe` when the daemon is
  unreachable (*"backend already running, signaling show-dashboard"*). That **does not help** — the
  CLI is still pointed at Ubuntu's socket, so starting Docker Desktop fixes nothing and makes it look
  like a Docker Desktop problem when it is not.
- ⚠️ A container **keeps running after its dockerd dies**, so the dev container stays usable while
  every `docker exec` from the host fails. Symptom in the log: repeated
  `Port forwarding ... Cannot connect to the Docker daemon`.

**THE FIX — give WSL one engine.** Enable Docker Desktop → *Settings → Resources → WSL Integration →
Ubuntu* (this is the step that matters: it makes `docker` inside Ubuntu resolve to Docker Desktop),
**and** disable Ubuntu's own daemon so it can never shadow it again:
```powershell
wsl -d Ubuntu -u root -- systemctl disable --now docker   # or: service docker stop
wsl -d Ubuntu -- docker ps -a                             # must now list Docker Desktop's containers
```
⚠️ **`wsl -u root` is the way in — this WSL user has NO sudo password**, so anything written as
`sudo …` simply prompts forever. If systemd is not enabled in the distro, `systemctl` errors with
*"System has not been booted with systemd"*; use `service docker stop` + `update-rc.d docker disable`.
Until one of these is done this recurs at random, because it depends on which daemon happened to be
alive when the workspace was opened.

Establish which engine you are on from **inside** the container, which always works:
```bash
hostname                                             # the dev container's own id
grep ' /var/lib/mysql ' /proc/self/mountinfo         # -> /var/lib/docker/volumes/<name>/_data
```
⚠️⚠️ **A NAMED VOLUME IS SCOPED TO ITS ENGINE, SO `aotv4-mysql-data` EXISTS TWICE** — two entirely
separate volumes that share a name. Both containers mount `aotv4-mysql-data` at `/var/lib/mysql`
exactly as `devcontainer.json` says; they simply resolve to **different disks**:

| engine | volume contents |
|---|---|
| `desktop-linux` | the **live** database — 930 MB, `character_data.ibd` dated Jul 30 17:38 |
| `default` | the **Jul 24 import**, written once and untouched since |

So **whichever engine VS Code happens to build the dev container on decides which database you
get**, silently. That is the "drift every once in a while" — it is not random, it is whichever
engine won that day. Verify with:
```powershell
docker inspect <ID> --format "{{range .Mounts}}{{.Type}} {{.Name}} -> {{.Destination}}{{end}}"
```
⚠️ **The fix is to pin every dev container to ONE engine** (`docker context use desktop-linux`, or
VS Code's `dev.containers.dockerPath` / Docker extension context setting). It is *not* a data
migration — the good data is already on a persistent volume and already survives rebuilds **on its
own engine**.
⚠️⚠️ **When deleting the stale copy, name the engine explicitly**:
`docker --context default volume rm aotv4-mysql-data`. The Docker Desktop UI only ever lists
`desktop-linux`'s volumes, so the two are indistinguishable by name there — and deleting the wrong
one causes exactly the loss this section exists to prevent.
- 📌 A **different** mechanism produced the same symptom on 2026-07-24 (§17): a container created
  *before* the volume mount was added kept `/var/lib/mysql` on its own writable layer, so the
  freshly-mounted volume came up empty. Both causes present identically — "the database is old" —
  so diagnose by evidence (below) rather than by assuming either one.

⚠️⚠️ **DOCKER DESKTOP SHOWS ONLY ITS OWN ENGINE'S CONTAINERS AND VOLUMES.**
On 2026-07-30 the dev container's own id was not known to the daemon Docker Desktop was talking to:
```
docker port 1a8ad2c4c550   ->   Error response from daemon: No such container
```
The container list therefore looked *incomplete* when it was in fact complete **for that daemon**.
Do not conclude "there is only one container" from the Docker Desktop UI. Confirm the id you are
running in (`hostname` inside the container) against `docker ps -a`; if it is absent, you are
looking at a different engine/context than the one hosting your shell.
- ⚠️ Two containers running at once also **split the host port bindings**. `d0efd1c563ed` published
  only part of 7000-7029 (missing 7003, 7006-7009, 7018, 7023, 7028) plus no 3307 and no 9001,
  because the other container had grabbed them first — so §0's "zone unavailable" bug was still
  live on a container that looked correctly configured. Port publishing is fixed **at container
  start**: freeing a port later does nothing until that container is restarted.

### How to detect it — the `/src` artifacts are the evidence
⚠️ **`/src` is a BIND MOUNT and survives every container**, so anything on it that was *generated
from* the database is datable proof of what the DB held at that moment. That asymmetry is the whole
trick, and it is what `db_sanity.sh` automates:

| artifact | written by | what it proves |
|---|---|---|
| `build/bin/shared/items`, `shared/spells` | `./shared_memory` | the item/spell tables at that timestamp |
| `build/bin/logs/**` | the running server | the stack was alive, and against which content |
| `build/bin/logs/zone/<zone>_..._inst_id_N_*.log` | a booted zone | the feature genuinely worked then |

If any of these is **hours newer** than the newest file in `/var/lib/mysql/peq`, the database in
front of you did not produce them. On 2026-07-30 `shared/items` contained all ten Delver's Sigils
and `shared/spells` contained `Sinewbite`, while the live DB contained neither — conclusive.
- ⚠️ **`sudo` is mandatory when inspecting the datadir.** The per-database dirs are `drwx------
  mysql`, so an unprivileged `find`/`du` returns nothing and reports **"no data" on a machine
  holding the full database**. Same trap as `docker exec -u root` (§17); it caught `db_sanity.sh`
  itself on its first run.
- ⚠️ **Check content markers by NAME, not only by id range.** An id-range count cannot tell "never
  created" apart from "renumbered". Grep a dump for an apostrophised name with care, too:
  `Delver's Sigil` is written `Delver\'s Sigil` in a mysqldump, so a literal grep finds **zero** and
  looks like proof of absence.

### Recovery — never import a snapshot first
> ⚠️ **First ask whether recovery is needed at all.** If the cause is the engine split above, the
> real database is sitting safe on the other engine's volume and the answer is to *point at it*, not
> to dump and import. On 2026-07-30 a full dump was taken (`peq_live_recovered.sql.gz`, 242 tables,
> verified) before this was understood — harmless as a belt-and-braces backup, but the actual fix was
> to build the dev container on `desktop-linux`. Below is for the case where the data really is
> stranded on a container you are about to discard.
The order that works, and the one CLAUDE.md §17 and `POST_REBUILD_RECOVERY.md` §STEP 0 both insist
on. Every command runs on the **Windows host**; `-u root` throughout.
```powershell
docker ps -a --format "{{.ID}}  {{.CreatedAt}}  {{.Status}}  {{.Names}}"
docker exec -u root <ID> sh -c "hostname; ls -l /var/lib/mysql/peq/character_data.ibd; du -sh /var/lib/mysql/peq"
```
`character_data.ibd`'s date **is the answer** — it is the newest-dated file that always exists.
Then dump straight onto the bind mount, where the dev container can verify it without any port
juggling or `docker cp` of a 750 MB datadir:
```powershell
docker exec -u root <ID> sh -c "service mariadb start; sleep 8; mysqldump --single-transaction --routines --events --databases peq | gzip > /src/peq_live_recovered.sql.gz"
```
⚠️ **Verify before importing and before deleting anything**: `gzip -t`, a `^CREATE TABLE` count
(**242** as of 2026-07-30; it was 233 pre-delve), a `Dump completed on` tail marker, and a spot
check for the newest feature you remember building.
⚠️ **Keep the source container STOPPED, not removed**, until the import is verified — until then its
layer is the only live copy.

### Prevention
- Run `db_sanity.sh` at the start of every session. Add a marker row to it whenever a datable
  feature lands; it is only as good as its markers.
- ⚠️⚠️ **NEVER OPEN OR REBUILD THE DEV CONTAINER WHILE DOCKER DESKTOP IS DOWN OR STILL STARTING.**
  Wait until it is fully up, then open VS Code. This is the suspected trigger for the whole
  2026-07-30 incident: Docker Desktop was failing to load, and a dev container was created at 19:08
  in the middle of that outage. With its engine unreachable, VS Code cannot see the workspace's
  EXISTING container (`d0efd1c563ed`, holding the live DB) and ends up building a fresh one
  somewhere else — which is how a second container appeared on an engine Docker Desktop cannot see,
  on a machine where the project is only ever opened from Windows. Correlation, not proof, but the
  cost of waiting two minutes is nil and the cost of getting it wrong was most of a session.
  📌 The same outage also *hides* the problem: with Docker down, `docker ps -a` cannot be run, so the
  two-container split has to be inferred from datadir mtimes and `/src` artifacts (below) instead of
  simply being read off a list. That inference took about an hour.
- ⚠️⚠️ **Build every dev container on the SAME engine — Docker Desktop's.** This is the whole fix
  for the 2026-07-30 incident. The data is already on a persistent volume and already survives
  rebuilds *on its own engine*; the only thing that goes wrong is being built on the other one.
  Confirm with `hostname` inside the container against `docker ps -a` on the host — **if your
  container's id is not in that list, you are on the wrong engine** and the database in front of you
  is not the one you were working on.
- **Get the database onto the named volume and keep it there.** A volume mount only becomes real for
  a container created *after* the mount exists, so after any recovery the DB must be imported into a
  container that actually mounts `aotv4-mysql-data` (the 2026-07-24 mechanism, §17).
- Prefer **one** container. Two split the host port bindings — on 2026-07-30 the second one silently
  stole 7003, 7006-7009, 7018, 7023, 7028, 3307 and 9001 — and make "which DB am I on?" ambiguous.
- ⚠️ **Never `docker system prune`, "Reset to factory defaults", or "Clean / Purge data"** while
  diagnosing this. Those delete container layers and volumes and are the only way to turn a
  recoverable access problem into real data loss.

## 26. Delve rewards — the Delver's Sigil and the Delve augments — 2026-07-31

Two reward lines come out of the delve (§24), and they work by **completely different mechanisms** —
which is the single most important thing to know before touching either:

| | Delver's Sigil (147500-147509) | Delve augments (147600-147915) |
|---|---|---|
| what it is | a native **evolving item** | **ordinary items**, `evoitem = 0` |
| how it grows | delve **score** | **combining 4 of a tier** in the Refining Crucible |
| where progress shows | the client's native evolving window | nowhere; the item simply changes |

- **Files**: `custom/sql/aotv4_delve_charm.sql`, `custom/sql/aotv4_delve_augs.sql` (**generated** by
  `custom/tools/gen_delve_augs.pl`), `custom/sql/aotv4_delve_remove_native_augs.sql`;
  `zone/aotv4_tiers.h` (tier id blocks), `zone/tradeskills.cpp` (`AoTv4RefineCombine`),
  `zone/lua_client.cpp` (`AddEvolveProgress`); `lua_modules/aotv4_dungeon.lua`
  (`stock_chest` / `roll_aug_tier` / `award_charm` / `on_close_timer`), `aotv4_dungeon_scale.lua`.

### ⚠️⚠️ WHY THE AUGMENTS ARE NOT EVOLVING ITEMS — do not "restore" `evoitem = 1`
They were, for one day, and it does not work: **an augment can never appear in the client's
evolving-item window.** That list is enumerated CLIENT side inside `eqgame.exe` — there is **no
enumeration packet anywhere**, the only server→client evolve message is `OP_EvolveItem UPDATE_ITEMS`
keyed by a unique id the client must already know — and its walk does not descend into augment
sockets. So the sigil showed natively while augments needed a second, custom window, and the design
goal was ONE evolve window. Combining removes the problem rather than working around it.
Three further reasons it was a bad fit, all still true and all invisible from the evolving code:
1. **They never accrue.** `Client::ProcessEvolvingItem` walks `GetInv().GetWorn()` — top-level worn
   items. An augment lives *inside* one and is never visited.
2. **They are never registered.** `EvolvingItemsManager::DoLootChecks` is the only thing that inserts
   a `character_evolving_items` row and is gated on `EQUIPMENT_BEGIN..EQUIPMENT_END`
   (`common/evolving_items.cpp:80`).
3. **Their in-memory evolve state is never loaded.** `SharedDatabase::GetInventory` rebuilds each
   augment with `PutAugment` → `CreateItem` (`shareddb.cpp:751`) — a fresh instance — then attaches
   evolve state only to the **host** (`:756`).
- 📌 A **runeword** system (D2 style: runes in sockets, in order, transform the base item) was
  evaluated against `EQEmu_Runewords_Source_Package` and **not built**. It drops in cleanly at the
  API level, but `MatchesRecipe` requires every socket past the last rune to be EMPTY, which fights
  the stat augments for the same sockets. Parked, not rejected.

### ⚠️⚠️ `required_amount` IS A PER-LEVEL COST, NOT A CUMULATIVE TOTAL (the sigil)
`current_amount` **resets to 0 on every evolve**, so each row states only what THAT level costs.
This was documented backwards for a day. Three things make the reset non-obvious:
1. the evolve path never transfers the amount — `SetEvolveCurrentAmount` appears ONLY in the XP
   transfer window paths (`client_evolving_items.cpp:469/471/517/519`);
2. the swap is `RemoveItemBySerialNumber` → `DeleteItemInInventory`, which **soft-deletes the row**
   (`zone/inventory.cpp:988`);
3. the replacement comes from `database.CreateItem()` — a fresh instance at 0 — and `DoLootChecks`
   inserts a NEW row for it.
⚠️ **Level 1 cannot tell the two readings apart** (same number either way), so a single observed run
at level 1 confirms nothing. That is exactly why the wrong version survived a live test.

### ⚠️ Evolving rewards ACTIVATE THEMSELVES — `AoTv4EnsureEvolveActivated` (zone/lua_client.cpp)
Gating accrual on the `activated` flag looks correct and is a trap: `NewEntity()` defaults it to 0,
and the native evolve creates a **brand new row** for the replacement item — so an item that was
active comes back INACTIVE after every evolve and silently stops progressing. The flag exists
natively to choose which item soaks a SHARED experience pool; our score is awarded explicitly, so it
buys nothing. Activate and carry on.
⚠️ `DoEvolveItemToggle` (`client_evolving_items.cpp`) had a **null dereference** reachable the moment
an evolving item could sit in a socket: `_HasItem` finds augments but returns the sentinel
`invslot::SLOT_AUGMENT_GENERIC_RETURN`, and `GetItem()` on that is nullptr. Guarded; keep the guard.

### The augment generator — `gen_delve_augs.pl`
316 rows in three CONTIGUOUS id blocks: **T1 147600-147615** (16, weight 2, single stat, exhaustive
one-per-stat-line), **T2 147616-147715** (100, weight 4, 1-2 lines), **T3 147716-147915** (200,
weight 8, 2-3 lines). hp/mana/endurance pay **10 points per weight**, so the tiers read 20/40/80 on
those lines and 2/4/8 on everything else — without that a "2 hp" T1 is a dead roll.
- ⚠️⚠️ **THE RNG IS A FIXED-SEED LCG AND MUST STAY THAT WAY.** These ids are live on player
  characters; a regen with a different seed silently rewrites the stats of augments people are
  wearing. Perl's own `rand()` is deliberately unused — it is per-process seeded and not stable
  across perl builds.
- ⚠️⚠️ **`sort` EVERY `keys` WHOSE ORDER CAN REACH THE OUTPUT.** Perl randomises hash key order per
  process; a bare `keys` in the weight distributor made the generator non-deterministic (three runs,
  three different files). **Verify after any change by regenerating and diffing.**
- ⚠️⚠️ **Rolls are deduped on the STAT SIGNATURE, not the name.** The first version deduped names
  only — when two rolls came out stat-identical it appended a roman numeral and kept both — which
  left **10 of 40 tier 2 rows redundant, 6 of them the same `CR 4`**. A quarter of the pool was
  wasted and Frost was six times likelier than any other single-stat T2.
- ⚠️ The requested count must stay well under the combination space or rejection sampling hangs:
  T2 weight 4 over 1-2 lines is ~376 combinations, T3 weight 8 over 2-3 lines is ~12,600.
  `$MAX_REROLLS` turns overshoot into a loud failure instead of a spin.
- ⚠️ `items.Name` is **varchar(64)**; longer is silently truncated, which can also collapse two
  distinct names into one. The generator widens the name then falls back to a roman numeral.
- ⚠️ `nodrop`/`norent` are **INVERTED**: `nodrop = 0` is No Drop, `norent = 1` is permanent.
- ⚠️ The DELETE clears the whole reserved band (147600-148199), not just the current rows — an
  earlier layout ran to 148007 and would otherwise strand its tail as orphans.
- ⚠️ **`items` is shared memory**: world down → `./shared_memory` → restart.

### The Refining Crucible does two different jobs — `AoTv4RefineCombine`
Item **2000060**, gated by item id (not bagtype). Four IDENTICAL gear items become 1 of the next gear
tier (+`AOTV4_TIER_STEP`); four **delve augments OF THE SAME TIER** become 1 **random** augment of the
next tier.
- ⚠️⚠️ **NOT "4 identical" for augments, and that difference is the point.** T2 and T3 have 100 and
  200 variants, so requiring four of the same roll would take hundreds of drops per upgrade. Four of
  a TIER is what makes the loop work.
- ⚠️ Each produced item rolls **independently** — combining eight gives two different results.
- ⚠️ Tier 3 is the top; four T3s are left alone rather than silently eaten.
- ⚠️⚠️ **THE TIER ID BLOCKS ARE MIRRORED IN THREE PLACES AND NOTHING ENFORCES IT**:
  `gen_delve_augs.pl`, `zone/aotv4_tiers.h`, and `aotv4_dungeon.lua M.AUG_TIER_BLOCK`. Change the
  variant counts and all three must move together — nothing fails to compile, the chest and the
  crucible just start handing out the wrong tier.
- 📌 `AOTV4_TIER_STEP`/`AoTv4TierBaseId` were migrated out of tradeskills.cpp into
  `zone/aotv4_tiers.h` when this landed (the header's own note says to migrate a duplicate the next
  time its file is touched). **Four duplicate sites remain**: loot.cpp, npc.cpp, questmgr.cpp,
  attack.cpp.

### The chest, the loot and the close timer
- ⚠️⚠️ **THE REWARD IS PLACED AT SPAWN AND THE DELVE DOES NOT CLOSE IMMEDIATELY.** The
  `on_chest_looted` hook fires on the chest's **death** — the instant the corpse appears, before
  anyone could have looted it. It used to call `M.leave` there, which destroyed the instance out from
  under the player **and took the augment and the platinum with it**. It now starts a client timer
  (`delveclose`, 120 s). ⚠️ A client timer **repeats** — `StopTimer` first in the handler, and
  `M.leave` stops it on every other way out.
- ⚠️ Loot goes on the **npc at spawn**: a corpse built from a lootless npc has nothing to add to.
  `AddCash(copper, silver, gold, platinum)` — **that argument order**.
  ⚠️ The coin is credited the moment the loot window OPENS (`AddMoneyToPP`, `corpse.cpp:1404`), and
  splits across the group if auto-split is on. The augment still has to be dragged out.
- **Coin = 10 platinum per rung, doubled under any affix** (non-Standard mode).
- ⚠️⚠️ **`eq.spawn2` RETURNS A `Lua_Mob`, NOT A `Lua_NPC`.** `AddItem`/`AddCash`/`ScaleNPC` are bound
  on Lua_NPC only, so calling one on the spawn2 result is *"attempt to call method 'AddItem' (a nil
  value)"* at runtime with nothing to warn you at write time. `CastToNPC` **is** bound (on
  Lua_Entity) — unlike `CastToClient`, so §24's entity-list workaround does not apply here.
  ⚠️ The cost when it bit was far wider than the chest: the error aborted the rest of
  `on_task_complete`, so the clear also paid no score and wrote no history. **The accounting now runs
  BEFORE the chest is touched** — everything irreplaceable is settled first.
- ⚠️⚠️ **DELVE MOBS DROP NOTHING** (`M.on_npc_spawn` → `ClearItemList` + `RemoveCash`). These are DoN
  zones and the stock drops are from an expansion the server has not unlocked. Done at spawn and
  **not** by editing loottable rows, because these are instances of REAL zones — stripping the tables
  would empty those zones for everyone, permanently. The nine stock augments that dropped in the six
  delve zones are removed from `lootdrop_entries` (verified to drop nowhere else); the two cosmetic
  ornaments are deliberately kept.

### ⚠️⚠️ THE SIGIL CURVE AND THE SCORING FUNCTION ARE ONE CALIBRATION, NOT TWO
`kill_value` is **`eff * eff`** (×3 named), no divisor and no minimum clamp. The sigil ladder
(**`4400 * n^2`**, written as per-level increments) is DERIVED from it — change one and the other is
wrong. **Delve score feeds the SIGIL ONLY**; augments are combined, not fed.
- ⚠️⚠️ **RETUNED `24000 * n^2` → `4400 * n^2` (2026-08-01) BECAUSE THE LEVEL CAP BECAME 30.** Exactly
  one input of the derivation changed: it assumes a player working on charm level n sits at rung
  **`7n`** (charm 10 ↔ rung 70), which was right at a cap of 70; a rung is only clearable if you can
  fight mobs scaled to it, so at a cap of 30 the reachable ceiling is ~30 and the mapping is **`3n`**.
  ⚠️ Run income is **quadratic in the rung**, so cutting the ceiling 70 → 30 does not halve run scores,
  it drops them by `(30/70)² ≈ 5.4×`. **Halving the cap does NOT mean halving this constant** — that
  intuition gives 12000 and is wrong by more than a factor of 2. Re-derive, don't scale.
  📌 `aotv4_dungeon.M.MAX_LEVEL` is deliberately **left at 70** (owner's call), so rungs above ~30 exist
  and stay unreachable. That is why the derivation uses the **character cap**, not `M.MAX_LEVEL` —
  reading the rung count puts it straight back to `7n`.
  Pace after the retune: ~90 runs climbing, ~28 parked at rung 30, ~25,600 farming rung 1.
- ⚠️ **`items_evolving_details` IS NOT SHARED MEMORY** — `EvolvingItemsManager::LoadEvolvingItems`
  reads it straight from the content DB (`common/evolving_items.cpp:31`) and is called once from
  `zone/main.cpp:376`. **Retuning the ladder alone needs only a zone restart**, no world down and no
  `./shared_memory`. (The sigil's `items` rows *are* shared memory; the charm SQL's header used to
  claim the ladder needed the same, which is wrong and would have cost a needless full cutover.)
- ⚠️⚠️ **`character_evolving_items.progression` is `double(22,0)` — ZERO decimal places**, i.e. an
  integer percent. Anything under 1% stores and displays as **0**, so a sigil that is genuinely
  accruing reads as "evolution experience didn't gain". Diagnose with `current_amount`, which is the
  real banked figure, never the percentage. This is what a 211-score run against the old 96,000
  level-2 requirement looked like (0.22% → 0).
- ⚠️⚠️ It used to be `floor(eff*eff/25)` clamped to a minimum of 1, and `floor(49/25)` is 1 — so
  **every effective level from 1 to 7 paid identically**, the quadratic did not engage until eff 8,
  and farming rung 1 was as efficient as rung 7.
- ⚠️⚠️ The ladder used to **double** each level (150, 450 … 76650). Doubling is EXPONENTIAL in the
  charm level while run score is QUADRATIC in the rung: charm levels 2-5 each took **under one run**,
  and the full chain was 18 runs against a stated design of ten per level. **An absolute number here
  means nothing on its own** — always divide it by what a run is actually worth.
- ⚠️⚠️ **UNSCALED MOBS MUST NEVER BE SCORED AT THEIR OWN LEVEL.** These are DoN zones, so an unscaled
  mob sits at level 60-70; `record_kill`'s fallback is the RUNG, never `npc:GetLevel()`. Nine such
  kills in a **level 3** delve were worth 4,900 each and produced a run score of **59,622** — enough
  to evolve the sigil outright — from a dungeon whose other 32 creatures were level 1. Three separate
  faults had to line up: the instance populates **before the player finishes zoning in** (so
  `on_npc_spawn` had no reference client and returned without scaling), the rescale sweep's power
  deadband was a **whole-sweep early return** that did nothing on any run after the first, and the
  ledger paid out at native level. All three fixed: with no player present a mob scales to the layer
  at power 1.0, the deadband is now a per-mob test that never skips an unstamped mob, and the
  fallback is the rung.

### 📌 OPEN — the level 35 rework invalidates most of the numbers above
The level cap is moving from 70 to **35**. Nothing here has been re-tuned for it, and the mismatch is
large enough to matter:
- **The augments are 1-7× an entire item at that cap.** Gear reachable at ≤35 averages **7.3 stat
  points / 6.3 AC / 10.8 HP** (banded by minimum dropper level — `reclevel` is useless for this,
  classic gear mostly leaves it 0, which is the §23 lesson again). A single T3 is 8 stat points or
  80 HP. Deliberately left as-is for now.
- **The delve is 70 rungs and `kill_value` is `eff²`**, so halving the ceiling **quarters** top-end
  run scores; the sigil ladder is then off by ~4×.
- `quest_difficulty.pl` bands on `level/10` clamped 1-8 and would only ever produce 1-4.
- 📌 A **craft-to-a-real-drop** system was designed but not built: feed augments into socketed gear
  and receive an actual droppable item whose stats match. The reachable pool at a 35 cap is **1,353
  items** banded 251 / 486 / 616 by minimum dropper level, which maps cleanly onto the three augment
  tiers. Parked until the cap rework lands, because the augment weights are an input to the matching.

## 27. Zone sharding (live's `/pick`) — NATIVE, built, deliberately NOT enabled — 2026-07-31

EQEmu already has live's picking system and it needs **no custom code**. Recorded here because it is
easy to miss and easy to re-implement by accident.

- **The switch is per zone**: `zone.shard_at_player_count` (0 = off). **0 of 618 zones use it today.**
- **Assignment is automatic on zone-in** (`zone/zoning.cpp:820`): it counts players per instance of
  that zone, drops you in the first shard **under** the threshold, and if all are full **creates a
  new instance** and puts you there. Players do not choose; they just land somewhere with room.
- **`#zoneshard`** is the player-facing command and is already `AccountStatus::Player` (access 0).
  Bare, it opens a saylink menu of live shards with populations (`Client::ShowZoneShardMenu`);
  `#zoneshard <zone> <instance>` jumps to one. ⚠️ `instance_id = -1` is the sentinel for "the normal
  unsharded zone", not an error.
- ⚠️ **Refused in combat** (`GetAggroCount() > 0`).
- ⚠️ Shards are created with a ~100 year duration (3,155,760,000 s), so they persist rather than
  expiring under the people standing in them.
- Rule `Zone:ZoneShardQuestMenuOnly` (currently false) suppresses the automatic assignment so only
  quests surface the menu — i.e. makes sharding opt-in.

### ⚠️⚠️ EVERY SHARD IS AN INSTANCE, SO EVERY SHARD COSTS A ZONE PROCESS AND A ZONE PORT
This is the reason it is off. The port range was widened to **7000-7029** on 2026-07-30 because six
was not enough for the `zone` launcher's **28 dynamics** (§0), and the Delve already burns an instance
per run (§24). Sharding stacks on top of both, and the failure mode is the one already seen: *"That
zone is unavailable"* and a bounce to character select. **Before enabling it anywhere, raise
`launcher.dynamics` AND the `appPort` / `eqemu_config.json` port range together** — those two must
always match, per §0.
📌 Enabling it later is one UPDATE on the target zone plus that headroom; nothing needs rebuilding.

## 28. Town factions — nobody is killed in a town for their race — 2026-08-01

All 16 races may be any class and go anywhere (§14 + the region system), but stock EQ faction is
built the opposite way round: an Ogre is born at −1000 with Kelethin and is cut down on sight by the
guards of a city the server has just told him he may walk into. Fixed by flooring the faction **con**
at Dubious for a curated set of civic factions. **Needs a zone rebuild** (`ruletypes.h` is in
`common/`, so it is a wide one).

- **Files**: `common/ruletypes.h` (`AoT:TownFactionFloor`), `zone/zone.h` + `zone/zone.cpp`
  (`LoadTownFactions` / `IsTownFaction`), `zone/client.cpp` (`Client::GetFactionLevel`),
  `custom/sql/aotv4_town_factions.sql` (the table + the 74-row list + the full derivation).
- ⚠️⚠️ **EVERY aggro branch requires the con to be exactly `FACTION_THREATENINGLY` or
  `FACTION_SCOWLS`** (`zone/aggro.cpp:326`, `:516`, `:545`) — and **`AlwaysAggro()` / `npc_aggro`
  sits in the ELIGIBILITY half of those conditions, not the faction half**, so an aggro-flagged NPC
  still needs a hostile con to swing. That is why the floor works at all, and equally why it would
  pacify **anything** it were applied to. Scope is the whole safety argument here.
- Thresholds are rules (`Faction:*FactionMinimum`): below −750 Scowls, −750..−501 Threatening, −500
  and up **Dubious**. `CalculateFaction` (`common/faction.cpp:57`) sums
  `earned + base + class_mod + race_mod + deity_mod`, so a fresh character's con **is** the race mod.
- ⚠️ **Dubious, not Indifferent.** The city still visibly dislikes you; it just does not murder you.
  The stock **merchant fix** immediately above our block (`client.cpp:8342`) already treats Dubious
  as tradeable, so shops keep working with no second exception.
- ⚠️ **The floor runs BEFORE the `CheckAggro` line**, which restores `THREATENINGLY` for an NPC that
  already has aggro on you — so a guard you attack still fights back. It prevents being attacked for
  existing, not combat you started.

### ⚠️⚠️ Why a con floor and NOT an edit to `faction_list_mod`
There are **7,134 negative race modifiers across 499 factions** (worst −2000) and clamping them was
the obvious move. It changes stored faction **values**, which is what quests read. **144 quest files**
test `e.other:GetFaction(e.self)`, and in EQ **lower is better** (Ally 1 … Scowls 9), so all of them
are "good enough?" tests at the opposite end of the scale from anything the floor touches — it only
ever moves 8/9 → 7. The four that test the hostile end were checked individually and all still pass:
`erudnext/Collier.lua` (`>= 7`, satisfied *by* a floor of 7), `erudnext/Weligon_Steelherder.lua`
(`> 5`), `felwithea/Tynkale.lua` (`> 4`), `westwastes/Sontalak.lua` (`>= 5`).
⚠️ **Re-verify that list if the floor is ever raised above Dubious** — Collier is the one that breaks.

### ⚠️⚠️ Why a FACTION list and not a ZONE list
**Kelethin is not a zone** — it is a treetop city inside `gfaydark`, which is also a real hunting
zone full of Crushbone orcs. A zone-scoped floor must choose between leaving Kelethin's guards lethal
and pacifying the orcs standing under it. Scoping by the NPC's own faction separates the two and
covers every other city for free. (`cancombat = 0` fails for the same reason, and more bluntly:
`GetFactionLevel` returns `FACTION_INDIFFERENTLY` outright when `!zone->CanDoCombat()`, `:8307`.)

### How the 74 were derived — and the two traps in deriving them
Candidates must be able to be hostile **by race alone** (some race below −500 before any earned
faction), then: named like a civic body, **and** show commerce — own merchants, or a majority of
their NPCs standing in a zone with 10+ merchants.
- ⚠️⚠️ **The commerce test is what keeps dungeon and raid factions with civic names OUT**: Nest
  Guardians (2,769 NPCs), Thunder Guardians (2,429), Citizens of Takish-Hiz (2,257), Inhabitants of
  **Hate** (900), Fallen Guard of Illsalin, Guardians of Veeshan, Befallen Inhabitants. All match the
  naming; all have zero merchants outside cities.
- ⚠️⚠️ **THE NAME TEST IS A KEYWORD LIST, NOT A CLASSIFIER, AND IT SILENTLY UNDER-COVERS.** It is
  built from Antonican and Faydwer vocabulary (*Merchants of, Guards of, Knights of, Militia,
  Residents*) and **nothing in Kunark is named that way** — a per-hub coverage check found Cabilis
  protected only by `Cabilis Residents` while its actual city guard, **`Legion of Cabilis`**, was
  still killing people on sight. Five Iksar guilds and `Jaggedpine Treefolk` were hand-added after
  verifying commerce. **📌 Check coverage per city after any change here; do not trust the name test.**
- The remaining gap was then closed by sweeping on **commerce alone, no name test**, which found six.
  Only three were towns (Heretics/Paineel, Wolves of the North/Halas, Gem Choppers). ⚠️ **The other
  three are raid content** — `Claws of Veeshan` (Skyshrine's dragons; Skyshrine and Great Divide both
  have merchants, so it scores 66% "city") and the three Seru factions (Sanctus Seru is literally a
  city *and* a raid target). **Commerce identifies civilisation, not friendliness** — which is why
  that sweep is a diagnostic, not the rule.
- ⚠️ **Deliberately left hostile** after being checked: `The Forsaken` (0 merchants; only 6 of its 263
  NPCs are in Cabilis — pacifying 257 wilderness mobs to fix 6 in town is the wrong trade),
  `Tunare's Scouts` (0 merchants, half its NPCs are inside Crushbone), `Pack of Tomar` (Dreadlands).
- ⚠️⚠️ **NEVER add a monster faction.** Crushbone Orcs, Sabertooths of Blackburrow, Goblins of
  Mountain Death, Frogloks of Guk, Trakanon and Vox all stay lethal. Adding one stops that content
  aggroing **server-wide, in every zone**, with no other symptom.
- ⚠️ Read at **zone boot**, not shared memory (only `items` and `spells` are shared) — editing the
  table needs a zone restart, no `./shared_memory` rebuild. A missing table is not an error: the
  server just behaves like stock.
- ⚠️ `faction_list_mod`.`mod` is a **MariaDB reserved word** and needs backticks everywhere.

## 29. Spell ranks, kept spells, and the parchment economy — 2026-07-31

**This system was live and completely undocumented here until 2026-08-01** — the only record was
`SPELL_RANKS.md`, a design draft still headed *"proposal, nothing built"*, which `.gitignore` was
also excluding (§0). Read that file for the design reasoning; this section is what actually shipped.

Three things are deliberately separate, and conflating them is the main way to misread the code:

| | what it is | survives death? |
|---|---|---|
| **KNOWN** | every spell this character has ever been awarded | **yes**, permanent |
| **RANK** | per BASE spell, 1-5 | **yes**, permanent |
| **KEPT** | at most **2** spells you START a run holding | this is the only part death touches |

- **Files**: `lua_modules/aotv4_spell_ranks_sys.lua` (all the logic), `lua_modules/spell_ranks.lua`
  (**generated** base → `{r2,r3,r4,r5}` map), `custom/tools/gen_spell_ranks.pl`,
  `custom/sql/aotv4_spell_ranks.sql` (the rows), `custom/sql/aotv4_spell_rank_economy.sql` (the two
  items + the global drop). Buckets, all per character: `spellrank_` / `spellknown_` / `spellkeep_`
  (permanent) and `pickspent_`.
- ⚠️⚠️ **A RANK IS A DIFFERENT SPELL ROW, NOT A MODIFIER.** 188 base spells × 4 ranks = **752 rows at
  43576-44327**, named `"… Rk. II"` … `"Rk. V"`. "Applying a rank" scribes a *different id*, which is
  why the native spellbook shows the real damage and the real mana cost with no client work at all.
- ⚠️⚠️ **THE NAMING IS LOAD-BEARING.** `gen_stock_pool.pl` filters the offerable pool with
  `name NOT LIKE '% Rk. %'` (§5), so the picker can only ever award a **base** spell, which this
  module then upgrades on the way in. **Renaming the rank rows would drop 752 upgraded spells straight
  into the reward pool.**
- ⚠️⚠️ **RANK IS A PROPERTY OF THE CHARACTER, NOT OF A SCRIBED COPY.** Earn rank III on a spell once
  and *every* future award of it arrives at rank III — from the picker, from a kept slot, from
  anywhere. It lives in a data bucket, so it survives the roguelite wipe like AA does.
  ⚠️ Therefore **keeping is NOT how you protect a rank** — the rank was never at risk. Keeping buys
  only the spell *in hand at level 1*, paid for by forfeiting the level-up pick at the level that
  spell was originally taken (`pickspent_`). Two kept, two picks lost: neutral in count, positive in
  control.
- ⚠️ **`ranked_id()` is the single funnel.** Everything that awards a spell goes through it, so there
  is one place that knows how to resolve base → owned rank. Do not add a second.
- **Economy**: **Parchment Fragment 147920** (one per spell destroyed on death, so income scales with
  how deep the run got — a full climb scribes ~31) and **Ink of the Lost 147921** (a `global_loot`
  drop at **1.5%**). Costs live in `M.COST` and **double for fragments while ink is flat +10**:
  rank 2 = 30/10, 3 = 60/20, 4 = 120/30, 5 = 240/40.
  ⚠️ The two scale differently **on purpose**: fragments come from *dying* so they track how hard you
  have been pushing, ink comes from *killing* so it tracks time played. A rank costs both.
  ⚠️ **Tune `M.COST` ONLY** — it is sent to the client as `SPELLRANKCOST` and the window renders
  whatever it says. There is no second copy in the dll, deliberately (the `kIcons[]` drift trap, §3).
- ⚠️⚠️ **FEEDBACK GOES TO THE WINDOW, NOT TO CHAT** (`SPELLRANKMSG`, written into the requirement panel
  under the description). Keeping and ranking happen inside a window that is already on screen, so a
  chat reply is both noise and the wrong place to look — and it is the player's own chat.
  ⚠️ **Both halves are needed**: the dll must also swallow the `/say spellkeep`-style **echoes**, or
  the commands stay visible even with every reply silenced. Say commands are intercepted in
  `Client::ChannelMessageReceived` (`spellkeep`, `spellrelease`, `spellrank`, `spellrankreq`,
  `spellkept`), the same place as the AdvLoot ones (§16), so they never reach chat or quests.
- ⚠️ **Ranks cover damage and healing only.** Buffs have no hook in either direction — a stat buff's
  magnitude is read straight from the spell row in `SpellEffect` — so there is nothing to scale.
- 📌 The cost curve is a first guess and should move once fragment income is *observed* rather than
  modelled. Same caveat as the sigil ladder in §26, and the same rule applies: divide it by what a
  run is actually worth before deciding it is too high or too low.

## 30. ⚠️ What is BUILT but NOT YET EXERCISED IN GAME (as of 2026-08-01)

Everything below compiles, applies and has been verified against the database or by static analysis,
but **no character has actually performed the action**. Listed so a tester knows where to look first
and so nobody reads "done" as "proven".

- **Town faction floor (§28)** — no character has walked into a hostile city as a KOS race. Fastest
  check: an Ogre in Kelethin or Felwithe, and an Iksar-hated race in Cabilis (Cabilis was the case
  that was actually broken and hand-fixed). Confirm guards ignore you AND that a guard you *attack*
  still fights back.
- **Crafting always yields Mythic** — no combine has been made. Any wearable recipe will do; the
  result should be the `Mythic …` name.
- **The two quest tier-matching fixes** — hand a **Mythic** copy of a craftable turn-in item to one of
  the 25 affected NPCs (Ghoulbane, Jagged Blade of War, Trueshot Longbow, Robe of the Lost Circle …).
  Those quests can now only ever *receive* a Mythic, so this is the sharp case.
- **The retuned sigil ladder (§26)** — one delve clear should now move the charm's percentage visibly.
  ⚠️ Remember `progression` is `double(22,0)`, so anything under 1 percent still reads **0**; judge it
  by `current_amount`, not the bar.
- **Delve rewards end to end** — no death has paid Parchment Fragments, no Ink of the Lost has
  dropped, no region-unlock achievement has fired, and the three Resplendent hub NPCs
  (Wayfinder Alessa / Reforger Vael / Herald Coren) have never been hailed by a real player.
- **Spell ranks (§29)** — the whole chain (keep 2, forfeit the picks, spend fragments + ink, receive
  the `Rk.` row) is untested in play.
