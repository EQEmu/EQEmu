-- AoTv4 -- fix the HOTKEY text of every renamed alternate ability.
-- =====================================================================================
-- Symptom: the AA window showed our name ("Iron Will") while a hotkey made from that same ability
-- showed the NATIVE host's name ("Frenzied Burnout"). Also seen as Second Wind -> "Rabid Bear" and
-- Fade -> "Impr. Fam.".
--
-- ⚠️⚠️ AN AA HAS *THREE* INDEPENDENT NAMES IN db_str, AND RENAMING ONE RENAMES NOTHING ELSE.
-- aa_ranks carries title_sid AND upper_hotkey_sid AND lower_hotkey_sid, and the client resolves
-- them out of its own dbstr_us.txt by (id, type):
--
--     type 1   the title, shown in the AA window          <- aotv4_aa_rename.sql already fixed this
--     type 2   the hotkey's UPPER line                    <- still the host's text
--     type 3   the hotkey's LOWER line                    <- still the host's text
--     type 4   the description                            <- aotv4_aa_rename.sql already fixed this
--
-- ⚠️ THE HOTKEY NAME IS SPLIT ACROSS TWO ROWS because the hotbutton draws two short lines. That is
-- why the fault resisted searching: grepping db_str (or dbstr_us.txt) for the string that was
-- actually on screen, 'Frenzied Burnout', returns NOTHING -- it is stored as 'Frenzied' + 'Burnout'.
-- Search for a single word, or dump every type for the sid, which is what finally exposed it.
--
-- ⚠️ In our pool upper_hotkey_sid, lower_hotkey_sid and title_sid all happen to be the SAME sid, so
-- types 1, 2 and 3 are three rows of one id. That makes it look like fixing the title should have
-- fixed the hotkey; it does not, because the TYPE differs, and the type is what the client selects on.
--
-- ⚠️ Only ACTIVATED abilities have a hotkey. Passives carry upper/lower_hotkey_sid = -1 and are
-- skipped here; writing db_str rows for sid -1 would be junk.
--
-- Splitting rule: first word on the upper line, the remainder on the lower line, and a single-word
-- name leaves the lower line empty. That matches how the native rows are written ('Frenzied' /
-- 'Burnout') and keeps each line short enough for the button.
--
-- ⚠️ AFTERWARDS the client files MUST be re-exported and dbstr_us.txt reinstalled -- the client reads
-- these strings from its OWN copy, never from the server. Nothing here reaches a player until that
-- file is replaced. Zones do not need restarting for the strings themselves.
--
-- Re-runnable: upserts keyed on db_str's (id, type) primary key; creates no ids of its own.

DROP TEMPORARY TABLE IF EXISTS aotv4_hk_chain;
DROP TEMPORARY TABLE IF EXISTS aotv4_hk;

-- Every rank of every enabled ability, carrying its ability's name. Walked via next_id because rank
-- ids are NOT contiguous (CLAUDE.md section 10) and aa_ranks has no aa_id column to join on.
CREATE TEMPORARY TABLE aotv4_hk_chain AS
WITH RECURSIVE chain AS (
    SELECT a.name AS nm, r.id AS rank_id, r.next_id,
           r.upper_hotkey_sid AS u, r.lower_hotkey_sid AS l
    FROM   aa_ability a
    JOIN   aa_ranks   r ON r.id = a.first_rank_id
    WHERE  a.enabled = 1
    UNION ALL
    SELECT c.nm, r.id, r.next_id, r.upper_hotkey_sid, r.lower_hotkey_sid
    FROM   chain c
    JOIN   aa_ranks r ON r.id = c.next_id
    WHERE  c.next_id > 0
)
SELECT nm, rank_id, u, l FROM chain;

-- One row per hotkey sid. Both the upper and lower sid are collected: they are the same value today,
-- but nothing guarantees that and a divergent pair would otherwise be half fixed.
CREATE TEMPORARY TABLE aotv4_hk (sid INT PRIMARY KEY, nm VARCHAR(128));
INSERT IGNORE INTO aotv4_hk (sid, nm) SELECT u, nm FROM aotv4_hk_chain WHERE u > 0;
INSERT IGNORE INTO aotv4_hk (sid, nm) SELECT l, nm FROM aotv4_hk_chain WHERE l > 0;

INSERT INTO db_str (id, type, value)
SELECT sid, 2, SUBSTRING_INDEX(nm, ' ', 1)
FROM   aotv4_hk
ON DUPLICATE KEY UPDATE value = VALUES(value);

INSERT INTO db_str (id, type, value)
SELECT sid, 3,
       CASE WHEN LOCATE(' ', nm) > 0 THEN SUBSTRING(nm, LOCATE(' ', nm) + 1) ELSE '' END
FROM   aotv4_hk
ON DUPLICATE KEY UPDATE value = VALUES(value);

-- Verify: title, upper and lower must now agree for every activated ability.
SELECT h.sid,
       h.nm                       AS ability_name,
       dt.value                   AS window_title,
       du.value                   AS hotkey_upper,
       dl.value                   AS hotkey_lower,
       CASE WHEN CONCAT_WS(' ', NULLIF(du.value,''), NULLIF(dl.value,'')) = h.nm
            THEN 'ok' ELSE 'MISMATCH' END AS status
FROM   aotv4_hk h
LEFT JOIN db_str dt ON dt.id = h.sid AND dt.type = 1
LEFT JOIN db_str du ON du.id = h.sid AND du.type = 2
LEFT JOIN db_str dl ON dl.id = h.sid AND dl.type = 3
ORDER BY h.sid;

DROP TEMPORARY TABLE IF EXISTS aotv4_hk_chain;
DROP TEMPORARY TABLE IF EXISTS aotv4_hk;
