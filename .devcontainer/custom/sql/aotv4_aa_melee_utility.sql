-- aotv4_aa_melee_utility.sql -- three utility AAs for the Melee tree.
-- =============================================================================================
--   host 104 Double Riposte  ranks 247,248,249,504,505    -> Backs to the Wall  [MARKER]
--   host  89 Soul Abrasion   ranks 210,211,212,1316,1317  -> Bracing            [MARKER]
--   host 108 Flurry          ranks 255,256,257,542,543    -> Run Them Down      [MARKER]
--
-- All three PASSIVE, all three markers -- the behaviour is in zone/aotv4_melee_aa.cpp and rank ids
-- 247, 210 and 255 are the ONLY join. Chains walked; title_sid and desc_sid confirmed equal to
-- first_rank_id on all three (the trap that made Overload show Quick Damage's description).
--
-- ⚠️ NONE OF THESE IS A STUN OR A PROC, deliberately. Stuns are already reachable through the spell
-- and combat-ability rewards, and weapon procs already carry a great deal of this server's power
-- budget. These three add reach into situations nothing else covers.
--
--   mysql -h127.0.0.1 -upeq -ppeqpass peq < .devcontainer/custom/sql/aotv4_aa_melee_utility.sql
--
-- No new spells, so no shared memory rebuild -- a zone restart is enough. db_str changes still need
-- ./export_client_files and dbstr_us.txt copied to the EQ root.
-- =============================================================================================

UPDATE aa_ability SET name='Backs to the Wall', classes=65535, enabled=1, type=4 WHERE id=104;
UPDATE aa_ability SET name='Bracing',           classes=65535, enabled=1, type=4 WHERE id=89;
UPDATE aa_ability SET name='Run Them Down',     classes=65535, enabled=1, type=4 WHERE id=108;

UPDATE aa_ranks SET level_req=5,  cost=3 WHERE id IN (247, 210, 255);
UPDATE aa_ranks SET level_req=15, cost=4 WHERE id IN (248, 211, 256);
UPDATE aa_ranks SET level_req=25, cost=5 WHERE id IN (249, 212, 257);
UPDATE aa_ranks SET level_req=35, cost=6 WHERE id IN (504, 1316, 542);
UPDATE aa_ranks SET level_req=45, cost=8 WHERE id IN (505, 1317, 543);

UPDATE aa_ranks SET next_id=-1 WHERE id IN (505, 1317, 543);

DELETE FROM aa_rank_effects WHERE rank_id IN
 (247,248,249,504,505, 210,211,212,1316,1317, 255,256,257,542,543);

UPDATE db_str SET value='Backs to the Wall' WHERE id=247 AND type=1;
UPDATE db_str SET value='Bracing'           WHERE id=210 AND type=1;
UPDATE db_str SET value='Run Them Down'     WHERE id=255 AND type=1;

-- ⚠️ Backs to the Wall gives NOTHING against a single attacker, on purpose: a defensive passive that
-- pays out when things are going well can be arranged by pulling carefully. This one only rewards
-- the situation melee actually ends up in. Reduction is FLAT (percentages are imperceptible against
-- this server's post-mitigation numbers) and capped at four extra attackers.
UPDATE db_str SET value='The more of them there are, the harder you are to put down. Every enemy fighting you beyond the first reduces each melee blow against you by 1, 2, 2, 3 and 3 points, counting up to four of them. It does nothing at all when only one thing is attacking you.' WHERE id=247 AND type=4;

-- ⚠️ Melee push only. Spell knockback takes a different path and is NOT covered -- stated here so
-- nobody discovers it mid-fight.
UPDATE db_str SET value='You are not easily moved. Blows can no longer shove you out of position. This holds against being pushed by weapons; it does not stop magic that hurls you.' WHERE id=210 AND type=4;

UPDATE db_str SET value='Nothing you have your hands on gets to leave. Enemies fighting you will not turn and run, however badly the fight is going for them.' WHERE id=255 AND type=4;
