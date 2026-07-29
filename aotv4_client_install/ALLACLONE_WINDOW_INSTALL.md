# Allaclone window — client install

The in-game item / NPC / spell / recipe lookup, formerly the GDI **Search** overlay. It is now a
**native SIDL window** (`CCustomWnd "AoTAllacloneWnd"`), so it is drawn by the client's own UI
engine: it matches whatever skin the player uses, scales with the UI, and works in **full screen**
as well as windowed — none of which the overlay could do.

**The server was not touched.** The wire format (`srch` / `srchdet` → `SRCHDATA` / `SRCHDET`) is
byte-for-byte what the overlay used, and `Client::SearchList` / `Client::SearchDetail` are unchanged.
This is a rendering change only.

## Steps

1. **Rebuild `dinput8.dll`** (VS2022, v143, `eq-core-dll-vs2022.vcxproj`). Close EQ first — it locks
   the dll.
   ⚠️ `core_allaclone.cpp` is a **new file**; it is already added to the `.vcxproj`, but if you build
   from a different project file it must be added there too or it silently does not compile and the
   window never appears.
2. Copy **`uifiles_default/EQUI_AoTAllacloneWnd.xml`** to `<EQ>\uifiles\default\`.
3. Add to `<EQ>\uifiles\default\EQUI.xml`, beside the other AoT includes:
   ```xml
   <Include>EQUI_AoTAllacloneWnd.xml</Include>
   ```
4. Copy **`uifiles_default/EQUI_AoTMenuWnd.xml`** as well — the launcher's button is relabelled
   from "Search" to "Allaclone".

## Using it

- `/allaclone` — and `/search` / `/find` still work, so nobody has to relearn a command.
- The **AoT menu** (`/aot`, Ctrl+Q) has an Allaclone button.
- Four kind buttons across the top: **Items / NPCs / Spells / Tradeskills**. The active one is
  marked `> like this <`. Switching kind re-runs the current term against the new kind.
- Type a name and press **Search**, or just stop typing — the box is polled and searches once the
  text settles (~0.45 s).
- Click a result for its full detail in the bottom pane, rendered as STML like a real item inspect.

## If nothing appears

`<EQ>\aotv4_allaclone.log` is the dll's own trace and answers the three failures that look identical
from in game:

| Log line | Meaning |
|---|---|
| *(file absent entirely)* | the dll was not rebuilt, or `areSearchWindowEnabled` is false |
| `EnsureWindow: UI managers still null` | `ppSidlMgr` / `ppWndMgr` wiring failed |
| `created AllacloneWnd, pXWnd=0000000` | **the XML is not loaded** — step 2 or 3 was missed |

That last one is by far the most common, and it is otherwise completely silent: `CCustomWnd` cannot
find its screen and simply returns, with no error anywhere.

## Notes

- The old GDI overlay is still in `core_spellwindow.cpp` but is **unreachable** —
  `EnableSearchWindow()` / `ShowSearchWindow()` are now forwarders to the new module and
  `g_searchEnabled` is never set. Delete `SearchPaint` / `SearchWndProc` / `SearchThreadProc` /
  `HandleSearchChat` and their state once this is confirmed working.
- ⚠️ Always run `bash aotv4_client_install/validate_ui_xml.sh` before copying any `EQUI_*.xml`. A
  `--` inside an XML comment aborts the whole file at UI load and kills the client before character
  select; `UIErrorLog.txt` in the EQ root names the file and line.
