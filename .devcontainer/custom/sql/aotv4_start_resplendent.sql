-- AoTv4 -- disable the tutorial and start every new character in The Resplendent Temple.
-- =====================================================================================
-- Starting hub is zone 729 `resplendent` at x -22, y 535, z 0, heading 0.
--
-- ⚠️⚠️ `resplendent` SHIPS WITH min_status = 80, i.e. GM ONLY. Left alone, every new character
-- would be created pointing at a zone they are not allowed to enter -- which does not fail at
-- creation, it fails later at zone-in, so it would look like a broken character rather than a
-- permissions setting. This is the single most important line in the file.
-- ⚠️ Its min_expansion / max_expansion are already -1 (always spawns), so the Classic content
-- filter does NOT hide it. `expansion = 18` is metadata and gates nothing.
--
-- ⚠️⚠️ START ZONES ARE MATCHED ON THE EXACT (class, deity, race) TRIPLE.
-- world/worlddb.cpp queries `player_class = X AND player_deity = Y AND player_race = Z` -- there is
-- NO wildcard row, so a 0/0/0 catch-all would match nobody. Every creatable combination needs its
-- own row: 13 of the 301 combinations `char_create_combinations` allows had none (a consequence of
-- opening all 16 classes, section 14 -- the stock table never covered those pairings), and those
-- characters would log "No start zone found for class/deity/race" and be created with an unset
-- bind. Both halves below are required: UPDATE the 293 that exist, INSERT the 13 that do not.
--
-- ⚠️ `bind_id` MUST be 0. When it is non-zero the loader IGNORES x/y/z entirely and uses the bind
-- zone's safe point instead, so a leftover bind_id would silently discard the coordinates above.
-- ⚠️ x/y/z/heading must not ALL be zero either -- that is the loader's "use the zone safe point"
-- sentinel. Here y is 535 so the row is safe, but a hub whose coords were genuinely 0,0,0 could not
-- be expressed in this table at all.
--
-- ⚠️ Re-runnable. The INSERT is keyed on what is missing, so a second run adds nothing.

-- ---------------------------------------------------------------- 1. make the hub enterable
UPDATE zone SET min_status = 0 WHERE zoneidnumber = 729;

-- ---------------------------------------------------------------- 2. turn the tutorial off
-- ⚠️ EnableTutorialButton false makes world SKIP the tutorial branch entirely rather than reject the
-- request, so a client that still sends one simply enters at its normal start point. It does not
-- kick or log a possible-hack, which is what the rejection path does when the rule is on but the
-- character is not eligible.
-- ⚠️ MaxLevelForTutorial 0 is belt and braces: it is the second condition in that same branch, so
-- the tutorial stays unreachable even if the button rule is ever flipped back on by hand.
UPDATE rule_values SET rule_value = 'false' WHERE rule_name = 'World:EnableTutorialButton';
UPDATE rule_values SET rule_value = '0'     WHERE rule_name = 'World:MaxLevelForTutorial';

-- ---------------------------------------------------------------- 3. point every start row at the hub
UPDATE start_zones
SET    zone_id = 729,
       x = -22, y = 535, z = 0, heading = 0,
       bind_id = 0,
       bind_x = -22, bind_y = 535, bind_z = 0;

-- ---------------------------------------------------------------- 4. cover the combos that had no row
INSERT INTO start_zones
      (x, y, z, heading, zone_id, bind_id, player_choice, player_class, player_deity, player_race,
       start_zone, bind_x, bind_y, bind_z, select_rank, min_expansion, max_expansion)
SELECT -22, 535, 0, 0, 729, 0, 0, c.class, c.deity, c.race,
       0, -22, 535, 0, 0, -1, -1
FROM  (SELECT DISTINCT class, race, deity FROM char_create_combinations) c
LEFT  JOIN start_zones s
       ON s.player_class = c.class AND s.player_race = c.race AND s.player_deity = c.deity
WHERE s.zone_id IS NULL;

-- ---------------------------------------------------------------- verify
SELECT (SELECT min_status FROM zone WHERE zoneidnumber = 729)                       AS hub_min_status,
       (SELECT rule_value FROM rule_values WHERE rule_name='World:EnableTutorialButton'
          AND ruleset_id = 1)                                                        AS tutorial_button,
       (SELECT COUNT(*) FROM start_zones WHERE zone_id <> 729)                       AS rows_not_pointing_at_hub,
       (SELECT COUNT(*) FROM (
           SELECT DISTINCT c.class FROM char_create_combinations c
           LEFT JOIN start_zones s ON s.player_class=c.class AND s.player_race=c.race
                                  AND s.player_deity=c.deity
           WHERE s.zone_id IS NULL) z)                                               AS combos_still_uncovered;
