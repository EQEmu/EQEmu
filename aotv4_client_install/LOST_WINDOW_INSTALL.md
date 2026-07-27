# "You Lost" window — native SIDL conversion

Converts the death-loss list from a self-drawn GDI overlay to a native `CCustomWnd`. **Client-side
only — the server was not touched.** Built 2026-07-26, **not yet compiled or seen in game.**

## Why

The GDI overlay painted its own chrome, so it never matched EQ; it could not scale with the UI; and
it only worked in **windowed mode**, because this client's D3D device cannot be hooked. A SIDL
window sidesteps all three — it looks native because it *is* native. Same move the level-up picker,
Advanced Loot, Shop and Achievement windows already made.

## Install

1. **Rebuild `dinput8.dll`** (`eq-core-dll-vs2022.vcxproj`, toolset v143). Close EQ first — it locks
   the dll. Two new files are already added to the project: `core_lostwindow.cpp` / `.h`.
2. Copy **`EQUI_AoTLostWnd.xml`** to `<EQ>\uifiles\default\`.
3. Add to `<EQ>\uifiles\default\EQUI.xml`, among the other `<Include>` lines:

```xml
<Include>EQUI_AoTLostWnd.xml</Include>
```

⚠️ **Missing the `<Include>` is completely silent.** `CCustomWnd` cannot find its screen and simply
returns — no error, no window, nothing in any log. It is the first thing to check if the window
"does nothing".

## What changed behaviourally

Three things the overlay did by hand are now the UI engine's job and are simply gone:

- **Scrolling** — the listbox has a real scrollbar, so the hand-paged Up/Dn buttons are unnecessary.
- **Dragging** — a real title bar, and the window remembers where you put it.
- **The 45-second idle timeout** — a click-through layered window that outlives its usefulness is a
  nuisance, so the overlay auto-hid. A real window with a close box is not, so **the list now stays
  until you dismiss it**. You died; you can read it in your own time.

**Dismiss also clears the list**, so reopening with Ctrl+Q later shows nothing rather than the
contents of a corpse from two zones ago. The close box just hides it and keeps the list.

## Unchanged

- **Wire format**: the server still sends `LOSTDATA name^name^…`. No server rebuild, no SQL.
- **Ctrl+Q**: still opens the level-up picker when a reward is pending, and this window otherwise.
- `areLostWindowEnabled` in `_options.h` still gates the whole thing.

## The old overlay

`PaintLostOverlay`, `LostOverlayWndProc` and `LostOverlayThreadProc` are still in
`core_spellwindow.cpp` but are now **unreachable** — the thread is behind `if (false && …)` and
nothing sets `g_lostVisible`. Left in place rather than deleted so this change stays reviewable;
delete them once the native window is confirmed working, the way the GDI loot overlay was.

## Verifying

- **Die with items lost** → the window should appear, titled "You Lost", listing them.
- **Nothing lost** → no window at all. It only pops when the list is non-empty, deliberately: a
  death that cost nothing should not throw a window in your face on top of everything else.
- **Ctrl+Q** with a list present → reopens it. With no list → nothing.
- **Fullscreen** → it should now work, which is the clearest sign the conversion took. The old
  overlay was invisible outside windowed mode.
- **No window at all** → the `<Include>` line, or an old dll.
