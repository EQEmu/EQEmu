# Native SIDL Advanced Loot Window (`/advl`) — client install

An EQ-styled **Advanced Loot** window (`AdvLootWnd`), rendered by the client's own SIDL UI engine (same
approach as the achievement + shop windows). It runs in **complement mode**: the stock RoF2 loot window
still opens on a corpse, and this window lists the *same* corpse's items with **Loot / Leave / Never**
plus **Loot All**, and a **Filters** tab for the persistent Never list.

## Install (client side)
1. Rebuild **our `dinput8.dll`** (the `AdvLootWnd` C++ + chat routing live in `core_spellwindow.cpp`).
2. Copy **`EQUI_AdvLootWnd.xml`** → `<EQ>/uifiles/default/`.
3. Add this line inside `<EQ>/uifiles/default/EQUI.xml` (next to the other `<Include>` lines):
   ```xml
   <Include>EQUI_AdvLootWnd.xml</Include>
   ```
4. Launch and loot a corpse — the window auto-opens when the corpse has items. **`/advl`** re-opens it
   (also the way to reach the Filters tab with no corpse open).

## Flow
- **Personal Loot tab:** select a row, then
  - **Loot** — loots that item into the first **free inventory slot** (cursor only if your bags are
    full). Routed through the native `Corpse::LootCorpseItem`, so lore/weight/cursor/corpse-removal
    checks are the stock ones.
  - **Leave** — no-op; the item stays on the corpse.
  - **Never** — adds the item **id** to your persistent Never list; it's hidden from this and every
    future corpse listing (it is *not* destroyed — it stays on the corpse and the native loot window
    still shows it).
  - **Loot All** — loots every listed (non-filtered) item. If your bags fill and something lands on the
    cursor it stops there and says so — clear the cursor and click Loot All again.
- The **Qty** column shows the real stack size for stackable loot (arrows, food, …), 1 otherwise.
- **Filters tab:** shows your Never rules; select one → **Remove Filter**.
- The window closes when the corpse session ends (closing the loot window, or zoning).

## Notes
- ⚠️ **Never put a double hyphen inside an XML comment.** It is illegal XML, and the SIDL parser aborts
  the **entire file** — the client then **crashes at UI load** (before you ever reach the game). The tell
  is `UIErrorLog.txt` in the EQ root: `[Line:N Source:UIFiles\Default\EQUI_AdvLootWnd.xml]
  ParseNodeList() SyntaxError` followed by `Error reading XML.` The line number points straight at it.
  This bit exactly once, on a `--` used as a dash in the header comment.
- Requires the matching **server build** — the protocol lives in `zone/client_packet.cpp`
  (`SendAdvLootData` / `SendAdvLootFilters` / `SendAdvLootClose` / `HandleAdvLootSay`) and
  `zone/client.cpp` (say interception). Chat protocol: `LOOTDATA` / `FILTERDATA` / `LOOTCLOSE` out,
  `/say alspick|alslootall|alsrefresh|alsfilters|alsfilterdel` back. The dll swallows all of it.
- Never rules persist per character in the `alsnever_<charid>` data bucket.
- If the window never appears, it's the SIDL manager wiring — `EnsureAdvLootWindow` applies the same
  `ppSidlMgr`/`ppWndMgr` fix as the achievement/shop windows (set from client globals if unset).
- Layout is **static** — no runtime `Move`/`GetScreenRect` (those struct-returning calls crash this
  client build). Resize the window in-game instead; keep ONE listbox of each type in the XML (a second
  listbox caused a UI-load error in the shop window).
