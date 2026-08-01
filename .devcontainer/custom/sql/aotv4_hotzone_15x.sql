-- AoTv4 -- hot zones pay 1.5x instead of 2x.
-- =====================================================================================
-- ⚠️⚠️ HotZoneBonus IS AN ADDITION TO ExpMultiplier, NOT A MULTIPLIER. zone/exp.cpp does:
--     modifier = Character:ExpMultiplier;   if (IsHotzone()) modifier += Zone:HotZoneBonus;
-- so the hot rate is (ExpMultiplier + HotZoneBonus) / ExpMultiplier:
--     2.0x  ->  0.65 + 0.650 = 1.300
--     1.5x  ->  0.65 + 0.325 = 0.975
-- Setting this to 1.5 would give (0.65 + 1.5) / 0.65 = 3.3x, not 1.5x -- the number here is NOT the
-- multiplier the player experiences.
-- ⚠️ It is expressed against Character:ExpMultiplier (0.65). If that ever moves, this must move with
-- it or the hot rate silently changes.
-- ⚠️ Rules are read at ZONE BOOT: zone restart, no shared memory rebuild.

UPDATE rule_values SET rule_value = '0.325'
WHERE  rule_name = 'Zone:HotZoneBonus';

SELECT r1.rule_value AS exp_multiplier, r2.rule_value AS hotzone_bonus,
       ROUND((r1.rule_value + r2.rule_value) / r1.rule_value, 3) AS effective_hot_rate
FROM   rule_values r1, rule_values r2
WHERE  r1.rule_name = 'Character:ExpMultiplier' AND r1.ruleset_id = 1
  AND  r2.rule_name = 'Zone:HotZoneBonus'       AND r2.ruleset_id = 1;
