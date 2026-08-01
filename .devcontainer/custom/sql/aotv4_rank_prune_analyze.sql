-- AoTv4 -- STEP 1 of 2: work out which native rank variants are safe to remove. DELETES NOTHING.
-- =====================================================================================
-- Builds the staging table `aotv4_rank_prune` and reports what step 2 would remove. Run this,
-- read the numbers, and only then run aotv4_rank_prune_execute.sql.
--
-- WHY. spells_new holds 41,049 rows against a HARD client ceiling of 45000 ids (measured -- see
-- SPELL_RANKS.md), leaving 3,951 free. Native `Rk. II`/`Rk. III` variants are ~7,165 of those rows
-- and are unreachable here: the level-up picker filters them out by name, and the planned custom
-- rank system replaces them entirely.
--
-- ⚠️⚠️ WHAT IS KEPT, AND WHY THE LIST MATTERS MORE THAN THE COUNT.
-- 5,076 rank variants ARE referenced -- but 4,611 of those references are the spell's OWN SCROLL,
-- which is circular: the scroll exists only to teach the spell, so the pair goes together. The
-- genuine dependencies are tiny and are all preserved:
--     aa_ranks             360  -- our AA trees REUSE native rows (section 6); deleting one leaves
--                                  an ability that trains and then does nothing, silently
--     items.click           31  -- clickies players can actually use
--     npc_spells_entries     1  -- the only rank variant any NPC casts
--     items.proc/worn/focus  0
--     character_spells       0  -- nobody has one scribed
--
-- ⚠️ Identified by name OR `rank` > 1. The two signals disagree on 1,177 spells (993 have rank > 1
-- with no "Rk." in the name, 184 the reverse), so neither alone catches the set. `rank` is the data
-- column and the name is cosmetic; requiring BOTH would miss 1,177 rows.
--
-- ⚠️ Read-only. Creates staging tables only.

-- ---------------------------------------------------------------- every reference EXCEPT the scroll
DROP TABLE IF EXISTS aotv4_ref_noscroll;
CREATE TABLE aotv4_ref_noscroll (id INT PRIMARY KEY);
INSERT IGNORE INTO aotv4_ref_noscroll
  SELECT clickeffect FROM items WHERE clickeffect>0
  UNION SELECT proceffect  FROM items WHERE proceffect>0
  UNION SELECT worneffect  FROM items WHERE worneffect>0
  UNION SELECT focuseffect FROM items WHERE focuseffect>0
  UNION SELECT bardeffect  FROM items WHERE bardeffect>0
  UNION SELECT spellid FROM npc_spells_entries WHERE spellid>0
  UNION SELECT attack_proc    FROM npc_spells WHERE attack_proc>0
  UNION SELECT defensive_proc FROM npc_spells WHERE defensive_proc>0
  UNION SELECT range_proc     FROM npc_spells WHERE range_proc>0
  UNION SELECT spell    FROM aa_ranks WHERE spell>0
  UNION SELECT spell_id FROM auras WHERE spell_id>0
  UNION SELECT spellid  FROM blocked_spells WHERE spellid>0
  UNION SELECT spellid  FROM damageshieldtypes WHERE spellid>0
  UNION SELECT spell_id FROM ldon_trap_templates WHERE spell_id>0
  UNION SELECT spell_id FROM spell_buckets WHERE spell_id>0
  UNION SELECT spellid  FROM spell_globals WHERE spellid>0
  UNION SELECT SpellId  FROM merc_buffs WHERE SpellId>0
  UNION SELECT spell_id FROM character_spells        WHERE spell_id>0
  UNION SELECT spell_id FROM character_memmed_spells WHERE spell_id>0
  UNION SELECT spell_id FROM character_buffs         WHERE spell_id>0
  UNION SELECT spell_id FROM character_auras         WHERE spell_id>0
  UNION SELECT RecourseLink FROM spells_new WHERE RecourseLink>0;

-- ⚠️⚠️ TRIGGER / PROC REFS LIVE IN effect_base_value AND effect_limit_value, for particular SPAs
-- only. Section 14 records this being missed once already: the 50xxx -> 43xxx renumber broke
-- Moonfire and Firefist because their trigger refs were not renumbered with them. Both fields are
-- checked for all ten spell-carrying SPAs (85, 289, 323, 340, 374, 383, 406, 419, 427, 429).
INSERT IGNORE INTO aotv4_ref_noscroll
  SELECT v FROM (
    SELECT effect_base_value1 v FROM spells_new WHERE effectid1 IN (85,289,323,340,374,383,406,419,427,429) AND effect_base_value1>0
    UNION SELECT effect_limit_value1 FROM spells_new WHERE effectid1 IN (85,289,323,340,374,383,406,419,427,429) AND effect_limit_value1>0
    UNION SELECT effect_base_value2  FROM spells_new WHERE effectid2 IN (85,289,323,340,374,383,406,419,427,429) AND effect_base_value2>0
    UNION SELECT effect_limit_value2 FROM spells_new WHERE effectid2 IN (85,289,323,340,374,383,406,419,427,429) AND effect_limit_value2>0
    UNION SELECT effect_base_value3  FROM spells_new WHERE effectid3 IN (85,289,323,340,374,383,406,419,427,429) AND effect_base_value3>0
    UNION SELECT effect_limit_value3 FROM spells_new WHERE effectid3 IN (85,289,323,340,374,383,406,419,427,429) AND effect_limit_value3>0
    UNION SELECT effect_base_value4  FROM spells_new WHERE effectid4 IN (85,289,323,340,374,383,406,419,427,429) AND effect_base_value4>0
    UNION SELECT effect_limit_value4 FROM spells_new WHERE effectid4 IN (85,289,323,340,374,383,406,419,427,429) AND effect_limit_value4>0
    UNION SELECT effect_base_value5  FROM spells_new WHERE effectid5 IN (85,289,323,340,374,383,406,419,427,429) AND effect_base_value5>0
    UNION SELECT effect_limit_value5 FROM spells_new WHERE effectid5 IN (85,289,323,340,374,383,406,419,427,429) AND effect_limit_value5>0
    UNION SELECT effect_base_value6  FROM spells_new WHERE effectid6 IN (85,289,323,340,374,383,406,419,427,429) AND effect_base_value6>0
    UNION SELECT effect_limit_value6 FROM spells_new WHERE effectid6 IN (85,289,323,340,374,383,406,419,427,429) AND effect_limit_value6>0
    UNION SELECT effect_base_value7  FROM spells_new WHERE effectid7 IN (85,289,323,340,374,383,406,419,427,429) AND effect_base_value7>0
    UNION SELECT effect_limit_value7 FROM spells_new WHERE effectid7 IN (85,289,323,340,374,383,406,419,427,429) AND effect_limit_value7>0
    UNION SELECT effect_base_value8  FROM spells_new WHERE effectid8 IN (85,289,323,340,374,383,406,419,427,429) AND effect_base_value8>0
    UNION SELECT effect_limit_value8 FROM spells_new WHERE effectid8 IN (85,289,323,340,374,383,406,419,427,429) AND effect_limit_value8>0
    UNION SELECT effect_base_value9  FROM spells_new WHERE effectid9 IN (85,289,323,340,374,383,406,419,427,429) AND effect_base_value9>0
    UNION SELECT effect_limit_value9 FROM spells_new WHERE effectid9 IN (85,289,323,340,374,383,406,419,427,429) AND effect_limit_value9>0
    UNION SELECT effect_base_value10 FROM spells_new WHERE effectid10 IN (85,289,323,340,374,383,406,419,427,429) AND effect_base_value10>0
    UNION SELECT effect_limit_value10 FROM spells_new WHERE effectid10 IN (85,289,323,340,374,383,406,419,427,429) AND effect_limit_value10>0
    UNION SELECT effect_base_value11 FROM spells_new WHERE effectid11 IN (85,289,323,340,374,383,406,419,427,429) AND effect_base_value11>0
    UNION SELECT effect_limit_value11 FROM spells_new WHERE effectid11 IN (85,289,323,340,374,383,406,419,427,429) AND effect_limit_value11>0
    UNION SELECT effect_base_value12 FROM spells_new WHERE effectid12 IN (85,289,323,340,374,383,406,419,427,429) AND effect_base_value12>0
    UNION SELECT effect_limit_value12 FROM spells_new WHERE effectid12 IN (85,289,323,340,374,383,406,419,427,429) AND effect_limit_value12>0
  ) t WHERE v > 0;

-- ---------------------------------------------------------------- the spells to remove
DROP TABLE IF EXISTS aotv4_rank_prune;
CREATE TABLE aotv4_rank_prune (id INT PRIMARY KEY, name VARCHAR(64), scroll_item INT);
INSERT INTO aotv4_rank_prune (id, name)
SELECT s.id, s.name
FROM   spells_new s
LEFT   JOIN aotv4_ref_noscroll r ON r.id = s.id
WHERE  (s.name LIKE '% Rk. %' OR s.`rank` > 1)
  AND  r.id IS NULL;

-- ---------------------------------------------------------------- the scrolls that go with them
-- ⚠️ ONLY items that are PURELY a scroll for a doomed spell. An item that also clicks, procs or has
-- a worn effect is something a player can use for another reason and is left alone -- it would just
-- lose its scroll teaching, which is harmless.
DROP TABLE IF EXISTS aotv4_scroll_prune;
CREATE TABLE aotv4_scroll_prune (id INT PRIMARY KEY, Name VARCHAR(64), scrolleffect INT);
INSERT INTO aotv4_scroll_prune (id, Name, scrolleffect)
SELECT i.id, i.Name, i.scrolleffect
FROM   items i
JOIN   aotv4_rank_prune p ON p.id = i.scrolleffect
WHERE  COALESCE(i.clickeffect,0) <= 0
  AND  COALESCE(i.proceffect,0)  <= 0
  AND  COALESCE(i.worneffect,0)  <= 0
  AND  COALESCE(i.focuseffect,0) <= 0
  AND  COALESCE(i.bardeffect,0)  <= 0;

UPDATE aotv4_rank_prune p
JOIN   aotv4_scroll_prune s ON s.scrolleffect = p.id
SET    p.scroll_item = s.id;

-- ---------------------------------------------------------------- report
SELECT (SELECT COUNT(*) FROM spells_new)                                        AS spells_now,
       (SELECT COUNT(*) FROM spells_new WHERE name LIKE '% Rk. %' OR `rank`>1)  AS rank_variants,
       (SELECT COUNT(*) FROM aotv4_rank_prune)                                  AS spells_to_remove,
       (SELECT COUNT(*) FROM aotv4_scroll_prune)                                AS scrolls_to_remove,
       (SELECT 45000 - COUNT(*) FROM spells_new WHERE id < 45000)               AS free_ids_before,
       (SELECT 45000 - COUNT(*) FROM spells_new WHERE id < 45000)
         + (SELECT COUNT(*) FROM aotv4_rank_prune WHERE id < 45000)             AS free_ids_after;

-- Rank variants deliberately KEPT, and what saved each one.
SELECT 'kept: referenced by aa_ranks'      AS kept, COUNT(DISTINCT s.id) AS n
  FROM spells_new s JOIN aa_ranks a ON a.spell=s.id
  WHERE (s.name LIKE '% Rk. %' OR s.`rank`>1)
UNION ALL SELECT 'kept: an NPC casts it', COUNT(DISTINCT s.id)
  FROM spells_new s JOIN npc_spells_entries n ON n.spellid=s.id
  WHERE (s.name LIKE '% Rk. %' OR s.`rank`>1)
UNION ALL SELECT 'kept: a usable click item', COUNT(DISTINCT s.id)
  FROM spells_new s JOIN items i ON i.clickeffect=s.id
  WHERE (s.name LIKE '% Rk. %' OR s.`rank`>1);

-- ⚠️⚠️ THE SAFETY CHECKS. Every one must read 0 before step 2 is run.
-- A scroll sitting in somebody's bags, bank, corpse, shop or a ground spawn becomes a broken item
-- reference the moment the row is deleted -- the id resolves to nothing and the slot renders empty.
-- ⚠️ `inventory` keys on `item_id`, NOT `itemid`; `object` (ground spawns) is the odd one out and
-- really is `itemid`. Getting that wrong fails loudly here, which is the point of running step 1.
SELECT (SELECT COUNT(*) FROM aotv4_rank_prune p JOIN character_spells c ON c.spell_id=p.id)  AS scribed_by_a_player,
       (SELECT COUNT(*) FROM aotv4_rank_prune p JOIN character_buffs  b ON b.spell_id=p.id)  AS active_as_a_buff,
       (SELECT COUNT(*) FROM aotv4_scroll_prune s JOIN inventory i    ON i.item_id=s.id)     AS in_player_bags,
       (SELECT COUNT(*) FROM aotv4_scroll_prune s JOIN sharedbank b   ON b.item_id=s.id)     AS in_shared_bank,
       (SELECT COUNT(*) FROM aotv4_scroll_prune s JOIN character_corpse_items c ON c.item_id=s.id) AS on_corpses,
       (SELECT COUNT(*) FROM aotv4_scroll_prune s JOIN trader t       ON t.item_id=s.id)     AS listed_in_shops,
       (SELECT COUNT(*) FROM aotv4_scroll_prune s JOIN object o       ON o.itemid=s.id)      AS ground_spawns,
       (SELECT COUNT(*) FROM aotv4_scroll_prune s JOIN starting_items st ON st.item_id=s.id) AS starting_items;

-- What step 2 will also have to clean so nothing dangles: rows that SELL or DROP a doomed scroll.
SELECT (SELECT COUNT(*) FROM merchantlist m     JOIN aotv4_scroll_prune s ON s.id=m.item)    AS merchant_rows,
       (SELECT COUNT(*) FROM lootdrop_entries l JOIN aotv4_scroll_prune s ON s.id=l.item_id) AS lootdrop_rows;
