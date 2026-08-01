-- AoTv4 -- town factions: nobody is killed in a town for being the wrong race.
-- =================================================================================================
-- All 16 races may now be any class and go anywhere (CLAUDE.md section 14 + the region system), but
-- stock EQ faction is built the opposite way round: an Ogre is born at -1000 with Kelethin and is
-- cut down on sight by the guards of a city the server tells him he is allowed to walk into.
--
-- WHAT ACTUALLY DECIDES IT. Every aggro branch in `NPC::CheckWillAggro` (zone/aggro.cpp:326 and
-- :516/:545) requires the con to be exactly FACTION_THREATENINGLY or FACTION_SCOWLS. Nothing else
-- reaches it -- and note `AlwaysAggro()` / `npc_aggro` sits in the ELIGIBILITY half of those
-- conditions, not the faction half, so an aggro-flagged NPC still needs a hostile con to swing.
-- The thresholds are rules (Faction:*FactionMinimum): below -750 Scowls, -750..-501 Threatening,
-- -500 and up Dubious. So a floor at DUBIOUS is the whole fix, and -500 is the exact boundary.
--
-- Dubious is deliberately the floor rather than Indifferent. The city still visibly dislikes you --
-- an Ogre is not welcome in Felwithe -- it simply does not murder you for existing, and
-- `Client::GetFactionLevel`'s own merchant fix (zone/client.cpp:8342) already treats Dubious as
-- tradeable, so shops keep working without a second exception.
--
-- WHY THIS IS A CON FLOOR AND NOT AN EDIT TO `faction_list_mod`. Clamping the 7,134 negative race
-- modifiers would change stored faction VALUES, which is what quests read.
-- ⚠️ 144 quest files test `e.other:GetFaction(e.self)`, and in EQ LOWER IS BETTER (Ally 1 .. Scowls
-- 9), so every one of them is a "good enough?" test at the opposite end of the scale from anything
-- this touches. The floor only ever moves 8/9 -> 7. The four quests that test the hostile end were
-- checked individually and all still pass:
--     erudnext/Collier.lua            >= 7   -- Dubious or worse; a floor OF 7 satisfies it
--     erudnext/Weligon_Steelherder.lua > 5
--     felwithea/Tynkale.lua            > 4
--     westwastes/Sontalak.lua         >= 5
-- ⚠️ Re-verify that list if the floor is ever raised above Dubious -- Collier is the one that breaks.
--
-- WHY A FACTION LIST AND NOT A ZONE LIST. Kelethin is not a zone: it is a treetop city inside
-- `gfaydark`, which is also a real hunting zone full of Crushbone orcs. A zone-scoped floor would
-- have to choose between leaving Kelethin's guards lethal and pacifying the orcs standing under it.
-- Scoping by the NPC's own faction separates the two cleanly, and covers every other city for free.
--
-- ⚠️⚠️ THE LIST IS DELIBERATELY NARROW: it is ONLY factions that can already be hostile purely by
-- race (some race sits below -500 before any earned faction), AND read as a civic body by name,
-- AND show commerce -- either merchants of their own or a majority of their NPCs standing in a zone
-- that has at least 10 merchants. That last test is what keeps dungeon and raid factions with civic
-- names OUT: Nest Guardians (2769 npcs), Thunder Guardians (2429), Citizens of Takish-Hiz (2257),
-- Inhabitants of Hate (900), Fallen Guard of Illsalin, Guardians of Veeshan and Befallen Inhabitants
-- all match the naming and all have zero merchants in non-city zones. Two real city guilds were
-- caught by that same test and are added back by hand: 275 Keepers of the Art (the Felwithe wizard
-- guild) and 266 High Council of Erudin.
-- ⚠️ Monster factions are NOT here and must never be added -- Crushbone Orcs, Sabertooths of
-- Blackburrow, Goblins of Mountain Death, Frogloks of Guk, Trakanon and Vox all stay lethal.
-- Adding one would make that content unaggressive server-wide, in every zone, with no other symptom.
--
-- ⚠️ The table is read at ZONE BOOT, not from shared memory (only `items` and `spells` are shared),
-- so editing it needs a zone restart and no `./shared_memory` rebuild.
-- ⚠️ `faction_list_mod`.`mod` is a MariaDB reserved word; it needs backticks wherever it appears.
-- Disable the whole mechanic at runtime with the rule AoT:TownFactionFloor.
-- ⚠️ Re-runnable.

CREATE TABLE IF NOT EXISTS aotv4_town_factions (
  faction_id INT         NOT NULL PRIMARY KEY,
  name       VARCHAR(64) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DELETE FROM aotv4_town_factions;
INSERT INTO aotv4_town_factions (faction_id, name) VALUES
  (220, 'Arcane Scientists'),
  (361, 'Ashen Order'),
  (440, 'Cabilis Residents'),
  (223, 'Circle of Unseen Hands'),
  (1719, 'Citizens of Gukta'),
  (228, 'Clurg'),
  (346, 'Commons Residents'),
  (230, 'Corrupt Qeynos Guards'),
  (231, 'Craftkeepers'),
  (233, 'Crimson Hands'),
  (241, 'Deeppockets'),
  (242, 'Deepwater Knights'),
  (370, 'Dreadguard Inner'),
  (334, 'Dreadguard Outer'),
  (244, 'Ebon Mask'),
  (326, 'Emerald Warriors'),
  (246, 'Faydarks Champions'),
  (254, 'Gate Callers'),
  (261, 'Green Blood Knights'),
  (263, 'Guardians of the Vale'),
  (262, 'Guards of Qeynos'),
  (1718, 'Guktan Elders'),
  (266, 'High Council of Erudin'),
  (267, 'High Guard of Erudin'),
  (332, 'Highpass Guards'),
  (248, 'Inhabitants of Firiona Vie'),
  (345, 'Karana Residents'),
  (275, 'Keepers of the Art'),
  (276, 'Kelethin Merchants'),
  (269, 'Kithicor Residents'),
  (280, 'Knights of Thunder'),
  (281, 'Knights of Truth'),
  (284, 'League of Antonican Bards'),
  (288, 'Merchants of Ak`Anon'),
  (289, 'Merchants of Erudin'),
  (325, 'Merchants of Felwithe'),
  (328, 'Merchants of Halas'),
  (290, 'Merchants of Kaladim'),
  (338, 'Merchants of Oggok'),
  (291, 'Merchants of Qeynos'),
  (292, 'Merchants of Rivervale'),
  (293, 'Miners Guild 249'),
  (322, 'Miners Guild 628'),
  (337, 'Oggok Guards'),
  (342, 'Order of Three'),
  (1713, 'Paladins of Gukta'),
  (298, 'Peace Keepers'),
  (340, 'Priests of Innoruuk'),
  (341, 'Priests of Life'),
  (362, 'Priests of Marr'),
  (300, 'Priests of Mischief'),
  (1709, 'Protectors of Gukta'),
  (302, 'Protectors of Pine'),
  (121, 'Qeynos Citizens'),
  (1597, 'Residents of Jaggedpine'),
  (308, 'Shadowknights of Night Keep'),
  (327, 'Shamen of Justice'),
  (394, 'Shamen of War'),
  (309, 'Silent Fist Clan'),
  (310, 'Soldiers of Tunare'),
  (401, 'Song Weavers'),
  (312, 'Storm Guard'),
  (330, 'The Freeport Militia'),
  (363, 'The Spurned'),
  (1714, 'Wizards of Gukta');

-- ⚠️⚠️ CABILIS NEEDED HAND-ADDING, AND THE REASON GENERALISES. The name test above is built from
-- Antonican and Faydwer vocabulary (Merchants of, Guards of, Knights of, Militia, Residents), and
-- NOTHING in Kunark is named that way -- so a per-hub coverage check found Cabilis protected only by
-- 'Cabilis Residents' while its actual city GUARD faction, Legion of Cabilis, was still killing
-- people on sight. Verified civic by commerce before adding, the same bar the automatic pass used:
--   441 Legion of Cabilis       1 merchant   the city guard -- this is the one that kills you
--   443 Brood of Kotiz         16 merchants  shaman guild
--   445 Scaled Mystics         12 merchants  mystic guild
--   442 Crusaders of Greenmist  3 merchants  shadowknight guild
--   444 Swift Tails             1 merchant   monk/rogue guild
--   272 Jaggedpine Treefolk     7 merchants  the Surefall Glade druid guild, outside Qeynos
-- 📌 CHECK COVERAGE PER CITY AFTER ANY CHANGE HERE rather than trusting the name test -- it is a
-- keyword list, not a classifier, and it silently under-covers anything not named in Common.
--
-- ⚠️ DELIBERATELY LEFT HOSTILE, having been checked and rejected: 446 The Forsaken (0 merchants,
-- only 6 of its 263 NPCs stand in Cabilis -- pacifying 257 wilderness mobs to fix 6 in town is the
-- wrong trade), 316 Tunare's Scouts (0 merchants, half its NPCs are inside Crushbone) and 283 Pack
-- of Tomar (0 merchants, Dreadlands). All three match the profile the commerce test exists to
-- reject. Re-check the merchant counts before ever promoting one of them.
INSERT INTO aotv4_town_factions (faction_id, name) VALUES
  (441, 'Legion of Cabilis'),
  (443, 'Brood of Kotiz'),
  (445, 'Scaled Mystics'),
  (442, 'Crusaders of Greenmist'),
  (444, 'Swift Tails'),
  (272, 'Jaggedpine Treefolk')
ON DUPLICATE KEY UPDATE name = VALUES(name);

-- Cabilis proved the keyword list under-covers, so the remaining gap was closed by SWEEPING on
-- commerce alone (no name test): any faction still missing with real merchants and a big majority of
-- its NPCs in zones that have 10+ merchants. That found six, and only three are towns.
--   265 Heretics             39 merchants, 93 percent city -- Paineel, a real city
--   320 Wolves of the North   3 merchants, 78 percent city -- the Halas shaman guild
--   255 Gem Choppers          1 merchant,  91 percent city -- gnome/dwarf guild, 22 NPCs
-- ⚠️⚠️ THE OTHER THREE ARE RAID CONTENT AND WERE REJECTED, which is why this sweep is not simply
-- run as the rule: 430 Claws of Veeshan (Skyshrine's dragons -- Great Divide and Skyshrine both have
-- merchants, so it scores 66 percent city) and the Seru factions 1485/1486/1487 (Sanctus Seru is
-- literally a city, and also a raid target). Commerce identifies civilisation, not friendliness.
-- Luclin and Velious are outside the open regions anyway, so nothing there is reachable to test.
INSERT INTO aotv4_town_factions (faction_id, name) VALUES
  (265, 'Heretics'),
  (320, 'Wolves of the North'),
  (255, 'Gem Choppers')
ON DUPLICATE KEY UPDATE name = VALUES(name);

INSERT INTO rule_values (ruleset_id, rule_name, rule_value, notes)
VALUES (1, 'AoT:TownFactionFloor', 'true',
        'Floor the faction con at Dubious for aotv4_town_factions, so no race is KOS in a town.')
ON DUPLICATE KEY UPDATE rule_value = VALUES(rule_value);

SELECT COUNT(*) AS town_factions FROM aotv4_town_factions;
