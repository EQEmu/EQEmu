-- =============================================================================================
-- AoTv4 -- Delve augments: three tiers, combined in the Refining Crucible
--                                                              GENERATED, DO NOT HAND EDIT
--   source: .devcontainer/custom/tools/gen_delve_augs.pl   (fixed seed 20260731)
--
-- 316 item rows, ids 147600..147915, in three CONTIGUOUS blocks:
--   tier 1  147600..147615   weight 2   single stat, one variant per stat line
--   tier 2  147616..147715   weight 4   randomly rolled over 1-2 lines
--   tier 3  147716..147915   weight 8   randomly rolled over 2-3 lines
--
-- ⚠️⚠️ THOSE BLOCKS ARE LOAD BEARING AND ARE MIRRORED IN C++. zone/aotv4_tiers.h holds the same
--    boundaries so AoTv4RefineCombine can tell an augment's tier from its id and pick a random
--    member of the next tier's block. CHANGE THE COUNTS HERE AND THAT HEADER MUST CHANGE TOO --
--    nothing will fail to compile, the crucible will just start producing the wrong items.
--
-- ⚠️ `evoitem` is 0: these are ORDINARY items, not native evolving items. See the script header for
--    why that is deliberate and must not be "restored".
-- ⚠️ `items` IS IN SHARED MEMORY: world down, ./shared_memory, restart. A zone restart is NOT enough.
-- ⚠️ Cloned via a temp table from 71402 so all ~250 item columns stay valid -- never hand list
--    item columns. Every row zeroes every stat column this generator knows about before setting its
--    own, so no row can inherit a stat from the template or from the row before it.
-- ⚠️ nodrop/norent are INVERTED in this schema: nodrop = 0 is No Drop, norent = 1 is permanent.
--    (Cloth Cap and Rusty Long Sword are both nodrop 1 / norent 1.)
-- =============================================================================================

DROP TEMPORARY TABLE IF EXISTS tmp_aug;
CREATE TEMPORARY TABLE tmp_aug AS SELECT * FROM items WHERE id = 71402;

-- ⚠️ Clears the WHOLE reserved band, not just the rows written below: earlier versions of this set
-- had a different layout reaching 148007, and deleting only the current range would leave their tail
-- behind as orphaned items.
DELETE FROM items WHERE id BETWEEN 147600 AND 148199;
-- the old evolving version of this set had rows here; they are no longer used by anything
DELETE FROM items_evolving_details WHERE item_evo_id IN (2001, 2002, 2003);

-- fields shared by every row in the set
UPDATE tmp_aug SET
    augtype = 255, augrestrict = 0, augdistiller = 0,
    icon = 1439, itemtype = 54, slots = 2097150,
    classes = 65535, races = 65535, loregroup = 0, magic = 1,
    nodrop = 0, norent = 1, price = 0, sellrate = 0, ldonsellbackrate = 0,
    reclevel = 0, reqlevel = 0, stacksize = 1,
    lore = '', evoitem = 0, evoid = 0, evolvinglevel = 0, evomax = 0;
-- T1  weight 2  (astr 2)
UPDATE tmp_aug SET id = 147600, Name = 'Delver''s Shard of Might', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, astr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T1  weight 2  (asta 2)
UPDATE tmp_aug SET id = 147601, Name = 'Delver''s Shard of Vigor', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T1  weight 2  (aagi 2)
UPDATE tmp_aug SET id = 147602, Name = 'Delver''s Shard of Grace', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T1  weight 2  (adex 2)
UPDATE tmp_aug SET id = 147603, Name = 'Delver''s Shard of Precision', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T1  weight 2  (aint 2)
UPDATE tmp_aug SET id = 147604, Name = 'Delver''s Shard of Intellect', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aint = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T1  weight 2  (awis 2)
UPDATE tmp_aug SET id = 147605, Name = 'Delver''s Shard of Insight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, awis = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T1  weight 2  (acha 2)
UPDATE tmp_aug SET id = 147606, Name = 'Delver''s Shard of Presence', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T1  weight 2  (ac 2)
UPDATE tmp_aug SET id = 147607, Name = 'Delver''s Shard of the Bulwark', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T1  weight 2  (hp 20)
UPDATE tmp_aug SET id = 147608, Name = 'Delver''s Shard of Life', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, hp = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T1  weight 2  (mana 20)
UPDATE tmp_aug SET id = 147609, Name = 'Delver''s Shard of the Mind', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, mana = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T1  weight 2  (endur 20)
UPDATE tmp_aug SET id = 147610, Name = 'Delver''s Shard of Stamina', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, endur = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T1  weight 2  (mr 2)
UPDATE tmp_aug SET id = 147611, Name = 'Delver''s Shard of Warding', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, mr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T1  weight 2  (fr 2)
UPDATE tmp_aug SET id = 147612, Name = 'Delver''s Shard of Embers', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, fr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T1  weight 2  (cr 2)
UPDATE tmp_aug SET id = 147613, Name = 'Delver''s Shard of Frost', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, cr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T1  weight 2  (dr 2)
UPDATE tmp_aug SET id = 147614, Name = 'Delver''s Shard of the Blight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, dr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T1  weight 2  (pr 2)
UPDATE tmp_aug SET id = 147615, Name = 'Delver''s Shard of Venom', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, pr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (mana 30, pr 1)
UPDATE tmp_aug SET id = 147616, Name = 'Delver''s Gem of the Mind', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, mana = 30, pr = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (aint 2, mana 20)
UPDATE tmp_aug SET id = 147617, Name = 'Delver''s Gem of Intellect', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aint = 2, mana = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (aagi 4)
UPDATE tmp_aug SET id = 147618, Name = 'Delver''s Gem of Grace', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (ac 3, aint 1)
UPDATE tmp_aug SET id = 147619, Name = 'Delver''s Gem of the Bulwark', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 3, aint = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (awis 4)
UPDATE tmp_aug SET id = 147620, Name = 'Delver''s Gem of Insight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, awis = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (astr 4)
UPDATE tmp_aug SET id = 147621, Name = 'Delver''s Gem of Might', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, astr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (aagi 2, aint 2)
UPDATE tmp_aug SET id = 147622, Name = 'Delver''s Gem of Grace and Intellect', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 2, aint = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (cr 4)
UPDATE tmp_aug SET id = 147623, Name = 'Delver''s Gem of Frost', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, cr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (endur 30, fr 1)
UPDATE tmp_aug SET id = 147624, Name = 'Delver''s Gem of Stamina', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, endur = 30, fr = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (astr 2, endur 20)
UPDATE tmp_aug SET id = 147625, Name = 'Delver''s Gem of Stamina and Might', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, astr = 2, endur = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (ac 4)
UPDATE tmp_aug SET id = 147626, Name = 'Delver''s Gem of the Bulwark II', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (aagi 3, asta 1)
UPDATE tmp_aug SET id = 147627, Name = 'Delver''s Gem of Grace and Vigor', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 3, asta = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (acha 2, pr 2)
UPDATE tmp_aug SET id = 147628, Name = 'Delver''s Gem of Presence', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 2, pr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (aint 4)
UPDATE tmp_aug SET id = 147629, Name = 'Delver''s Gem of Intellect II', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aint = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (aint 3, hp 10)
UPDATE tmp_aug SET id = 147630, Name = 'Delver''s Gem of Intellect and Life', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aint = 3, hp = 10;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (endur 40)
UPDATE tmp_aug SET id = 147631, Name = 'Delver''s Gem of Stamina II', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, endur = 40;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (adex 3, fr 1)
UPDATE tmp_aug SET id = 147632, Name = 'Delver''s Gem of Precision', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 3, fr = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (aagi 2, asta 2)
UPDATE tmp_aug SET id = 147633, Name = 'Delver''s Gem of Grace II', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 2, asta = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (aagi 1, awis 3)
UPDATE tmp_aug SET id = 147634, Name = 'Delver''s Gem of Insight and Grace', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 1, awis = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (astr 3, dr 1)
UPDATE tmp_aug SET id = 147635, Name = 'Delver''s Gem of Might and the Blight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, astr = 3, dr = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (cr 3, endur 10)
UPDATE tmp_aug SET id = 147636, Name = 'Delver''s Gem of Frost and Stamina', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, cr = 3, endur = 10;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (pr 4)
UPDATE tmp_aug SET id = 147637, Name = 'Delver''s Gem of Venom', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, pr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (adex 1, mr 3)
UPDATE tmp_aug SET id = 147638, Name = 'Delver''s Gem of Warding', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 1, mr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (awis 2, dr 2)
UPDATE tmp_aug SET id = 147639, Name = 'Delver''s Gem of the Blight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, awis = 2, dr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (adex 4)
UPDATE tmp_aug SET id = 147640, Name = 'Delver''s Gem of Precision II', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (mana 40)
UPDATE tmp_aug SET id = 147641, Name = 'Delver''s Gem of the Mind II', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, mana = 40;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (asta 4)
UPDATE tmp_aug SET id = 147642, Name = 'Delver''s Gem of Vigor', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (fr 4)
UPDATE tmp_aug SET id = 147643, Name = 'Delver''s Gem of Embers', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, fr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (fr 3, mr 1)
UPDATE tmp_aug SET id = 147644, Name = 'Delver''s Gem of Embers and Warding', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, fr = 3, mr = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (aint 1, pr 3)
UPDATE tmp_aug SET id = 147645, Name = 'Delver''s Gem of Venom and Intellect', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aint = 1, pr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (adex 2, asta 2)
UPDATE tmp_aug SET id = 147646, Name = 'Delver''s Gem of Precision and Vigor', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 2, asta = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (acha 1, dr 3)
UPDATE tmp_aug SET id = 147647, Name = 'Delver''s Gem of the Blight and Presence', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 1, dr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (aint 2, dr 2)
UPDATE tmp_aug SET id = 147648, Name = 'Delver''s Gem of the Blight and Intellect', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aint = 2, dr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (astr 2, dr 2)
UPDATE tmp_aug SET id = 147649, Name = 'Delver''s Gem of the Blight and Might', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, astr = 2, dr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (cr 2, fr 2)
UPDATE tmp_aug SET id = 147650, Name = 'Delver''s Gem of Frost and Embers', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, cr = 2, fr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (ac 2, aint 2)
UPDATE tmp_aug SET id = 147651, Name = 'Delver''s Gem of the Bulwark and Intellect', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 2, aint = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (mr 3, pr 1)
UPDATE tmp_aug SET id = 147652, Name = 'Delver''s Gem of Warding and Venom', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, mr = 3, pr = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (ac 1, astr 3)
UPDATE tmp_aug SET id = 147653, Name = 'Delver''s Gem of Might and the Bulwark', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 1, astr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (aagi 3, mr 1)
UPDATE tmp_aug SET id = 147654, Name = 'Delver''s Gem of Grace and Warding', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 3, mr = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (adex 2, mr 2)
UPDATE tmp_aug SET id = 147655, Name = 'Delver''s Gem of Precision and Warding', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 2, mr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (asta 3, hp 10)
UPDATE tmp_aug SET id = 147656, Name = 'Delver''s Gem of Vigor and Life', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 3, hp = 10;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (dr 4)
UPDATE tmp_aug SET id = 147657, Name = 'Delver''s Gem of the Blight II', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, dr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (acha 2, dr 2)
UPDATE tmp_aug SET id = 147658, Name = 'Delver''s Gem of Presence and the Blight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 2, dr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (hp 40)
UPDATE tmp_aug SET id = 147659, Name = 'Delver''s Gem of Life', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, hp = 40;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (ac 1, awis 3)
UPDATE tmp_aug SET id = 147660, Name = 'Delver''s Gem of Insight and the Bulwark', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 1, awis = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (ac 2, hp 20)
UPDATE tmp_aug SET id = 147661, Name = 'Delver''s Gem of the Bulwark and Life', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 2, hp = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (dr 1, pr 3)
UPDATE tmp_aug SET id = 147662, Name = 'Delver''s Gem of Venom and the Blight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, dr = 1, pr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (mr 4)
UPDATE tmp_aug SET id = 147663, Name = 'Delver''s Gem of Warding II', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, mr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (cr 2, hp 20)
UPDATE tmp_aug SET id = 147664, Name = 'Delver''s Gem of Frost and Life', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, cr = 2, hp = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (dr 1, mana 30)
UPDATE tmp_aug SET id = 147665, Name = 'Delver''s Gem of the Mind and the Blight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, dr = 1, mana = 30;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (acha 4)
UPDATE tmp_aug SET id = 147666, Name = 'Delver''s Gem of Presence II', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (hp 30, mr 1)
UPDATE tmp_aug SET id = 147667, Name = 'Delver''s Gem of Life and Warding', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, hp = 30, mr = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (awis 1, hp 30)
UPDATE tmp_aug SET id = 147668, Name = 'Delver''s Gem of Life and Insight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, awis = 1, hp = 30;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (dr 2, mr 2)
UPDATE tmp_aug SET id = 147669, Name = 'Delver''s Gem of the Blight and Warding', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, dr = 2, mr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (asta 3, mana 10)
UPDATE tmp_aug SET id = 147670, Name = 'Delver''s Gem of Vigor and the Mind', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 3, mana = 10;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (cr 3, pr 1)
UPDATE tmp_aug SET id = 147671, Name = 'Delver''s Gem of Frost and Venom', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, cr = 3, pr = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (astr 3, pr 1)
UPDATE tmp_aug SET id = 147672, Name = 'Delver''s Gem of Might and Venom', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, astr = 3, pr = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (adex 3, cr 1)
UPDATE tmp_aug SET id = 147673, Name = 'Delver''s Gem of Precision and Frost', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 3, cr = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (aint 2, mr 2)
UPDATE tmp_aug SET id = 147674, Name = 'Delver''s Gem of Intellect and Warding', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aint = 2, mr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (hp 20, mana 20)
UPDATE tmp_aug SET id = 147675, Name = 'Delver''s Gem of Life and the Mind', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, hp = 20, mana = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (ac 2, mana 20)
UPDATE tmp_aug SET id = 147676, Name = 'Delver''s Gem of the Bulwark and the Mind', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 2, mana = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (astr 2, fr 2)
UPDATE tmp_aug SET id = 147677, Name = 'Delver''s Gem of Embers and Might', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, astr = 2, fr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (aint 2, pr 2)
UPDATE tmp_aug SET id = 147678, Name = 'Delver''s Gem of Intellect and Venom', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aint = 2, pr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (dr 2, hp 20)
UPDATE tmp_aug SET id = 147679, Name = 'Delver''s Gem of the Blight and Life', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, dr = 2, hp = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (aagi 1, aint 3)
UPDATE tmp_aug SET id = 147680, Name = 'Delver''s Gem of Intellect and Grace', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 1, aint = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (awis 2, endur 20)
UPDATE tmp_aug SET id = 147681, Name = 'Delver''s Gem of Stamina and Insight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, awis = 2, endur = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (asta 2, dr 2)
UPDATE tmp_aug SET id = 147682, Name = 'Delver''s Gem of the Blight and Vigor', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 2, dr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (ac 3, mr 1)
UPDATE tmp_aug SET id = 147683, Name = 'Delver''s Gem of the Bulwark and Warding', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 3, mr = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (adex 2, cr 2)
UPDATE tmp_aug SET id = 147684, Name = 'Delver''s Gem of Frost and Precision', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 2, cr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (aagi 2, pr 2)
UPDATE tmp_aug SET id = 147685, Name = 'Delver''s Gem of Grace and Venom', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 2, pr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (asta 2, mana 20)
UPDATE tmp_aug SET id = 147686, Name = 'Delver''s Gem of the Mind and Vigor', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 2, mana = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (ac 2, adex 2)
UPDATE tmp_aug SET id = 147687, Name = 'Delver''s Gem of the Bulwark and Precision', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 2, adex = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (awis 3, cr 1)
UPDATE tmp_aug SET id = 147688, Name = 'Delver''s Gem of Insight and Frost', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, awis = 3, cr = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (aagi 2, dr 2)
UPDATE tmp_aug SET id = 147689, Name = 'Delver''s Gem of Grace and the Blight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 2, dr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (cr 1, dr 3)
UPDATE tmp_aug SET id = 147690, Name = 'Delver''s Gem of the Blight and Frost', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, cr = 1, dr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (adex 2, pr 2)
UPDATE tmp_aug SET id = 147691, Name = 'Delver''s Gem of Precision and Venom', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 2, pr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (acha 2, aint 2)
UPDATE tmp_aug SET id = 147692, Name = 'Delver''s Gem of Presence and Intellect', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 2, aint = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (adex 1, fr 3)
UPDATE tmp_aug SET id = 147693, Name = 'Delver''s Gem of Embers and Precision', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 1, fr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (endur 20, hp 20)
UPDATE tmp_aug SET id = 147694, Name = 'Delver''s Gem of Stamina and Life', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, endur = 20, hp = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (dr 2, fr 2)
UPDATE tmp_aug SET id = 147695, Name = 'Delver''s Gem of the Blight and Embers', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, dr = 2, fr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (cr 2, endur 20)
UPDATE tmp_aug SET id = 147696, Name = 'Delver''s Gem of Frost II', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, cr = 2, endur = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (astr 3, cr 1)
UPDATE tmp_aug SET id = 147697, Name = 'Delver''s Gem of Might and Frost', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, astr = 3, cr = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (aagi 3, fr 1)
UPDATE tmp_aug SET id = 147698, Name = 'Delver''s Gem of Grace and Embers', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 3, fr = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (ac 1, fr 3)
UPDATE tmp_aug SET id = 147699, Name = 'Delver''s Gem of Embers and the Bulwark', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 1, fr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (acha 1, mr 3)
UPDATE tmp_aug SET id = 147700, Name = 'Delver''s Gem of Warding and Presence', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 1, mr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (awis 1, fr 3)
UPDATE tmp_aug SET id = 147701, Name = 'Delver''s Gem of Embers and Insight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, awis = 1, fr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (mana 30, mr 1)
UPDATE tmp_aug SET id = 147702, Name = 'Delver''s Gem of the Mind and Warding', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, mana = 30, mr = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (asta 2, fr 2)
UPDATE tmp_aug SET id = 147703, Name = 'Delver''s Gem of Embers and Vigor', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 2, fr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (aint 2, cr 2)
UPDATE tmp_aug SET id = 147704, Name = 'Delver''s Gem of Frost and Intellect', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aint = 2, cr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (ac 2, acha 2)
UPDATE tmp_aug SET id = 147705, Name = 'Delver''s Gem of the Bulwark and Presence', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 2, acha = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (asta 2, endur 20)
UPDATE tmp_aug SET id = 147706, Name = 'Delver''s Gem of Stamina and Vigor', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 2, endur = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (endur 30, mana 10)
UPDATE tmp_aug SET id = 147707, Name = 'Delver''s Gem of Stamina and the Mind', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, endur = 30, mana = 10;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (asta 3, fr 1)
UPDATE tmp_aug SET id = 147708, Name = 'Delver''s Gem of Vigor and Embers', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 3, fr = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (awis 3, hp 10)
UPDATE tmp_aug SET id = 147709, Name = 'Delver''s Gem of Insight and Life', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, awis = 3, hp = 10;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (ac 3, endur 10)
UPDATE tmp_aug SET id = 147710, Name = 'Delver''s Gem of the Bulwark and Stamina', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 3, endur = 10;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (mr 1, pr 3)
UPDATE tmp_aug SET id = 147711, Name = 'Delver''s Gem of Venom and Warding', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, mr = 1, pr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (awis 2, pr 2)
UPDATE tmp_aug SET id = 147712, Name = 'Delver''s Gem of Venom and Insight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, awis = 2, pr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (dr 2, pr 2)
UPDATE tmp_aug SET id = 147713, Name = 'Delver''s Gem of the Blight and Venom', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, dr = 2, pr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (asta 2, pr 2)
UPDATE tmp_aug SET id = 147714, Name = 'Delver''s Gem of Venom and Vigor', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 2, pr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T2  weight 4  (ac 2, asta 2)
UPDATE tmp_aug SET id = 147715, Name = 'Delver''s Gem of the Bulwark and Vigor', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 2, asta = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 3, acha 2, pr 3)
UPDATE tmp_aug SET id = 147716, Name = 'Delver''s Prism of Grace', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 3, acha = 2, pr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 3, endur 20, hp 30)
UPDATE tmp_aug SET id = 147717, Name = 'Delver''s Prism of Grace and Life', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 3, endur = 20, hp = 30;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (hp 40, mana 40)
UPDATE tmp_aug SET id = 147718, Name = 'Delver''s Prism of Life', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, hp = 40, mana = 40;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (acha 2, adex 4, astr 2)
UPDATE tmp_aug SET id = 147719, Name = 'Delver''s Prism of Precision', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 2, adex = 4, astr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (endur 40, hp 20, mana 20)
UPDATE tmp_aug SET id = 147720, Name = 'Delver''s Prism of Stamina', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, endur = 40, hp = 20, mana = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (acha 5, cr 3)
UPDATE tmp_aug SET id = 147721, Name = 'Delver''s Prism of Presence', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 5, cr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (cr 3, dr 2, pr 3)
UPDATE tmp_aug SET id = 147722, Name = 'Delver''s Prism of Frost', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, cr = 3, dr = 2, pr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (adex 4, astr 2, endur 20)
UPDATE tmp_aug SET id = 147723, Name = 'Delver''s Prism of Precision and Stamina', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 4, astr = 2, endur = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (asta 4, cr 2, endur 20)
UPDATE tmp_aug SET id = 147724, Name = 'Delver''s Prism of Vigor', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 4, cr = 2, endur = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (acha 1, adex 4, hp 30)
UPDATE tmp_aug SET id = 147725, Name = 'Delver''s Prism of Precision and Life', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 1, adex = 4, hp = 30;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (cr 5, fr 3)
UPDATE tmp_aug SET id = 147726, Name = 'Delver''s Prism of Frost and Embers', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, cr = 5, fr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aint 2, heroic_str 1, mr 3)
UPDATE tmp_aug SET id = 147727, Name = 'Delver''s Prism of Warding', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aint = 2, heroic_str = 1, mr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (asta 4, fr 4)
UPDATE tmp_aug SET id = 147728, Name = 'Delver''s Prism of Embers', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 4, fr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (dr 5, heroic_int 1)
UPDATE tmp_aug SET id = 147729, Name = 'Delver''s Prism of the Blight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, dr = 5, heroic_int = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (endur 20, pr 6)
UPDATE tmp_aug SET id = 147730, Name = 'Delver''s Prism of Venom', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, endur = 20, pr = 6;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (endur 20, mana 20, mr 4)
UPDATE tmp_aug SET id = 147731, Name = 'Delver''s Prism of Warding and Stamina', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, endur = 20, mana = 20, mr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 4, aint 2, cr 2)
UPDATE tmp_aug SET id = 147732, Name = 'Delver''s Prism of the Bulwark', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 4, aint = 2, cr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aint 3, mana 10, mr 4)
UPDATE tmp_aug SET id = 147733, Name = 'Delver''s Prism of Warding and Intellect', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aint = 3, mana = 10, mr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (heroic_dr 1, hp 50)
UPDATE tmp_aug SET id = 147734, Name = 'Delver''s Prism of Life and the Blight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, heroic_dr = 1, hp = 50;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aint 5, heroic_dex 1)
UPDATE tmp_aug SET id = 147735, Name = 'Delver''s Prism of Intellect', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aint = 5, heroic_dex = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (adex 4, cr 2, hp 20)
UPDATE tmp_aug SET id = 147736, Name = 'Delver''s Prism of Precision and Frost', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 4, cr = 2, hp = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (acha 2, asta 2, astr 4)
UPDATE tmp_aug SET id = 147737, Name = 'Delver''s Prism of Might', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 2, asta = 2, astr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (asta 4, pr 4)
UPDATE tmp_aug SET id = 147738, Name = 'Delver''s Prism of Venom and Vigor', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 4, pr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (adex 3, cr 2, hp 30)
UPDATE tmp_aug SET id = 147739, Name = 'Delver''s Prism of Precision, Life and Frost', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 3, cr = 2, hp = 30;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (astr 4, dr 1, pr 3)
UPDATE tmp_aug SET id = 147740, Name = 'Delver''s Prism of Might and Venom', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, astr = 4, dr = 1, pr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 5, hp 30)
UPDATE tmp_aug SET id = 147741, Name = 'Delver''s Prism of the Bulwark and Life', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 5, hp = 30;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aint 3, asta 5)
UPDATE tmp_aug SET id = 147742, Name = 'Delver''s Prism of Vigor and Intellect', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aint = 3, asta = 5;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 4, ac 2, adex 2)
UPDATE tmp_aug SET id = 147743, Name = 'Delver''s Prism of Grace and the Bulwark', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 4, ac = 2, adex = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (adex 4, fr 4)
UPDATE tmp_aug SET id = 147744, Name = 'Delver''s Prism of Precision and Embers', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 4, fr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (asta 2, awis 3, mr 3)
UPDATE tmp_aug SET id = 147745, Name = 'Delver''s Prism of Warding and Insight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 2, awis = 3, mr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (awis 3, dr 2, pr 3)
UPDATE tmp_aug SET id = 147746, Name = 'Delver''s Prism of Venom and Insight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, awis = 3, dr = 2, pr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aint 3, astr 2, heroic_str 1)
UPDATE tmp_aug SET id = 147747, Name = 'Delver''s Prism of Intellect and Might', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aint = 3, astr = 2, heroic_str = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (awis 4, fr 4)
UPDATE tmp_aug SET id = 147748, Name = 'Delver''s Prism of Embers and Insight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, awis = 4, fr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aint 3, asta 4, awis 1)
UPDATE tmp_aug SET id = 147749, Name = 'Delver''s Prism of Vigor, Intellect and Insight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aint = 3, asta = 4, awis = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 3, pr 5)
UPDATE tmp_aug SET id = 147750, Name = 'Delver''s Prism of Venom and Grace', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 3, pr = 5;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (asta 3, endur 30, mr 2)
UPDATE tmp_aug SET id = 147751, Name = 'Delver''s Prism of Stamina and Vigor', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 3, endur = 30, mr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 4, astr 4)
UPDATE tmp_aug SET id = 147752, Name = 'Delver''s Prism of Grace and Might', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 4, astr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 1, acha 3, asta 4)
UPDATE tmp_aug SET id = 147753, Name = 'Delver''s Prism of Vigor and Presence', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 1, acha = 3, asta = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (cr 2, fr 6)
UPDATE tmp_aug SET id = 147754, Name = 'Delver''s Prism of Embers and Frost', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, cr = 2, fr = 6;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 3, acha 2, aint 3)
UPDATE tmp_aug SET id = 147755, Name = 'Delver''s Prism of Grace and Intellect', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 3, acha = 2, aint = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 3, cr 2, fr 3)
UPDATE tmp_aug SET id = 147756, Name = 'Delver''s Prism of Grace and Embers', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 3, cr = 2, fr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (asta 3, awis 2, hp 30)
UPDATE tmp_aug SET id = 147757, Name = 'Delver''s Prism of Life and Vigor', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 3, awis = 2, hp = 30;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (asta 4, dr 1, heroic_fr 1)
UPDATE tmp_aug SET id = 147758, Name = 'Delver''s Prism of Vigor and the Blight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 4, dr = 1, heroic_fr = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (asta 3, endur 50)
UPDATE tmp_aug SET id = 147759, Name = 'Delver''s Prism of Stamina II', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 3, endur = 50;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 2, adex 6)
UPDATE tmp_aug SET id = 147760, Name = 'Delver''s Prism of Precision and the Bulwark', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 2, adex = 6;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 3, heroic_pr 1, pr 2)
UPDATE tmp_aug SET id = 147761, Name = 'Delver''s Prism of Grace and Venom', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 3, heroic_pr = 1, pr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (acha 2, cr 1, pr 5)
UPDATE tmp_aug SET id = 147762, Name = 'Delver''s Prism of Venom and Presence', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 2, cr = 1, pr = 5;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 1, cr 3, pr 4)
UPDATE tmp_aug SET id = 147763, Name = 'Delver''s Prism of Venom and Frost', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 1, cr = 3, pr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 4, acha 1, pr 3)
UPDATE tmp_aug SET id = 147764, Name = 'Delver''s Prism of Grace, Venom and Presence', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 4, acha = 1, pr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (acha 3, astr 2, dr 3)
UPDATE tmp_aug SET id = 147765, Name = 'Delver''s Prism of Presence and the Blight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 3, astr = 2, dr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 1, cr 6, hp 10)
UPDATE tmp_aug SET id = 147766, Name = 'Delver''s Prism of Frost and Grace', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 1, cr = 6, hp = 10;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (adex 3, awis 5)
UPDATE tmp_aug SET id = 147767, Name = 'Delver''s Prism of Insight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 3, awis = 5;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (adex 2, cr 2, endur 40)
UPDATE tmp_aug SET id = 147768, Name = 'Delver''s Prism of Stamina and Frost', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 2, cr = 2, endur = 40;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 1, heroic_pr 1, hp 20, pr 2)
UPDATE tmp_aug SET id = 147769, Name = 'Delver''s Prism of Life and Venom', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 1, heroic_pr = 1, hp = 20, pr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 2, asta 3, awis 3)
UPDATE tmp_aug SET id = 147770, Name = 'Delver''s Prism of Vigor and Insight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 2, asta = 3, awis = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 3, awis 5)
UPDATE tmp_aug SET id = 147771, Name = 'Delver''s Prism of Insight and the Bulwark', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 3, awis = 5;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 4, aint 4)
UPDATE tmp_aug SET id = 147772, Name = 'Delver''s Prism of Grace II', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 4, aint = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 2, adex 3, mana 30)
UPDATE tmp_aug SET id = 147773, Name = 'Delver''s Prism of Precision and the Mind', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 2, adex = 3, mana = 30;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 2, cr 4, pr 2)
UPDATE tmp_aug SET id = 147774, Name = 'Delver''s Prism of Frost and the Bulwark', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 2, cr = 4, pr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (cr 2, endur 20, mana 40)
UPDATE tmp_aug SET id = 147775, Name = 'Delver''s Prism of the Mind', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, cr = 2, endur = 20, mana = 40;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (cr 4, hp 20, mr 2)
UPDATE tmp_aug SET id = 147776, Name = 'Delver''s Prism of Frost and Life', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, cr = 4, hp = 20, mr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (asta 3, pr 5)
UPDATE tmp_aug SET id = 147777, Name = 'Delver''s Prism of Venom II', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 3, pr = 5;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 6, ac 2)
UPDATE tmp_aug SET id = 147778, Name = 'Delver''s Prism of Grace III', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 6, ac = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (adex 3, aint 2, asta 3)
UPDATE tmp_aug SET id = 147779, Name = 'Delver''s Prism of Precision and Vigor', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 3, aint = 2, asta = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (asta 4, astr 4)
UPDATE tmp_aug SET id = 147780, Name = 'Delver''s Prism of Vigor and Might', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 4, astr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (awis 1, endur 50, pr 2)
UPDATE tmp_aug SET id = 147781, Name = 'Delver''s Prism of Stamina and Venom', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, awis = 1, endur = 50, pr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (acha 3, aint 3, fr 2)
UPDATE tmp_aug SET id = 147782, Name = 'Delver''s Prism of Presence and Intellect', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 3, aint = 3, fr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aint 2, dr 1, heroic_dr 1, pr 2)
UPDATE tmp_aug SET id = 147783, Name = 'Delver''s Prism of Intellect and Venom', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aint = 2, dr = 1, heroic_dr = 1, pr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (adex 3, mana 50)
UPDATE tmp_aug SET id = 147784, Name = 'Delver''s Prism of the Mind and Precision', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 3, mana = 50;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 1, adex 2, mana 50)
UPDATE tmp_aug SET id = 147785, Name = 'Delver''s Prism of the Mind, Precision and Grace', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 1, adex = 2, mana = 50;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (cr 3, endur 10, mana 40)
UPDATE tmp_aug SET id = 147786, Name = 'Delver''s Prism of the Mind and Frost', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, cr = 3, endur = 10, mana = 40;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (acha 3, dr 2, mana 30)
UPDATE tmp_aug SET id = 147787, Name = 'Delver''s Prism of Presence and the Mind', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 3, dr = 2, mana = 30;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (adex 2, dr 6)
UPDATE tmp_aug SET id = 147788, Name = 'Delver''s Prism of the Blight and Precision', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 2, dr = 6;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (dr 1, endur 50, hp 20)
UPDATE tmp_aug SET id = 147789, Name = 'Delver''s Prism of Stamina and Life', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, dr = 1, endur = 50, hp = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (astr 4, cr 2, mr 2)
UPDATE tmp_aug SET id = 147790, Name = 'Delver''s Prism of Might and Frost', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, astr = 4, cr = 2, mr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (adex 2, fr 2, mr 4)
UPDATE tmp_aug SET id = 147791, Name = 'Delver''s Prism of Warding and Precision', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 2, fr = 2, mr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (astr 1, dr 7)
UPDATE tmp_aug SET id = 147792, Name = 'Delver''s Prism of the Blight and Might', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, astr = 1, dr = 7;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 2, endur 40, hp 20)
UPDATE tmp_aug SET id = 147793, Name = 'Delver''s Prism of Stamina and Grace', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 2, endur = 40, hp = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 2, astr 6)
UPDATE tmp_aug SET id = 147794, Name = 'Delver''s Prism of Might and the Bulwark', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 2, astr = 6;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (acha 5, pr 3)
UPDATE tmp_aug SET id = 147795, Name = 'Delver''s Prism of Presence and Venom', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 5, pr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (acha 4, mr 4)
UPDATE tmp_aug SET id = 147796, Name = 'Delver''s Prism of Presence and Warding', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 4, mr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (awis 3, mana 20, mr 3)
UPDATE tmp_aug SET id = 147797, Name = 'Delver''s Prism of Warding, Insight and the Mind', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, awis = 3, mana = 20, mr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 2, heroic_cr 1, mr 3)
UPDATE tmp_aug SET id = 147798, Name = 'Delver''s Prism of Warding and the Bulwark', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 2, heroic_cr = 1, mr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (fr 4, hp 40)
UPDATE tmp_aug SET id = 147799, Name = 'Delver''s Prism of Embers and Life', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, fr = 4, hp = 40;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 3, adex 3, aint 2)
UPDATE tmp_aug SET id = 147800, Name = 'Delver''s Prism of the Bulwark and Precision', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 3, adex = 3, aint = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (acha 3, astr 3, mana 20)
UPDATE tmp_aug SET id = 147801, Name = 'Delver''s Prism of Presence and Might', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 3, astr = 3, mana = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (awis 5, endur 30)
UPDATE tmp_aug SET id = 147802, Name = 'Delver''s Prism of Insight and Stamina', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, awis = 5, endur = 30;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (astr 5, awis 3)
UPDATE tmp_aug SET id = 147803, Name = 'Delver''s Prism of Might and Insight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, astr = 5, awis = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (acha 3, asta 3, astr 2)
UPDATE tmp_aug SET id = 147804, Name = 'Delver''s Prism of Presence and Vigor', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 3, asta = 3, astr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (adex 2, dr 2, mana 40)
UPDATE tmp_aug SET id = 147805, Name = 'Delver''s Prism of the Mind, Precision and the Blight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 2, dr = 2, mana = 40;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (adex 5, awis 2, mr 1)
UPDATE tmp_aug SET id = 147806, Name = 'Delver''s Prism of Precision and Insight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 5, awis = 2, mr = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (endur 20, heroic_pr 2)
UPDATE tmp_aug SET id = 147807, Name = 'Delver''s Prism of Stamina III', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, endur = 20, heroic_pr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 3, fr 5)
UPDATE tmp_aug SET id = 147808, Name = 'Delver''s Prism of Embers and Grace', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 3, fr = 5;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (hp 30, mana 20, mr 3)
UPDATE tmp_aug SET id = 147809, Name = 'Delver''s Prism of Life and Warding', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, hp = 30, mana = 20, mr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (asta 4, dr 4)
UPDATE tmp_aug SET id = 147810, Name = 'Delver''s Prism of the Blight and Vigor', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 4, dr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 3, acha 4, fr 1)
UPDATE tmp_aug SET id = 147811, Name = 'Delver''s Prism of Presence and the Bulwark', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 3, acha = 4, fr = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (asta 2, fr 4, pr 2)
UPDATE tmp_aug SET id = 147812, Name = 'Delver''s Prism of Embers and Venom', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 2, fr = 4, pr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (cr 3, dr 2, mr 3)
UPDATE tmp_aug SET id = 147813, Name = 'Delver''s Prism of Frost and Warding', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, cr = 3, dr = 2, mr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 3, dr 2, endur 30)
UPDATE tmp_aug SET id = 147814, Name = 'Delver''s Prism of the Bulwark and Stamina', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 3, dr = 2, endur = 30;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (astr 1, cr 2, fr 2, heroic_str 1)
UPDATE tmp_aug SET id = 147815, Name = 'Delver''s Prism of Frost, Embers and Might', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, astr = 1, cr = 2, fr = 2, heroic_str = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (awis 2, mana 60)
UPDATE tmp_aug SET id = 147816, Name = 'Delver''s Prism of the Mind and Insight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, awis = 2, mana = 60;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (astr 5, fr 3)
UPDATE tmp_aug SET id = 147817, Name = 'Delver''s Prism of Might and Embers', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, astr = 5, fr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (acha 3, fr 5)
UPDATE tmp_aug SET id = 147818, Name = 'Delver''s Prism of Embers and Presence', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 3, fr = 5;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (astr 4, hp 40)
UPDATE tmp_aug SET id = 147819, Name = 'Delver''s Prism of Life and Might', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, astr = 4, hp = 40;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 2, acha 2, mana 40)
UPDATE tmp_aug SET id = 147820, Name = 'Delver''s Prism of the Mind and Grace', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 2, acha = 2, mana = 40;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (awis 4, mana 40)
UPDATE tmp_aug SET id = 147821, Name = 'Delver''s Prism of the Mind II', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, awis = 4, mana = 40;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (adex 1, awis 3, mr 4)
UPDATE tmp_aug SET id = 147822, Name = 'Delver''s Prism of Warding, Insight and Precision', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 1, awis = 3, mr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (asta 2, cr 4, hp 20)
UPDATE tmp_aug SET id = 147823, Name = 'Delver''s Prism of Frost, Life and Vigor', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 2, cr = 4, hp = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (adex 2, asta 5, astr 1)
UPDATE tmp_aug SET id = 147824, Name = 'Delver''s Prism of Vigor and Precision', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 2, asta = 5, astr = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 5, heroic_mr 1)
UPDATE tmp_aug SET id = 147825, Name = 'Delver''s Prism of the Bulwark and Warding', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 5, heroic_mr = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (astr 3, dr 3, mr 2)
UPDATE tmp_aug SET id = 147826, Name = 'Delver''s Prism of the Blight, Might and Warding', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, astr = 3, dr = 3, mr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (astr 6, hp 20)
UPDATE tmp_aug SET id = 147827, Name = 'Delver''s Prism of Might and Life', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, astr = 6, hp = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 4, mana 40)
UPDATE tmp_aug SET id = 147828, Name = 'Delver''s Prism of Grace and the Mind', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 4, mana = 40;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 4, adex 2, awis 2)
UPDATE tmp_aug SET id = 147829, Name = 'Delver''s Prism of Grace and Precision', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 4, adex = 2, awis = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (acha 4, pr 4)
UPDATE tmp_aug SET id = 147830, Name = 'Delver''s Prism of Presence II', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 4, pr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (fr 3, mr 5)
UPDATE tmp_aug SET id = 147831, Name = 'Delver''s Prism of Warding and Embers', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, fr = 3, mr = 5;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (awis 2, fr 3, mana 30)
UPDATE tmp_aug SET id = 147832, Name = 'Delver''s Prism of Embers and the Mind', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, awis = 2, fr = 3, mana = 30;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (fr 5, hp 30)
UPDATE tmp_aug SET id = 147833, Name = 'Delver''s Prism of Embers II', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, fr = 5, hp = 30;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 2, adex 2, pr 4)
UPDATE tmp_aug SET id = 147834, Name = 'Delver''s Prism of Venom, Grace and Precision', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 2, adex = 2, pr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (acha 5, adex 3)
UPDATE tmp_aug SET id = 147835, Name = 'Delver''s Prism of Presence and Precision', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 5, adex = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (awis 4, pr 4)
UPDATE tmp_aug SET id = 147836, Name = 'Delver''s Prism of Venom III', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, awis = 4, pr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (heroic_agi 2, mana 20)
UPDATE tmp_aug SET id = 147837, Name = 'Delver''s Prism of the Mind III', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, heroic_agi = 2, mana = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (awis 5, dr 3)
UPDATE tmp_aug SET id = 147838, Name = 'Delver''s Prism of Insight and the Blight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, awis = 5, dr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (dr 5, fr 3)
UPDATE tmp_aug SET id = 147839, Name = 'Delver''s Prism of the Blight and Embers', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, dr = 5, fr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (adex 4, aint 2, astr 2)
UPDATE tmp_aug SET id = 147840, Name = 'Delver''s Prism of Precision and Intellect', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 4, aint = 2, astr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 2, ac 3, dr 3)
UPDATE tmp_aug SET id = 147841, Name = 'Delver''s Prism of the Bulwark and the Blight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 2, ac = 3, dr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 4, hp 40)
UPDATE tmp_aug SET id = 147842, Name = 'Delver''s Prism of the Bulwark II', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 4, hp = 40;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (asta 3, dr 1, endur 40)
UPDATE tmp_aug SET id = 147843, Name = 'Delver''s Prism of Stamina, Vigor and the Blight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 3, dr = 1, endur = 40;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 3, endur 20, mana 30)
UPDATE tmp_aug SET id = 147844, Name = 'Delver''s Prism of the Bulwark and the Mind', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 3, endur = 20, mana = 30;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 2, acha 3, astr 3)
UPDATE tmp_aug SET id = 147845, Name = 'Delver''s Prism of Presence, Might and the Bulwark', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 2, acha = 3, astr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (astr 5, mana 30)
UPDATE tmp_aug SET id = 147846, Name = 'Delver''s Prism of Might and the Mind', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, astr = 5, mana = 30;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (awis 4, dr 2, mr 2)
UPDATE tmp_aug SET id = 147847, Name = 'Delver''s Prism of Insight, the Blight and Warding', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, awis = 4, dr = 2, mr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aint 5, fr 1, hp 20)
UPDATE tmp_aug SET id = 147848, Name = 'Delver''s Prism of Intellect and Life', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aint = 5, fr = 1, hp = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 2, astr 2, cr 4)
UPDATE tmp_aug SET id = 147849, Name = 'Delver''s Prism of Frost, the Bulwark and Might', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 2, astr = 2, cr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 5, ac 3)
UPDATE tmp_aug SET id = 147850, Name = 'Delver''s Prism of Grace IV', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 5, ac = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (acha 2, aint 1, fr 5)
UPDATE tmp_aug SET id = 147851, Name = 'Delver''s Prism of Embers, Presence and Intellect', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 2, aint = 1, fr = 5;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 3, mr 5)
UPDATE tmp_aug SET id = 147852, Name = 'Delver''s Prism of Warding and Grace', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 3, mr = 5;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 3, acha 5)
UPDATE tmp_aug SET id = 147853, Name = 'Delver''s Prism of Presence III', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 3, acha = 5;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (fr 6, pr 2)
UPDATE tmp_aug SET id = 147854, Name = 'Delver''s Prism of Embers III', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, fr = 6, pr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 5, dr 3)
UPDATE tmp_aug SET id = 147855, Name = 'Delver''s Prism of Grace and the Blight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 5, dr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (astr 4, cr 4)
UPDATE tmp_aug SET id = 147856, Name = 'Delver''s Prism of Frost and Might', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, astr = 4, cr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aint 3, hp 50)
UPDATE tmp_aug SET id = 147857, Name = 'Delver''s Prism of Life and Intellect', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aint = 3, hp = 50;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (acha 3, pr 5)
UPDATE tmp_aug SET id = 147858, Name = 'Delver''s Prism of Venom IV', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 3, pr = 5;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (awis 2, endur 30, hp 30)
UPDATE tmp_aug SET id = 147859, Name = 'Delver''s Prism of Stamina, Life and Insight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, awis = 2, endur = 30, hp = 30;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (adex 3, astr 3, dr 2)
UPDATE tmp_aug SET id = 147860, Name = 'Delver''s Prism of Precision and Might', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 3, astr = 3, dr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (dr 3, mana 50)
UPDATE tmp_aug SET id = 147861, Name = 'Delver''s Prism of the Mind and the Blight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, dr = 3, mana = 50;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (astr 2, endur 20, mana 40)
UPDATE tmp_aug SET id = 147862, Name = 'Delver''s Prism of the Mind and Stamina', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, astr = 2, endur = 20, mana = 40;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (awis 3, heroic_fr 1, pr 2)
UPDATE tmp_aug SET id = 147863, Name = 'Delver''s Prism of Insight and Venom', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, awis = 3, heroic_fr = 1, pr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aint 5, cr 3)
UPDATE tmp_aug SET id = 147864, Name = 'Delver''s Prism of Intellect and Frost', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aint = 5, cr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 2, ac 3, hp 30)
UPDATE tmp_aug SET id = 147865, Name = 'Delver''s Prism of the Bulwark, Life and Grace', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 2, ac = 3, hp = 30;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (awis 5, endur 10, pr 2)
UPDATE tmp_aug SET id = 147866, Name = 'Delver''s Prism of Insight, Venom and Stamina', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, awis = 5, endur = 10, pr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 2, fr 3, hp 30)
UPDATE tmp_aug SET id = 147867, Name = 'Delver''s Prism of Embers, Life and the Bulwark', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 2, fr = 3, hp = 30;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (astr 4, mr 1, pr 3)
UPDATE tmp_aug SET id = 147868, Name = 'Delver''s Prism of Might, Venom and Warding', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, astr = 4, mr = 1, pr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 3, awis 3, mr 2)
UPDATE tmp_aug SET id = 147869, Name = 'Delver''s Prism of Grace and Insight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 3, awis = 3, mr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 2, mana 60)
UPDATE tmp_aug SET id = 147870, Name = 'Delver''s Prism of the Mind IV', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 2, mana = 60;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (awis 4, mr 4)
UPDATE tmp_aug SET id = 147871, Name = 'Delver''s Prism of Warding II', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, awis = 4, mr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (dr 2, heroic_wis 1, mr 3)
UPDATE tmp_aug SET id = 147872, Name = 'Delver''s Prism of Warding and the Blight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, dr = 2, heroic_wis = 1, mr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 2, asta 3, pr 3)
UPDATE tmp_aug SET id = 147873, Name = 'Delver''s Prism of Venom, Vigor and the Bulwark', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 2, asta = 3, pr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (asta 3, astr 2, endur 30)
UPDATE tmp_aug SET id = 147874, Name = 'Delver''s Prism of Stamina, Vigor and Might', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 3, astr = 2, endur = 30;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (acha 6, aint 2)
UPDATE tmp_aug SET id = 147875, Name = 'Delver''s Prism of Presence IV', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 6, aint = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (endur 60, pr 2)
UPDATE tmp_aug SET id = 147876, Name = 'Delver''s Prism of Stamina IV', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, endur = 60, pr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (adex 1, dr 5, endur 20)
UPDATE tmp_aug SET id = 147877, Name = 'Delver''s Prism of the Blight and Stamina', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 1, dr = 5, endur = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (acha 5, awis 3)
UPDATE tmp_aug SET id = 147878, Name = 'Delver''s Prism of Presence and Insight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 5, awis = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 3, cr 2, fr 3)
UPDATE tmp_aug SET id = 147879, Name = 'Delver''s Prism of the Bulwark and Embers', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 3, cr = 2, fr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aint 3, awis 3, endur 20)
UPDATE tmp_aug SET id = 147880, Name = 'Delver''s Prism of Intellect and Insight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aint = 3, awis = 3, endur = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (endur 20, mana 20, pr 4)
UPDATE tmp_aug SET id = 147881, Name = 'Delver''s Prism of Venom and Stamina', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, endur = 20, mana = 20, pr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 6, mr 2)
UPDATE tmp_aug SET id = 147882, Name = 'Delver''s Prism of Grace and Warding', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 6, mr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (dr 5, hp 30)
UPDATE tmp_aug SET id = 147883, Name = 'Delver''s Prism of the Blight and Life', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, dr = 5, hp = 30;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (asta 4, hp 40)
UPDATE tmp_aug SET id = 147884, Name = 'Delver''s Prism of Life II', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 4, hp = 40;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 3, acha 1, heroic_cha 1, hp 10)
UPDATE tmp_aug SET id = 147885, Name = 'Delver''s Prism of the Bulwark and Presence', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 3, acha = 1, heroic_cha = 1, hp = 10;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (cr 3, mr 5)
UPDATE tmp_aug SET id = 147886, Name = 'Delver''s Prism of Warding and Frost', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, cr = 3, mr = 5;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 5, cr 1, pr 2)
UPDATE tmp_aug SET id = 147887, Name = 'Delver''s Prism of the Bulwark and Venom', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 5, cr = 1, pr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (adex 4, endur 40)
UPDATE tmp_aug SET id = 147888, Name = 'Delver''s Prism of Precision II', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 4, endur = 40;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 3, acha 3, pr 2)
UPDATE tmp_aug SET id = 147889, Name = 'Delver''s Prism of the Bulwark, Presence and Venom', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 3, acha = 3, pr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 2, ac 6)
UPDATE tmp_aug SET id = 147890, Name = 'Delver''s Prism of the Bulwark and Grace', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 2, ac = 6;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (cr 1, endur 20, heroic_cr 1, mana 20)
UPDATE tmp_aug SET id = 147891, Name = 'Delver''s Prism of Stamina and the Mind', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, cr = 1, endur = 20, heroic_cr = 1, mana = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (dr 4, hp 40)
UPDATE tmp_aug SET id = 147892, Name = 'Delver''s Prism of the Blight II', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, dr = 4, hp = 40;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 2, ac 3, cr 3)
UPDATE tmp_aug SET id = 147893, Name = 'Delver''s Prism of the Bulwark and Frost', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 2, ac = 3, cr = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (acha 1, mana 30, mr 4)
UPDATE tmp_aug SET id = 147894, Name = 'Delver''s Prism of Warding and the Mind', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 1, mana = 30, mr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (acha 2, aint 5, endur 10)
UPDATE tmp_aug SET id = 147895, Name = 'Delver''s Prism of Intellect and Presence', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 2, aint = 5, endur = 10;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 1, ac 4, heroic_agi 1)
UPDATE tmp_aug SET id = 147896, Name = 'Delver''s Prism of the Bulwark III', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 1, ac = 4, heroic_agi = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (astr 1, heroic_str 1, mana 40)
UPDATE tmp_aug SET id = 147897, Name = 'Delver''s Prism of the Mind and Might', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, astr = 1, heroic_str = 1, mana = 40;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (acha 1, aint 3, asta 4)
UPDATE tmp_aug SET id = 147898, Name = 'Delver''s Prism of Vigor, Intellect and Presence', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 1, aint = 3, asta = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (acha 3, adex 2, endur 30)
UPDATE tmp_aug SET id = 147899, Name = 'Delver''s Prism of Presence and Stamina', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 3, adex = 2, endur = 30;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (adex 2, astr 4, endur 20)
UPDATE tmp_aug SET id = 147900, Name = 'Delver''s Prism of Might and Precision', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 2, astr = 4, endur = 20;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (acha 3, awis 5)
UPDATE tmp_aug SET id = 147901, Name = 'Delver''s Prism of Insight and Presence', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, acha = 3, awis = 5;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (asta 3, endur 10, pr 4)
UPDATE tmp_aug SET id = 147902, Name = 'Delver''s Prism of Venom, Vigor and Stamina', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, asta = 3, endur = 10, pr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (cr 1, fr 3, mr 4)
UPDATE tmp_aug SET id = 147903, Name = 'Delver''s Prism of Warding, Embers and Frost', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, cr = 1, fr = 3, mr = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 2, cr 4, fr 2)
UPDATE tmp_aug SET id = 147904, Name = 'Delver''s Prism of Frost, Grace and Embers', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 2, cr = 4, fr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 2, hp 40, mr 2)
UPDATE tmp_aug SET id = 147905, Name = 'Delver''s Prism of Life and Grace', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 2, hp = 40, mr = 2;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 2, heroic_agi 1, hp 30)
UPDATE tmp_aug SET id = 147906, Name = 'Delver''s Prism of Life III', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 2, heroic_agi = 1, hp = 30;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (ac 4, asta 1, awis 3)
UPDATE tmp_aug SET id = 147907, Name = 'Delver''s Prism of the Bulwark and Insight', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, ac = 4, asta = 1, awis = 3;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 4, asta 1, heroic_wis 1)
UPDATE tmp_aug SET id = 147908, Name = 'Delver''s Prism of Grace and Vigor', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 4, asta = 1, heroic_wis = 1;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (heroic_cr 1, hp 50)
UPDATE tmp_aug SET id = 147909, Name = 'Delver''s Prism of Life and Frost', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, heroic_cr = 1, hp = 50;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aint 4, asta 4)
UPDATE tmp_aug SET id = 147910, Name = 'Delver''s Prism of Intellect and Vigor', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aint = 4, asta = 4;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (adex 1, aint 2, endur 50)
UPDATE tmp_aug SET id = 147911, Name = 'Delver''s Prism of Stamina and Intellect', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, adex = 1, aint = 2, endur = 50;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (aagi 3, adex 5)
UPDATE tmp_aug SET id = 147912, Name = 'Delver''s Prism of Precision and Grace', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, aagi = 3, adex = 5;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (mana 20, mr 6)
UPDATE tmp_aug SET id = 147913, Name = 'Delver''s Prism of Warding III', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, mana = 20, mr = 6;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (heroic_str 1, hp 50)
UPDATE tmp_aug SET id = 147914, Name = 'Delver''s Prism of Life IV', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, heroic_str = 1, hp = 50;
INSERT INTO items SELECT * FROM tmp_aug;
-- T3  weight 8  (heroic_fr 2, mana 20)
UPDATE tmp_aug SET id = 147915, Name = 'Delver''s Prism of the Mind and Embers', astr = 0, asta = 0, aagi = 0, adex = 0, aint = 0, awis = 0, acha = 0, ac = 0, hp = 0, mana = 0, endur = 0, mr = 0, fr = 0, cr = 0, dr = 0, pr = 0, heroic_str = 0, heroic_sta = 0, heroic_agi = 0, heroic_dex = 0, heroic_int = 0, heroic_wis = 0, heroic_cha = 0, heroic_mr = 0, heroic_fr = 0, heroic_cr = 0, heroic_dr = 0, heroic_pr = 0, heroic_fr = 2, mana = 20;
INSERT INTO items SELECT * FROM tmp_aug;

DROP TEMPORARY TABLE tmp_aug;

-- ---------------------------------------------------------------- verify
SELECT 'aug items' AS what, COUNT(*) n FROM items WHERE id BETWEEN 147600 AND 147915
UNION ALL SELECT 'tier 1', COUNT(*) FROM items WHERE id BETWEEN 147600 AND 147615
UNION ALL SELECT 'tier 2', COUNT(*) FROM items WHERE id BETWEEN 147616 AND 147715
UNION ALL SELECT 'tier 3', COUNT(*) FROM items WHERE id BETWEEN 147716 AND 147915
UNION ALL SELECT 'still flagged evolving', COUNT(*) FROM items WHERE id BETWEEN 147600 AND 147915 AND evoitem <> 0;
-- expected: 316 / 16 / 100 / 200 / 0
