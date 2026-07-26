-- aotv4_light_of_marr_fix.sql -- repair Hopebringer's group-heal proc.
-- =============================================================================================
-- 3606 "Light of Marr" is the proc on Hopebringer, the Paladin sword whose entire identity is
-- healing your group when it fires. In this database it is a STUB and does the opposite of that:
--
--     targettype 5 (single target)   goodEffect 0 (detrimental)   SPA 0 base -1
--
-- i.e. it deals ONE POINT OF DAMAGE to the mob you just hit. Its recourse (4119 "Light of Marr R.")
-- is an empty shell too: SPA 10 with base 0, which does nothing.
--
-- This reshapes 3606 into the group heal it is supposed to be, copying the field layout of the
-- native working example rather than inventing one: 1976 "Blessing of the Blackstar", the proc on
-- 31348 Blackstar, Mace of Night.
--
--   field            was            now      why
--   targettype       5              41       ST_Group -- the whole point
--   goodEffect       0              1        ExecWeaponProc (zone/mob.cpp:5358) casts a BENEFICIAL
--                                            proc on the SWINGER and a detrimental one on the
--                                            victim. At 0 the heal would land on the mob.
--   resisttype       1 (magic)      0        a heal on your own group should never be resisted
--   effect_base_1    -1  (damage)   300      the heal
--   range            200            100      matches 1976
--   new_icon         139            99       the heal icon, not the bash icon
--   RecourseLink     4119           0        4119 is an empty spell; nothing to recourse to
--
-- HEAL SIZE -- calibrated against the only two native group-heal procs in the DB:
--     Blackstar, Mace of Night   no req level, dmg 20/dly 25   heals 100
--     Sword of the Sanative      level 100,    dmg 102/dly 22  heals 550
--   Hopebringer sits at level 65 with dmg 41/dly 25, so 300 places it between the two, nearer the
--   lower end because its damage ratio is modest.
--
-- ⚠️ SHARED BY SIX ITEMS. 3606 is the proc on Hopebringer (23498) AND Bloodstone Blade of the
-- Zun'Muram (69112) -- both level 65, so one heal value suits both -- plus our Hallowed (+300000)
-- and Mythic (+600000) tiers of each, which inherit it automatically. Fixing the spell fixes all six.
--
--   mysql -h127.0.0.1 -upeq -ppeqpass peq < .devcontainer/custom/sql/aotv4_light_of_marr_fix.sql
--
-- ⚠️ spells_new IS in shared memory (section 10): stop world + zones, run ./shared_memory, restart.
-- No pool regen needed -- 3606 is an item proc, never offered as a level-up reward.
-- =============================================================================================

UPDATE spells_new SET
    targettype         = 41,     -- ST_Group
    goodEffect         = 1,      -- load-bearing: selects the "cast on the swinger" branch
    resisttype         = 0,      -- RESIST_NONE
    effectid1          = 0,      -- SpellEffect::CurrentHP
    effect_base_value1 = 300,    -- positive = heal
    effect_limit_value1= 0,
    max1               = 0,
    `range`            = 100,
    aoerange           = 0,
    buffduration       = 0,
    buffdurationformula= 0,
    new_icon           = 99,
    icon               = 2510,
    RecourseLink       = 0,
    spell_category     = -99
WHERE id = 3606;

-- Description shown in the item's proc text.
UPDATE db_str SET value = 'Washes your group in the light of Marr, healing each member for 300 points.'
WHERE id = 3606 AND type = 6;
INSERT INTO db_str (id, type, value)
SELECT 3606, 6, 'Washes your group in the light of Marr, healing each member for 300 points.'
WHERE NOT EXISTS (SELECT 1 FROM db_str WHERE id = 3606 AND type = 6);
