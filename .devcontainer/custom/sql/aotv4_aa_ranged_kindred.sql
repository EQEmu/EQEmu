-- aotv4_aa_ranged_kindred.sql -- Kindred Bond, the first AA on the Ranged tab.
-- =============================================================================================
-- Every pet now carries a standing ward suited to its family (custom/sql/aotv4_pet_wards.sql).
-- Kindred Bond hands a copy of that ward to the pet's owner, so what your companion is changes what
-- YOU are: a fire pet lends you its damage shield, an air pet its speed, a familiar its clarity.
--
-- Read the header of aotv4_aa_tank_hosted.sql first. Same two rules: a custom AA with a new id
-- never reaches the client, and rank ids are NOT contiguous -- the chain below was walked.
--
-- ⚠️ type = 3 puts this on the tab labelled "Ranged" in EQUI_AAWindow.xml. It is currently the ONLY
-- AA on that tab; the rest of the Ranged tree is still to be designed.
--
--   host 32  Finishing Blow  ranks 119,120,121,440,441  -> Kindred Bond
--
-- ⚠️ Rank id 119 is the ONLY join to zone/aotv4_pet_aa.cpp and nothing checks it.
--
-- PASSIVE, so unlike Sanctified Blow and Sanguine Frenzy it may carry aa_rank_effects -- the "no
-- effect rows" requirement applies only to ACTIVATED abilities. Ranks 3 and 4 use that: they are
-- native pet SPAs and need no code at all.
--
--   rank 1  you gain your pet's ward                              [code]
--   rank 2  ...and so does your group                             [code]
--   rank 3  your pet is hardier          SPA 213 PetMaxHP         [native]
--   rank 4  your pet strikes true        SPA 218 PetCriticalHit   [native]
--   rank 5  the ward outlives the pet by a minute                 [code]
--
-- ⚠️ THE WARD IS APPLIED AT SUMMON TIME. Buying or ranking up this AA does nothing to a pet that is
-- already out -- the owner must re-summon. That is a deliberate simplification: the alternative is
-- polling every owner, and a pet is cheap to re-summon.
--
--   mysql -h127.0.0.1 -upeq -ppeqpass peq < .devcontainer/custom/sql/aotv4_aa_ranged_kindred.sql
--
-- AAs are NOT in shared memory -- a zone restart applies this. The pet ward SPELLS are, so
-- aotv4_pet_wards.sql additionally needs ./shared_memory with world down.
-- =============================================================================================

UPDATE aa_ability SET name='Kindred Bond', classes=65535, enabled=1, type=3 WHERE id=32;

-- Same ladder as every other tree.
UPDATE aa_ranks SET level_req=5,  cost=3 WHERE id=119;
UPDATE aa_ranks SET level_req=15, cost=4 WHERE id=120;
UPDATE aa_ranks SET level_req=25, cost=5 WHERE id=121;
UPDATE aa_ranks SET level_req=35, cost=6 WHERE id=440;
UPDATE aa_ranks SET level_req=45, cost=8 WHERE id=441;

-- Stop at rank 5; natively this line runs to six (442).
UPDATE aa_ranks SET next_id=-1 WHERE id=441;

DELETE FROM aa_rank_effects WHERE rank_id IN (119,120,121,440,441);

-- Ranks 3-5 also carry native pet bonuses. Ranks 1 and 2 are pure code, so they get no rows -- a
-- passive AA with no effects is perfectly legal, it simply does nothing on its own.
--   SPA 213 PetMaxHP       -- percent increase to the pet's maximum health
--   SPA 218 PetCriticalHit -- baseline critical chance for the pet
INSERT INTO aa_rank_effects (rank_id,slot,effect_id,base1,base2) VALUES
 (121,1,213,10,0),
 (440,1,213,10,0),(440,2,218,5,0),
 (441,1,213,15,0),(441,2,218,8,0);

UPDATE db_str SET value='Kindred Bond' WHERE id=119 AND type=1;
UPDATE db_str SET value='What your companion is, you become a little of yourself. While your pet lives you carry its ward as well, whether that is the fire elemental burning aura, the air elemental swiftness, or the quiet clarity of a familiar. The second rank extends the ward to your group. The third and fourth make your companion hardier and surer of its blows, and at the fifth rank the ward outlives the pet by a minute. The ward is granted when the pet is summoned, so summon again after training this.' WHERE id=119 AND type=4;
