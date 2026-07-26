-- aotv4_aa_customonly.sql -- show ONLY the custom AA trees in the AA window.
-- =============================================================================================
-- Hides every native AA so the custom trees can be judged on their own, without 1,500 stock
-- abilities around them. Purely presentational: nothing about the custom AAs changes.
--
-- The switch is aa_ability.enabled -- zone/aa.cpp:1779 loads with
--     AaAbilityRepository::GetWhere(*this, "`enabled` = 1")
-- so a disabled AA is never loaded at all, rather than being loaded and filtered later.
--
-- ⚠️ REVERSIBLE, and the undo is exact. One native AA is already disabled in stock PEQ; blanket
-- re-enabling everything would quietly turn it on. So the ids that were ALREADY disabled are
-- recorded in aotv4_aa_disabled_backup first, and the undo re-disables exactly those.
--
-- ⚠️ Characters keep any native AAs they already bought -- the character_alternate_abilities rows
-- are untouched. Those AAs simply stop loading, so they neither show nor apply until re-enabled.
--
-- ⚠️ AAs load at ZONE BOOT. Restart zones after running this.
--
--   mysql -h127.0.0.1 -upeq -ppeqpass peq < .devcontainer/custom/sql/aotv4_aa_customonly.sql
--
-- TO UNDO (restores the native list exactly as it was):
--   UPDATE aa_ability SET enabled = 1 WHERE id NOT BETWEEN 40000 AND 49999;
--   UPDATE aa_ability a JOIN aotv4_aa_disabled_backup b ON b.aa_id = a.id SET a.enabled = 0;
-- =============================================================================================

-- Remember what was already off, so the undo does not turn it on.
CREATE TABLE IF NOT EXISTS aotv4_aa_disabled_backup (
  aa_id INT UNSIGNED NOT NULL PRIMARY KEY
) ENGINE=InnoDB;

INSERT IGNORE INTO aotv4_aa_disabled_backup (aa_id)
SELECT id FROM aa_ability WHERE enabled = 0 AND id NOT BETWEEN 40000 AND 49999;

-- Hide every native AA. 40000-49999 is reserved for the custom trees (tank 40000-40005; healer,
-- ranged dps and melee dps to follow), so the band is excluded wholesale rather than by id.
UPDATE aa_ability SET enabled = 0 WHERE id NOT BETWEEN 40000 AND 49999;

-- ...and make sure every custom one is on.
UPDATE aa_ability SET enabled = 1 WHERE id BETWEEN 40000 AND 49999;
