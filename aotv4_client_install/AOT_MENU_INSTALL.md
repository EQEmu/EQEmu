# AoT menu (`/aot`) — client install

A plain list of buttons, one per custom window, styled like the Advanced Loot window's buttons
(`BDT_Normal`, 110x26, coloured text — the "Always Need" look).

It opens by itself the first time you enter the world each session, and re-docks to the left of the
stock SC / EQ bar so both stay usable. Close it and it stays closed; `/aot` or `/menu` reopens it.

---

## Install

1. Rebuild `dinput8.dll` (VS2022, v143). Close EQ first.
2. Copy `EQUI_AoTMenuWnd.xml` to `<EQ>\uifiles\default\`.
3. Add to `<EQ>\uifiles\default\EQUI.xml`:
   ```xml
   <Include>EQUI_AoTMenuWnd.xml</Include>
   ```

**Nothing stock is modified.** `EQUI_EQMainWnd.xml` is NOT part of this — see below.

---

## ⚠️⚠️ Do not put buttons into the stock SC / EQ bar

An earlier attempt added an AoT button to `EQUI_EQMainWnd.xml` and caught its click by copying
`EQMainWnd`'s vtable and overwriting **entry 34** (the slot `SetWndNotification` writes), because
`CCustomWnd::ReplacevfTable` only ever operates on `this`.

**It crashed on the first click** and the approach was removed. It patches a window this dll did not
construct, on a build whose UI structs are documented as not matching the headers (CLAUDE.md
section 13), and it can only ever fail at runtime — there is no way to check it short of clicking.

The list is our own window instead. It receives notifications the ordinary way and cannot corrupt
anything stock. If you reverted `EQUI_EQMainWnd.xml` for the earlier attempt, keep it reverted.

---

## Check

| Do this | Expect |
|---|---|
| enter the world | the list appears next to the SC / EQ bar |
| click each button | its window opens |
| close it, then `/aot` | it comes back |

`<EQ>\aotv4_menu.log` traces it:

```
auto show on entering the world
```

Nothing in the log means the dll was not rebuilt or the flag is off. The window failing to appear
with that line present means `EQUI_AoTMenuWnd.xml` was not copied or not `<Include>`d.

---

## Buttons

| Button | Opens |
|---|---|
| Spells | the three-tab spell window (Choose / Known / Pool) |
| Autoskill | combat ability on/off plus reuse timers |
| Adv Loot | the advanced loot window |
| Trader | your shop (queues `/trader`, since the server sends its contents) |
| Achievements | the achievement window (queues `/ach`, same reason) |
| Search | in game item and spell search |
| Last Death | what you lost when you last died |
