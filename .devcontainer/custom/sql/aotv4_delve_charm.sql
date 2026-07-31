-- AoTv4 -- the Delver's Sigil: a charm that evolves on DELVE SCORE                   2026-07-30
-- ============================================================================================
-- Ten item rows forming one native evolving chain. The delve system feeds it: on a cleared run,
-- lua_modules/aotv4_dungeon.lua adds that run's SCORE to the charm's evolve amount.
--
-- ⚠️⚠️ `required_amount` IS A PER LEVEL COST. `current_amount` RESETS TO ZERO ON EVERY EVOLVE, and
-- progression is
--     CalculateProgression = current_amount / required_amount(OF THE CURRENT ITEM) * 100
-- (common/evolving_items.cpp:61), so each row states only what THAT level costs on its own.
--
-- ⚠️⚠️ AN EARLIER VERSION OF THIS COMMENT SAID THE EXACT OPPOSITE -- "a cumulative lifetime total,
-- never reset, writing per level costs would make every level after the first nearly instant" -- and
-- it was wrong. Do not restore it; it is worth knowing WHY, because the reset is not obvious from
-- reading DoEvolveCheckProgression:
--   1. it never transfers the amount. `SetEvolveCurrentAmount` appears ONLY in the XP transfer
--      window paths (client_evolving_items.cpp:469/471/517/519), never in the evolve path.
--   2. the swap is RemoveItemBySerialNumber -> DeleteItemInInventory, which SOFT DELETES the
--      character_evolving_items row outright (zone/inventory.cpp:988).
--   3. the replacement comes from database.CreateItem(new_item_id), which is a fresh instance at 0,
--      and DoLootChecks then inserts a NEW row for it.
-- ⚠️ Level 1 cannot tell the two readings apart -- they are the same number there -- so a single
-- observed run at level 1 CONFIRMS NOTHING about which is right. That is what made this survive.
--
-- ⚠️⚠️ THE INCREMENT IS `24000 * n^2` AND IT IS DERIVED, NOT PICKED. It is the same SHAPE as the
-- scoring function, and that is the whole point -- see the failure it replaced, below.
--
-- The derivation, so it can be recomputed if aotv4_dungeon_scale.M.kill_value ever changes:
--   a kill is worth eff^2, a named 3x  (lua_modules/aotv4_dungeon_scale.lua)
--   an observed run is ~41 kills of which ~4 are named  ->  37*L^2 + 4*3*L^2 = 49*L^2 per run
--   a player working on charm level n is around rung 7n ->  49*(7n)^2 = 2401*n^2 per run
--   ten runs per charm level                            ->  24010*n^2, rounded to 24000*n^2
-- Increments: 24000, 96000, 216000, 384000, 600000, 864000, 1176000, 1536000, 1944000.
--
--     pace              runs to fully evolve
--     climbing with the ladder        ~85
--     parked at rung 70                ~29
--     farming rung 1              ~139,000
-- Anti farming is now carried by the SCORING function rather than by this curve: a rung 1 kill is
-- worth 1 and a rung 70 kill 4900, so the bottom of the ladder is 4,900x less efficient per kill.
--
-- ⚠️⚠️ THE CURVE USED TO DOUBLE EACH LEVEL (150, 450, 1050 ... 76650) AND THAT WAS THE BUG. Doubling
-- is EXPONENTIAL in the charm level while run score is QUADRATIC in the rung, and the two were never
-- checked against each other -- so the requirement started far behind and the doubling never caught
-- up. Modelled against a real run, charm levels 2 through 5 each took UNDER ONE RUN, and the full
-- chain was 18 runs climbing / 8 runs at cap against a stated design of ten runs per level.
-- ⚠️ It read as correct because the numbers LOOK punishing in isolation: 76,650 to finish is a big
-- number, and "it doubles every level" sounds like the anti farming mechanism it was described as.
-- Neither survives being divided by what a run is actually worth. If this ladder is retuned again,
-- retune it against M.kill_value the same way -- an absolute number here means nothing on its own.
--
-- ⚠️ Progress only accrues while the charm is EQUIPPED, and there is a 30 second arming delay after
-- equipping (rule EvolvingItems:DelayUponEquipping). Both are native behaviour, not ours.
-- ⚠️ The evolve STEP itself runs in Client::ProcessEvolvingItem, called from exp.cpp on XP gain --
-- adding the amount does not immediately swap the item. Delving grants XP, so it lands within a kill
-- or two of the run ending. That is native and not worth fighting.
--
-- ⚠️ `items` IS IN SHARED MEMORY: world down, ./shared_memory, restart. A zone restart is NOT enough.
--    `items_evolving_details` is loaded by the EvolvingItemsManager at boot, so it needs the same.
-- ⚠️ Re-runnable; the DELETE names only the ids this script creates (147500-147509, evo id 2000).
-- ============================================================================================

-- ---------------------------------------------------------------- the ten item rows
-- ⚠️ Cloned VIA A TEMP TABLE from 8258 Wolf Fang Necklace so all ~250 item columns stay valid.
-- Never hand-list item columns. 8258 is the template because it ALREADY carries the click we want:
-- spell 278 Spirit of Wolf on clicktype 5 (usable by any class at any level), which is the
-- run speed click that was asked for.
DROP TEMPORARY TABLE IF EXISTS tmp_sigil;
CREATE TEMPORARY TABLE tmp_sigil AS SELECT * FROM items WHERE id = 8258;

DELETE FROM items WHERE id BETWEEN 147500 AND 147509;

-- level 1
UPDATE tmp_sigil SET
    id = 147500, Name = 'Cracked Delver''s Sigil', evolvinglevel = 1,
    -- ⚠️ slots = 1 is the CHARM slot (bit 0). The template is a neck item; without this the sigil
    -- would occupy the wrong slot and the charm slot would stay empty.
    slots = 1,
    evoitem = 1, evoid = 2000, evomax = 10,
    -- ⚠️ One shared loregroup across all ten so a character cannot carry two sigils at once, and so
    -- an evolved one cannot sit beside a fresh one. (loregroup is the numeric group; `lore` is text.)
    loregroup = 20000,
    nodrop = 0, norent = 1, price = 0, sellrate = 0,
    astr=1, asta=1, aagi=1, adex=1, awis=1, aint=1, acha=1,
    mr=1, fr=1, cr=1, dr=1, pr=1, ac=1, hp=10, mana=10, endur=10,
    clickeffect = 278, clicktype = 5, casttime = 2000, recastdelay = 0, recasttype = -1,
    -- ⚠️⚠️ BOTH OF THESE ARE INHERITED FROM THE TEMPLATE AND BOTH MUST BE OVERRIDDEN. Wolf Fang
    -- Necklace ships `maxcharges = 5` and `clicklevel2 = 29`, which on this charm means it runs dry
    -- after five clicks AND cannot be clicked at all by the level 1 character who is handed it for
    -- finishing their first delve -- so the reward would arrive unusable and then expire.
    -- ⚠️ maxcharges = -1 is UNLIMITED (0 is not; 0 means no charges at all).
    -- ⚠️ clicklevel2 is the level gate on the CLICK. clicklevel is a separate field and is already 0.
    maxcharges = -1, clicklevel2 = 1, clicklevel = 0,
    -- ⚠️ reclevel/reqlevel stay 0: the sigil is meant to be worn from level 1 all the way up, and a
    -- required level would also block the very first grant.
    reclevel = 0, reqlevel = 0;
INSERT INTO items SELECT * FROM tmp_sigil;

UPDATE tmp_sigil SET id=147501, Name='Chipped Delver''s Sigil',      evolvinglevel=2,
    astr=2,asta=2,aagi=2,adex=2,awis=2,aint=2,acha=2, mr=2,fr=2,cr=2,dr=2,pr=2, ac=2, hp=20,mana=20,endur=20;
INSERT INTO items SELECT * FROM tmp_sigil;
UPDATE tmp_sigil SET id=147502, Name='Worn Delver''s Sigil',         evolvinglevel=3,
    astr=3,asta=3,aagi=3,adex=3,awis=3,aint=3,acha=3, mr=3,fr=3,cr=3,dr=3,pr=3, ac=3, hp=30,mana=30,endur=30;
INSERT INTO items SELECT * FROM tmp_sigil;
UPDATE tmp_sigil SET id=147503, Name='Polished Delver''s Sigil',     evolvinglevel=4,
    astr=4,asta=4,aagi=4,adex=4,awis=4,aint=4,acha=4, mr=4,fr=4,cr=4,dr=4,pr=4, ac=4, hp=40,mana=40,endur=40;
INSERT INTO items SELECT * FROM tmp_sigil;
UPDATE tmp_sigil SET id=147504, Name='Gleaming Delver''s Sigil',     evolvinglevel=5,
    astr=5,asta=5,aagi=5,adex=5,awis=5,aint=5,acha=5, mr=5,fr=5,cr=5,dr=5,pr=5, ac=5, hp=50,mana=50,endur=50;
INSERT INTO items SELECT * FROM tmp_sigil;
UPDATE tmp_sigil SET id=147505, Name='Radiant Delver''s Sigil',      evolvinglevel=6,
    astr=6,asta=6,aagi=6,adex=6,awis=6,aint=6,acha=6, mr=6,fr=6,cr=6,dr=6,pr=6, ac=6, hp=60,mana=60,endur=60;
INSERT INTO items SELECT * FROM tmp_sigil;
UPDATE tmp_sigil SET id=147506, Name='Resplendent Delver''s Sigil',  evolvinglevel=7,
    astr=7,asta=7,aagi=7,adex=7,awis=7,aint=7,acha=7, mr=7,fr=7,cr=7,dr=7,pr=7, ac=7, hp=70,mana=70,endur=70;
INSERT INTO items SELECT * FROM tmp_sigil;
UPDATE tmp_sigil SET id=147507, Name='Ascendant Delver''s Sigil',    evolvinglevel=8,
    astr=8,asta=8,aagi=8,adex=8,awis=8,aint=8,acha=8, mr=8,fr=8,cr=8,dr=8,pr=8, ac=8, hp=80,mana=80,endur=80;
INSERT INTO items SELECT * FROM tmp_sigil;
UPDATE tmp_sigil SET id=147508, Name='Transcendent Delver''s Sigil', evolvinglevel=9,
    astr=9,asta=9,aagi=9,adex=9,awis=9,aint=9,acha=9, mr=9,fr=9,cr=9,dr=9,pr=9, ac=9, hp=90,mana=90,endur=90;
INSERT INTO items SELECT * FROM tmp_sigil;
UPDATE tmp_sigil SET id=147509, Name='Eternal Delver''s Sigil',      evolvinglevel=10,
    astr=10,asta=10,aagi=10,adex=10,awis=10,aint=10,acha=10, mr=10,fr=10,cr=10,dr=10,pr=10, ac=10, hp=100,mana=100,endur=100;
INSERT INTO items SELECT * FROM tmp_sigil;

DROP TEMPORARY TABLE tmp_sigil;

-- ---------------------------------------------------------------- the evolve requirements
-- ⚠️ type = 1 is AMOUNT_OF_EXP (EvolvingItems::Types::AMOUNT_OF_EXP, common/evolving_items.h). We
-- reuse that type because it is the only one whose counter can be driven from Lua -- but NOTHING
-- about this charm uses experience: aotv4_dungeon.lua adds the run SCORE via AddEvolveAmount.
-- ⚠️ sub_type 0 is ALL_EXP. Leave it: a narrower sub type would make the native XP path contribute
-- as well, which would let ordinary levelling evolve a charm that is supposed to track delving.
-- ⚠️ Level 10 gets a row too. It is never consumed (DoEvolveCheckProgression stops at evomax) but
-- its absence makes CalculateProgression divide by a missing cache entry and report 0 percent, so
-- the item would show an empty progress bar at max instead of a full one.
DELETE FROM items_evolving_details WHERE item_evo_id = 2000;
INSERT INTO items_evolving_details (item_evo_id, item_evolve_level, item_id, type, sub_type, required_amount) VALUES
-- ⚠️ These ARE the increments (per level costs), not running totals -- see the note above.
    (2000,  1, 147500, 1, '0',    24000),
    (2000,  2, 147501, 1, '0',    96000),
    (2000,  3, 147502, 1, '0',   216000),
    (2000,  4, 147503, 1, '0',   384000),
    (2000,  5, 147504, 1, '0',   600000),
    (2000,  6, 147505, 1, '0',   864000),
    (2000,  7, 147506, 1, '0',  1176000),
    (2000,  8, 147507, 1, '0',  1536000),
    (2000,  9, 147508, 1, '0',  1944000),
-- ⚠️ Level 10 is never consumed (DoEvolveCheckProgression stops at evomax) but the row must exist or
-- CalculateProgression divides by a missing cache entry and reports 0 percent forever. It mirrors
-- level 9 so a maxed sigil that keeps being fed reads as a full bar rather than an empty one.
    (2000, 10, 147509, 1, '0',  1944000);

-- ---------------------------------------------------------------- verify
SELECT 'sigil items'   AS what, COUNT(*) n FROM items WHERE id BETWEEN 147500 AND 147509
UNION ALL
SELECT 'evolve rows',  COUNT(*)           FROM items_evolving_details WHERE item_evo_id = 2000
UNION ALL
SELECT 'charm slot',   COUNT(*)           FROM items WHERE id BETWEEN 147500 AND 147509 AND slots = 1
UNION ALL
SELECT 'sow click',    COUNT(*)           FROM items WHERE id BETWEEN 147500 AND 147509 AND clickeffect = 278;
-- expected: 10 / 10 / 10 / 10
