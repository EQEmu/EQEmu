# Spell window — Choose / Known / Pool — client install

The level-up reward picker now carries **three tabs**:

| Tab | What it is |
|---|---|
| **Choose** | the reward cards — icons, two-step Select then Confirm. **Unchanged.** |
| **Known** | every spell this character has scribed, under level band headers |
| **Pool** | what can still be *offered* at a chosen level, with a `<<` / `>>` stepper |

The Pool tab is why this exists. The reward pool is **2,154 spells across 78 levels** and was
previously invisible: a player could see what they had been handed, never what they might be.

> ⚠️ **There is no separate Journal window any more.** It was built as one
> (`EQUI_AoTSpellJournalWnd.xml`) and then folded into the picker, so there is a single spell window
> rather than two. That XML is **still on disk as a backup and is no longer used** — do **not** copy
> it and do **not** add an `<Include>` for it.

---

## Install

1. **Rebuild `dinput8.dll`** (VS2022, `eq-core-dll-vs2022.vcxproj`, toolset v143).
   **Close EQ first** — it locks the dll.

2. **Copy the regenerated UI file** into `<EQ>\uifiles\default\`:

   ```
   aotv4_client_install/uifiles_default/EQUI_AoTSpellChoiceWnd.xml
   ```

   ⚠️ This file is **generated**. If the reward pool changes, re-run
   `perl aotv4_client_install/gen_choice_xml.pl` and copy it again — its per-row icon set is built
   from `spell_icons.lua` + `skill_pool.lua`, and a stale copy silently draws blank icons.

3. **`<Include>` line** — if you already had the picker installed, it is **already there** and
   nothing changes:

   ```xml
   <Include>EQUI_AoTSpellChoiceWnd.xml</Include>
   ```

4. **Server side is already live** — `spell_choice.send_pool` / `handle_journal_say`, routed from
   `global_player.event_say`. Lua modules are `require`d once per zone process, so a **zone restart**
   is needed, not `#reloadquest`.

---

## Use

- **Level up** → the window opens by itself on **Choose**, exactly as before.
- **`/journal`** or **`/spells`** → opens it for browsing with no reward pending.
- **Ctrl+Q** → unchanged; still only opens when a reward is actually owed.

Pool rows are **gold** for offerable and **grey** for spells you already know. Selecting any row on
either tab fills the detail pane below it.

---

## Diagnosing

`<EQ>\aotv4_spelljournal.log` traces the two browse tabs:

| Log line | Meaning |
|---|---|
| nothing at all | the dll was not rebuilt, or `areSpellJournalEnabled` is off |
| `SjRefreshKnown: 0 rows` | no spells scribed, or `CHARINFO2` was not readable yet |
| `transport: level N chunk 1/2 …` | working; the server is answering |
| `transport: dropping stale chunk` | normal — a late answer for a level you paged away from |

If the **window itself** does not appear, that is the picker, not this module: check the
`<Include>` and that the regenerated XML was copied.

---

## Notes

- **Descriptions on Known and Pool are built client side** from the client's own spell record, so
  they cannot drift and cost no chat traffic. The Choose tab still uses the server's
  `SPELLDESCDATA` text — that path is untouched.
- A spell whose every effect slot is empty shows *"Handled by the server; no client-visible
  effects."* rather than "no effects". Several of ours are deliberate inert marker buffs paid by Lua
  (the Thirst line 43342-43347, the Shield Wall buffs). That wording is deliberate.
- A Pool row reading **`Spell #NNNNN (client data missing)`** means the client's `spells_us.txt` is
  older than the server's spell set. Re-run `./export_client_files` and copy it to the EQ root.
- ⚠️ Run `bash aotv4_client_install/validate_ui_xml.sh` before copying any `EQUI_*.xml`. A `--`
  inside an XML comment crashes the client at UI load, before char select.
