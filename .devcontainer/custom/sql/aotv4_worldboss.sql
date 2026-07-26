-- aotv4_worldboss.sql -- the roaming world boss NPC and its loot.
-- =============================================================================================
-- A single boss that can be armed for any classic dungeon with #worldboss. Everyone who damages it
-- may loot the corpse (granted in lua_modules/aotv4_worldboss.lua, up to the 72 a corpse can hold),
-- so a pickup crowd can fight it together and all roll through the Advanced Loot window.
--
-- ⚠️ THE NAME IS A PLACEHOLDER. It is called "#The_Nameless" precisely because the boss is
-- undecided -- the leading # marks it as a NAMED npc, which is the convention the achievement
-- hunter categories key off (section 15). Renaming it later is a one-line UPDATE; nothing in the
-- Lua matches on the name, only on npc id 2000200.
--
-- Cloned from 32040 Lord_Nagafen, a classic dragon, so every unset column is a working boss rather
-- than a default. Then tuned DOWN: Nagafen is a level 55 raid dragon with 32000 hp, which is wrong
-- for something a pickup group at the current level 50 cap is meant to beat.
--
--   level  50        at the era cap, so it is a real fight for a capped character
--   hp     120000    long enough to need several people, short enough not to be a slog
--   size / race      kept from the template
--
-- ⚠️ Tuning is a guess until it is fought. Expect to adjust hp and damage after the first kill.
--
--   mysql -h127.0.0.1 -upeq -ppeqpass peq < .devcontainer/custom/sql/aotv4_worldboss.sql
--
-- ⚠️ npc_types is NOT in shared memory (unlike items/spells) -- a zone restart picks it up. The
-- loottable IS read at zone boot too. No ./shared_memory run needed for this file alone.
-- =============================================================================================

DELETE FROM npc_types       WHERE id = 2000200;
DELETE FROM loottable       WHERE id = 200020;
DELETE FROM loottable_entries WHERE loottable_id = 200020;
DELETE FROM lootdrop        WHERE id = 200020;
DELETE FROM lootdrop_entries WHERE lootdrop_id = 200020;

-- ---------------------------------------------------------------- the NPC
DROP TEMPORARY TABLE IF EXISTS aotv4_npc_tmpl;
CREATE TEMPORARY TABLE aotv4_npc_tmpl LIKE npc_types;
INSERT INTO aotv4_npc_tmpl SELECT * FROM npc_types WHERE id = 32040;

UPDATE aotv4_npc_tmpl SET
    id            = 2000200,
    name          = '#The_Nameless',
    lastname      = 'the Unbidden',
    level         = 50,
    hp            = 120000,
    mana          = 20000,
    loottable_id  = 200020,
    npc_faction_id = 0,          -- hostile to everyone, no faction hits either way
    -- Roaming boss, not a raid dragon: no unkillable specials, no summon-and-melt.
    special_abilities = '1,1^2,1^7,1^13,1^14,1^24,1',   -- summon, enrage, rampage, slow-immune, mez-immune, no-harm-from-fear
    npc_spells_id = 0,           -- no spell list yet; melee only until it is tuned
    aggroradius   = 300,
    assistradius  = 300,
    runspeed      = 1.25;

INSERT INTO npc_types SELECT * FROM aotv4_npc_tmpl;
DROP TEMPORARY TABLE IF EXISTS aotv4_npc_tmpl;

-- ---------------------------------------------------------------- loot
-- Deliberately small and obviously placeholder. The point right now is to prove that everyone who
-- fought can see and roll on the corpse, not to be a reward table -- curate this once the fight is
-- tuned. Hallowed tier (base id + 300000, section 10) so it is worth turning up for.
INSERT INTO loottable (id, name, mincash, maxcash, avgcoin) VALUES
 (200020, 'The Nameless', 5000, 25000, 15000);

INSERT INTO loottable_entries (loottable_id, lootdrop_id, multiplier, probability)
VALUES (200020, 200020, 3, 100);

-- Three rolls from a small pool, so a group gets several items to distribute rather than one.
INSERT INTO lootdrop (id, name) VALUES (200020, 'The Nameless - placeholder');

INSERT INTO lootdrop_entries (lootdrop_id, item_id, item_charges, equip_item, chance)
SELECT 200020, i.id, 1, 1, 20
FROM items i
WHERE i.id BETWEEN 300000 AND 900000        -- Hallowed / Mythic band
  AND i.slots > 0 AND i.reqlevel BETWEEN 35 AND 50
ORDER BY RAND()
LIMIT 5;
