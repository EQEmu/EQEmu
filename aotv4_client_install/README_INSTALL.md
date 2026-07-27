# Client install — what to copy, and where

Everything in this folder goes to the EQ client. Source of truth is the dev container; these are
exported/generated copies. Regenerate rather than hand-editing.

Last regenerated: **2026-07-27**, after the four AA trees (40 AAs), the pet ward rework, and the
native "You Lost" window.

---

## 1. `<EQ>\` (the EverQuest root, next to eqgame.exe)

| File | Why |
|---|---|
| `spells_us.txt` | **Stale copies break new spells.** The client renders spell names, class levels and cast times from this file. 69 custom spells were added (43300-43382) that an old copy has never heard of. |
| `dbstr_us.txt` | Spell and AA descriptions. Also where AA names come from. |
| `SkillCaps.txt` | Skill caps must match the server or the skill window lies. |
| `BaseData.txt` | Per-class base stats. |

Back up the originals first — these overwrite stock EQ files.

## 2. `<EQ>\uifiles\default\`

| File | Why |
|---|---|
| `EQUI_AoTSpellChoiceWnd.xml` | The level-up reward picker. **~8,600 lines** — it predefines one animation and one hidden button per spell icon, which is the only way to show per-row icons on this client. |
| `EQUI_AdvLootWnd.xml` | Advanced Loot window. |
| `EQUI_ShopWnd.xml` | Player shop (`/trader`). |
| `EQUI_AoTLostWnd.xml` | The "You Lost" death window, now a native SIDL window. |
| `EQUI_AoTAutoSkillWnd.xml` | Autoskill window (`/autoskill`) -- on/off per combat skill plus reuse timers. |
| `EQUI_AAWindow.xml` | **Overwrites a stock file** — renames the four AA tabs to Tank / Healer / Ranged / Melee. Back up the original. |

Then add the matching lines to `<EQ>\uifiles\default\EQUI.xml` among the other `<Include>` entries:

```xml
<Include>EQUI_AoTSpellChoiceWnd.xml</Include>
<Include>EQUI_AdvLootWnd.xml</Include>
<Include>EQUI_ShopWnd.xml</Include>
<Include>EQUI_AoTLostWnd.xml</Include>
<Include>EQUI_AoTAutoSkillWnd.xml</Include>
```

`EQUI_AAWindow.xml` needs **no** `<Include>` — it is a stock file EQUI.xml already includes, so
copying it over is the whole install.

### The AA tabs

The AA window has exactly four pages and the client picks one per AA from **`aa_ability.type`**:

| `type` | Stock label | Now reads |
|---|---|---|
| 1 | General | **Tank** |
| 2 | Archetype | **Healer** |
| 3 | Class | **Ranged** |
| 4 | Special | **Melee** |

So an AA lands on a tab purely by its `type` column — nothing else in the window is per-tab, and
`aa_ability.category` is a sub-grouping *within* a tab, not the tab itself.

The labels are safe to edit: the stock strings ("Archetype", "Special", …) do **not** appear in
`eqgame.exe`, so they are read from this XML rather than hardcoded. Page and listbox `item=` names
must stay exactly as they are — the client resolves most of them positionally, not by name.

⚠️ **Missing the `<Include>` is silent.** `CCustomWnd` cannot find its screen and simply returns —
no error, no window, nothing in any log. If a window "does nothing", check this first.

## 3. `dinput8.dll` — rebuild, do not copy from here

Built from `.devcontainer/repo/eq-core-dll/` (VS2022, toolset v143), then dropped next to
`eqgame.exe`. **Close EQ before copying — it holds the dll open.**

Rebuild needed whenever the dll sources change. Since the last build that includes: the deleted GDI
reward windows, the native spell picker's icon lookup, the Advanced Loot module, and the **native
"You Lost" window** (core_lostwindow.cpp) and the **Autoskill window** (core_autoskill.cpp).
See LOST_WINDOW_INSTALL.md and AUTOSKILL_WINDOW_INSTALL.md.

---

## Verifying it worked

- **Level up** → the reward window appears with three cards, each showing an icon
- **Blank icons** → `EQUI_AoTSpellChoiceWnd.xml` is stale; regenerate with
  `perl aotv4_client_install/gen_choice_xml.pl` and copy again
- **A spell shows as a number or blank name** → `spells_us.txt` is stale
- **No window at all** → the `<Include>` line is missing, or you are running an old dll

## Regenerating

```bash
# UI XML (reads the icon set out of the generated Lua, so run the pool generator first)
perl .devcontainer/custom/spells/gen_stock_pool.pl
perl aotv4_client_install/gen_choice_xml.pl
bash aotv4_client_install/validate_ui_xml.sh aotv4_client_install/uifiles_default/EQUI_AoTSpellChoiceWnd.xml

# client data files
cd /src/build/bin && ./export_client_files
cp export/{spells_us.txt,dbstr_us.txt,SkillCaps.txt,BaseData.txt} /src/aotv4_client_install/
```

⚠️ **Always run `validate_ui_xml.sh` before copying any `EQUI_*.xml`.** A double hyphen inside an XML
comment is illegal, aborts the whole file, and **crashes the client before character select**. That
one has cost two sessions. `UIErrorLog.txt` in the EQ root names the file and line when it happens.
