# Autoskill window

A native SIDL window for the autoskill system: the combat specials this character has, an on/off per
skill, and the reuse timers counting down. Built 2026-07-27, **not yet compiled or seen in game.**

## What already existed

**The autoskill system predates this window and was not changed.** Server side it was already:

- `Client::GetAvailableAutoSkills()` — the ten activated combat specials
- `Client::GetAutoSkillsList()` — those this character actually has
- `Client::Get/SetAutoSkillStatus()` — per-skill on/off, persisted in `autoskill.<id>` data buckets
- the auto-fire loop in `Client::Process` — runs your enabled specials while auto-attacking a
  non-client target
- `#autoskill [skill] [enable|disable|status]` and `#autoskill list`

`#autoskill` still works exactly as before. This adds only the window and its transport.

## Install

1. **Rebuild `dinput8.dll`** (`eq-core-dll-vs2022.vcxproj`, v143). Close EQ first. New files
   `core_autoskill.cpp` / `.h` are already in the project.
2. Copy **`EQUI_AoTAutoSkillWnd.xml`** to `<EQ>\uifiles\default\`.
3. Add to `EQUI.xml`:

```xml
<Include>EQUI_AoTAutoSkillWnd.xml</Include>
```

⚠️ **A missing `<Include>` is completely silent** — no error, no window, nothing in any log.

4. The **server** side is already built and deployed (`ninja zone` done, zones cycled).

## Using it

`/autoskill` opens it. **Two native tabs**:

**Abilities** — every combat special you have. Select one, then **Enable** or **Disable**;
**All On** / **All Off** do the lot. Green means auto-firing, grey means off.

**Timers** — only the skills autoskill is actually firing, with `Ready In` counting down and the
nominal `Reuse` alongside. A disabled skill has no timer worth watching, so it is not listed.

⚠️ These are REAL native tabs (`TabBox` / `Page` / `TabText`), not a hand-rolled toggle button. All
six tags are in this client's SIDL parser and both `FT_DefTabBorder` / `FT_DefPageBorder` are
defined in `EQUI_Templates.xml`. The Advanced Loot window fakes its tabs only because that was not
known at the time. Multiple Listboxes per window are likewise fine.

## How the timer works, and why

⚠️ **The countdown runs on the client**, not streamed from the server. The server sends the remaining
seconds with each update and the client ticks it down per frame. Pushing a fresh `ASKILLDATA` every
second would be a chat line per second per player, and **the RoF2 chat pipe drops and reorders
bursts** — the same failure that made achievement completions "update only sometimes".

Local counting alone drifts the moment autoskill fires something, since the client cannot see a timer
restart. So the window **resyncs every 2.5 seconds, but only while it is actually on screen**. Closed,
it costs nothing at all.

⚠️ The **Timers page skips disabled skills**, so its row N is not skill N -- it carries its own
`m_torder` row-to-skill mapping and the countdown walks that. Writing straight through would put
each timer on the wrong row.

⚠️ The list is only **rebuilt when the rows change** (a skill gained, or one toggled). A rebuild calls
`DeleteAll`, which clears the selection — doing that on every resync would yank the selection out from
under you every couple of seconds. Otherwise just the timer column is rewritten in place.

## Protocol

```
server -> dll : ASKILLDATA <n>^skillid|name|enabled|cooldown_secs|reuse_secs^...
dll -> server : /say askset <skillid> <0|1>
                /say askrefresh
```

Intercepted in `Client::ChannelMessageReceived` before EVENT_SAY, so the commands never reach chat or
quests — same treatment as the Advanced Loot `als*` commands.

## Two things worth knowing

⚠️ **Taunt reads a different timer.** Every other special uses the per-skill reuse timer
(`AOTV4_SKILL_TIMER_BASE + skill`), but Taunt runs on the stock `pTimerTaunt` — the auto-fire loop
special-cases it for the same reason. `GetAutoSkillCooldown` handles it; reading the wrong one would
report a permanently-ready Taunt.

⚠️ **A never-started timer reports `0xFFFFFFFF`, not 0** (`PersistentTimer::GetRemainingTime` returns
that when disabled). Without the guard in `GetAutoSkillCooldown`, a ready skill would display a
49-day cooldown.

## If nothing happens

The dll writes **`<EQ>\aotv4_autoskill.log`**. Three lines tell the three causes apart:

| Log says | Meaning |
|---|---|
| nothing at all, no file | the dll is not rebuilt, or `areAutoSkillWindowEnabled` is false |
| `InitAutoSkill: module enabled` but no `AutoSkillShow` | the `/autoskill` intercept never fired |
| `EnsureWindow: ... pXWnd=0000000000` | **the XML is not loaded** -- missing `<Include>` in EQUI.xml, or the file was not copied |
| `EnsureWindow: UI managers still null` | the `ppSidlMgr`/`ppWndMgr` wiring failed (see section 15) |

Also check **`<EQ>\UIErrorLog.txt`**: if the XML itself failed to parse it names the file and line.

⚠️ EQ printing an **unknown command** error is diagnostic in itself -- it means the dll never saw
`/autoskill`, so it is an old build. Silence means the dll DID intercept it and the window failed.

## Verifying

- `/autoskill` → the window lists only skills you actually have. An empty list means you have none of
  the ten specials yet, which is correct for a fresh character.
- Toggle one → the row turns green and the Auto column flips. `#autoskill <skill> status` should agree.
- Auto-attack something → enabled skills fire, and their Ready In counts down and resets.
- Close the window → no further chat traffic.
- **No window at all** → the `<Include>` line, or an old dll.
