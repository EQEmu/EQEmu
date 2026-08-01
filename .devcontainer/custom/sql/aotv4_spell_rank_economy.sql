-- AoTv4 -- the currency behind spell ranks: Parchment Fragments + Ink of the Lost.
-- =====================================================================================
--   147920  Parchment Fragment   awarded on death, one per spell the roguelite wipe destroyed
--   147921  Ink of the Lost      low-chance global drop off any mob, the scarce half of the recipe
--
-- Upgrading a spell one rank consumes both. Costs double per rank (see aotv4_spell_ranks_sys.lua),
-- so rank 1 is a single deep run and rank 5 is a long project.
--
-- ⚠️ BOTH ARE TRADEABLE AND NOT LORE, as specified. That means an economy: fragments become the
-- currency of the rank system and can be bought from other players. Deliberate, but it does mean
-- plat converts into spell power at whatever rate the playerbase settles on.
-- ⚠️ `nodrop` and `norent` ARE INVERTED in this schema: nodrop = 0 means No Drop, norent = 1 means
-- permanent. So tradeable+permanent is nodrop=1, norent=1. Getting this backwards produces items
-- that silently vanish on camp.
-- ⚠️ Cloned VIA A TEMP TABLE so all ~180 columns stay byte-identical to the template (section 5).
-- ⚠️ `items` IS SHARED MEMORY: world down, ./shared_memory, restart.

DELETE FROM items WHERE id IN (147920, 147921);

CREATE TEMPORARY TABLE aotv4_tmp_item AS SELECT * FROM items WHERE id = 2093;  -- Small Portal Fragments

UPDATE aotv4_tmp_item SET
    id = 147920, Name = 'Parchment Fragment',
    stackable = 1, stacksize = 1000,   -- a deep death yields ~31; rank 5 needs 480 in one stack
    nodrop = 1, norent = 1, loregroup = 0,
    icon = 682, price = 0,   -- Sealed Parchment. ⚠️ NOT 645, which is a COIN (Aged Gold Coin, Token of Leadership)
    -- ⚠️ Sell value 0 on purpose. These are the rank currency; a vendor price would let a player
    -- convert them to coin at a fixed rate and quietly set a floor under the player economy.
    sellrate = 0;
INSERT INTO items SELECT * FROM aotv4_tmp_item;

UPDATE aotv4_tmp_item SET
    id = 147921, Name = 'Ink of the Lost',
    stackable = 1, stacksize = 100,
    icon = 2047;   -- the icon 'Ink of Reach' and the Bottle of Spirits line use. ⚠️ NOT 646 (a coin)
                   -- and not 1155 (a vial, but reads small); 2047 is used by 33 bottle/ink items.
INSERT INTO items SELECT * FROM aotv4_tmp_item;
DROP TEMPORARY TABLE aotv4_tmp_item;

-- ---------------------------------------------------------------- the global drop
-- ⚠️⚠️ `global_loot` HAS NO CHANCE COLUMN. The rate lives in the lootdrop_entries row it ultimately
-- points at, so the drop needs its own loottable + lootdrop pair rather than being a one-liner.
DELETE FROM global_loot        WHERE id = 100;
DELETE FROM loottable          WHERE id = 200030;
DELETE FROM lootdrop           WHERE id = 200030;
DELETE FROM loottable_entries  WHERE loottable_id = 200030;
DELETE FROM lootdrop_entries   WHERE lootdrop_id  = 200030;

INSERT INTO loottable (id, name, mincash, maxcash, avgcoin, done) VALUES
    (200030, 'AoTv4 Ink of the Lost (global)', 0, 0, 0, 0);
INSERT INTO lootdrop (id, name) VALUES
    (200030, 'AoTv4 Ink of the Lost (global)');
-- probability 100 = this lootdrop is always CONSIDERED; the per-item chance below is the real rate.
INSERT INTO loottable_entries (loottable_id, lootdrop_id, multiplier, droplimit, mindrop, probability)
    VALUES (200030, 200030, 1, 1, 0, 100);
-- ⚠️ chance 1.5 = 1.5 percent per kill, about one ink per 67 kills. Ink is DELIBERATELY the long
-- pole: at 10/20/30/40 per rank, one spell to rank 5 is ~100 ink, roughly 6,700 kills. Tune HERE,
-- not in global_loot,
-- which has no chance column at all.
-- ⚠️ Loot is read at ZONE BOOT, not from shared memory, so changing this needs a zone restart only.
INSERT INTO lootdrop_entries (lootdrop_id, item_id, item_charges, equip_item, chance, disabled_chance,
                              trivial_min_level, trivial_max_level, multiplier)
    VALUES (200030, 147921, 1, 0, 1.5, 0, 0, 0, 1);

-- ⚠️ min_level 10 so the optimal farm is not level-1 trash. rare/raid 0 = every mob qualifies.
INSERT INTO global_loot (id, description, loottable_id, enabled, min_level, max_level, rare, raid)
    VALUES (100, 'AoTv4 Ink of the Lost', 200030, 1, 10, 0, 0, 0);

SELECT (SELECT COUNT(*) FROM items WHERE id IN (147920,147921))            AS items_created,
       (SELECT COUNT(*) FROM global_loot WHERE id=100 AND enabled=1)       AS global_drop_enabled,
       (SELECT chance FROM lootdrop_entries WHERE lootdrop_id=200030)      AS ink_chance_percent;
