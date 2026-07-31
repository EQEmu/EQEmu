# Delve window install (scaling dungeon)

The **server half needs nothing installed** and is already live — `/say delve`,
`/say delveenter <level>` and `/say delveexit` work by hand right now. These steps are only for the
native SIDL window and the launcher button.

## 1. Copy the UI files

From `C:\AoTv3\AoTv4\aotv4_client_install\uifiles_default\` into `<EQ>\uifiles\default\`:

| file | why |
|---|---|
| `EQUI_AoTDungeonWnd.xml` | the Delve window itself (new) |
| `EQUI_AoTMenuWnd.xml` | **overwrite** — gained the `Delve` button and grew 232 → 262 tall |

## 2. Add the Include

In `<EQ>\uifiles\default\EQUI.xml`, beside the other AoT lines:

```xml
<Include>EQUI_AoTDungeonWnd.xml</Include>
```

⚠️ **Missing this is the single most common failure and it is completely silent.** `CCustomWnd`
cannot find the screen `AoTDungeonWnd`, returns a NULL `pXWnd`, and nothing appears — no error in
game, nothing in `UIErrorLog.txt`. If the window does not open, check
`<EQ>\aotv4_dungeon.log`: it records `pXWnd=NULL means the XML is not loaded` for exactly this case,
and distinguishes it from an unbuilt dll (no log file at all) and from the UI managers being null.

`EQUI_AoTMenuWnd.xml` needs **no** Include — it already has one.

## 3. Rebuild `dinput8.dll`

Visual Studio, `eq-core-dll/src/eq-core-dll-vs2022.vcxproj`, toolset v143. **Close EQ first** — it
locks the dll.

⚠️ `core_dungeon.cpp` and `core_dungeon.h` are already added to the vcxproj. A `.cpp` missing from
that file silently does not compile, and the symptom is identical to a missing Include.

## 4. Two tabs

**Layers** — pick a level, Enter / Exit Delve.

**Score Sheet** — every finished run, newest first, in the Death Book style: one header row per run
with `[+]`/`[-]`, and clicking it opens the level-band breakdown underneath.

```
[-] 07-29 16:04  lvl 50  Lavaspinner's Locals    4820   47   cleared
      average level 51.3  (weakest 45, strongest 68)
      level 45 to 49: 12 killed
      level 50 to 54: 20 killed
      level 55 to 59: 15 killed
[+] 07-29 15:31  lvl 50  Lavaspinner's Locals     980   14   DIED
```

Result colours: cleared (gold), **DIED** (red), abandoned (grey). A run with no kills is not recorded.

## 5. Try it

- **`/delve`** (or `/dungeon`), or the **Delve** button on the `/aot` launcher.
- Select a level, press **Enter** — you should be moved into a private instance with a
  `Delve: <dungeon> [<level>]` entry in the quest journal.
- Kill things. The journal counts them.
- On completion a chest drops **where you struck the last blow**, and the next level appears in the
  window.
- **Exit Delve** (or looting the chest) closes the instance and puts you back where you entered from.

Only level **50** is available on a fresh character — the rest unlock one at a time as you clear
them. That is server-enforced: locked layers are never sent to the client at all.

## Testing notes

- The **level gate is not enforced on the character**, only the unlock chain, so a low-level
  character can enter layer 1 and be killed instantly by level 50 mobs. That is expected — it is
  the fastest way to test the plumbing without levelling first.
- To reset the unlock chain for a character:
  ```sql
  DELETE FROM data_buckets WHERE `key` LIKE 'delve_cleared_<charid>';
  ```
- To clear a stuck run (if you ever end up marked as inside a delve you are not in):
  ```sql
  DELETE FROM data_buckets WHERE `key` LIKE 'delve_run_<charid>';
  ```
- `aotv4_dungeon.log` in the EQ root traces every `DUNGDATA` line received and the
  `EnsureWindow` pointer values — the fastest way to tell "not built" from "XML not loaded".

## Boss models: `Resources/GlobalLoad_chr.txt`

**File:** `aotv4_client_install/Resources/GlobalLoad_chr.txt`
**Copy to:** `<EQ>\Resources\GlobalLoad_chr.txt`  (a SUBFOLDER, unlike `spells_us.txt` and friends,
which go in the EQ root)

⚠️ **Back up the original first.** A malformed global load file fails at client startup, before
character select, so the stock 13 line version is your way back.

### What it is for

Delve wardens use models from **after** the Dragons of Norrath era so they never look like a bigger
version of the local wildlife. A model only renders in a zone whose `_chr` archive contains it, and
the delve zones (DoN) contain none of these — `GlobalLoad_chr.txt` is the client's list of character
archives to load in EVERY zone, which is the only way to get them there. There is no server side
switch for this and no way to target a single zone.

The five archives added carry **47 usable races** between them:

| Archive | Races | Era |
|---|---|---|
| `sepulcher_chr` | 12 | Seeds of Destruction |
| `beastdomain_chr` | 13 | House of Thule |
| `pillarsalra_chr` | 9 | Veil of Alaris |
| `thulehouse2_chr` | 9 | House of Thule |
| `shardslanding_chr` | 4 | Veil of Alaris |

⚠️⚠️ **This file and `M.BOSS_RACES` in `lua_modules/aotv4_dungeon.lua` are ONE UNIT.** A race listed
in the Lua whose archive is not listed here renders as an untextured placeholder or a generic human
— and it fails only on the client, with nothing wrong server side and nothing in any log.

⚠️ **Line 1 is a COUNT.** Adding an entry without incrementing it means the client stops reading
before your new line and silently ignores it.

⚠️ **CRLF line endings**, like the stock file. Do not let an editor rewrite them to LF.

### Choosing archives (if you extend it)

Pick by ARCHIVE, not by race: the archive is the unit of client memory, so the question is which
FILE carries the most usable races, not which race you happen to want. Choosing races individually
gave 10 races for 5 archives; choosing the five densest archives gave 47 for the same five.

There are **160** post DoN races in total. The next densest archives are `morellcastle` (13),
`hillsofshade` (13) and `sarithcity` (12).

⚠️ Memory matters here: `eqgame.exe` is 32 bit and NOT large address aware (PE characteristics
`0x0102`), so the whole client lives in **2 GB** of address space. A handful of archives is fine;
loading every race is not, which is why this is a curated list rather than a switch.
