-- hotzones.lua -- AoTv4 daily-rotating Hot Zones: ONE PER REGION, 1.5x EXP.
--
-- Deterministic from the calendar date, so no cron or "midnight roll" is needed: every zone process
-- computes today's set the same way, which means the #hotzone popup, the connect welcome message and
-- the actual XP apply (global_player.event_enter_zone -> eq.set_hotzone) always agree.
--
-- ⚠️⚠️ THE MULTIPLIER IS A RULE, NOT A NUMBER IN THIS FILE. exp.cpp does:
--     modifier = Character:ExpMultiplier (0.65);  if IsHotzone(): modifier += Zone:HotZoneBonus
-- so the hot rate is (ExpMultiplier + HotZoneBonus) / ExpMultiplier.
--     2.0x needs HotZoneBonus = 0.650   (0.65 + 0.65  = 1.300)
--     1.5x needs HotZoneBonus = 0.325   (0.65 + 0.325 = 0.975)
-- ⚠️ HotZoneBonus is expressed against ExpMultiplier, NOT as a multiplier itself -- if ExpMultiplier
-- ever moves, this has to move with it or the hot rate silently changes.
--
-- ⚠️⚠️ THE POOL IS THE REGION MAP, NOT A CURATED LIST. The old pool was hand-written before the
-- region system and named zones like chardok, everfrost, najena, sebilis and the Feerrott -- all of
-- which the 0.1.1 balance pass put in region 99, meaning NOBODY CAN ENTER THEM. A hot zone nobody
-- can reach is worse than no hot zone: it advertises a bonus that cannot be collected.
--
-- ⚠️⚠️ TOWNS ARE EXCLUDED BY NAME, and it has to be by name. Merchant density does not separate them
-- here: Greater Faydark shows 63 merchants because it IS Kelethin, Rathe Mountains 60, East
-- Commonlands 39 for the trading tunnel -- all real hunting zones. The excluded list is:
--     freporte freportn freportw   Freeport city
--     qeynos qeynos2 qrg           Qeynos city and Surefall Glade
--     cabeast cabwest              Cabilis city
--     thurgadina                   City of Thurgadin
--     rivervale                    Rivervale
-- Greater Faydark is KEPT despite being Kelethin: it is also that region's main hunting ground.
--
-- ⚠️ Regenerate after any change to zone_regions (the town list is the NOT IN clause):
--     SELECT zr.region_id, r.name, z.short_name, z.long_name
--     FROM zone_regions zr JOIN regions r ON r.id=zr.region_id
--     JOIN zone z ON z.zoneidnumber=zr.zone_id AND z.version=0
--     WHERE zr.region_id BETWEEN 1 AND 6 AND z.short_name NOT IN (...towns...)
--     ORDER BY zr.region_id, z.long_name;
--
-- ⚠️ Region 0 (Resplendent and the delve zones) is deliberately absent -- the hub is not a place to
-- grind, and a delve is already scaled to the player. Region 99 is unreachable by anyone.

local M = {}

-- region id -> the hunting zones in it
M.REGION_ZONES = {
  [1] = {  -- Kelethin
    { s = "crushbone", l = "Crushbone" },
    { s = "cauldron", l = "Dagnor's Cauldron" },
    { s = "kedge", l = "Kedge Keep" },
    { s = "mistmoore", l = "The Castle of Mistmoore" },
    { s = "unrest", l = "The Estate of Unrest" },
    { s = "gfaydark", l = "The Greater Faydark" },
    { s = "lfaydark", l = "The Lesser Faydark" },
  },
  [2] = {  -- Freeport
    { s = "befallen", l = "Befallen" },
    { s = "ecommons", l = "East Commonlands" },
    { s = "beholder", l = "Gorge of King Xorbb" },
    { s = "kithicor", l = "Kithicor Forest" },
    { s = "misty", l = "Misty Thicket" },
    { s = "nro", l = "Northern Desert of Ro" },
    { s = "oasis", l = "Oasis of Marr" },
    { s = "oot", l = "Ocean of Tears" },
    { s = "sro", l = "Southern Desert of Ro" },
    { s = "runnyeye", l = "The Liberated Citadel of Runnyeye" },
    { s = "hateplaneb", l = "The Plane of Hate" },
    { s = "commons", l = "West Commonlands" },
  },
  [3] = {  -- Thurgadin
    { s = "eastwastes", l = "Eastern Wastes" },
    { s = "thurgadinb", l = "Icewell Keep" },
    { s = "crystal", l = "The Crystal Caverns" },
    { s = "greatdivide", l = "The Great Divide" },
    { s = "iceclad", l = "The Iceclad Ocean" },
    { s = "sleeper", l = "The Sleeper's Tomb" },
    { s = "frozenshadow", l = "The Tower of Frozen Shadow" },
    { s = "velketor", l = "Velketor's Labyrinth" },
  },
  [4] = {  -- Firiona Vie
    { s = "firiona", l = "Firiona Vie" },
    { s = "frontiermtns", l = "Frontier Mountains" },
    { s = "karnor", l = "Karnor's Castle" },
    { s = "droga", l = "Mines of Droga" },
    { s = "nurga", l = "Mines of Nurga" },
    { s = "dreadlands", l = "The Dreadlands" },
    { s = "swampofnohope", l = "The Swamp of No Hope" },
    { s = "timorous", l = "Timorous Deep" },
  },
  [5] = {  -- Qeynos
    { s = "blackburrow", l = "Blackburrow" },
    { s = "eastkarana", l = "Eastern Plains of Karana" },
    { s = "paw", l = "Lair of the Splitpaw" },
    { s = "lakerathe", l = "Lake Rathetear" },
    { s = "northkarana", l = "The Northern Plains of Karana" },
    { s = "qcat", l = "The Qeynos Aqueduct System" },
    { s = "qeytoqrg", l = "The Qeynos Hills" },
    { s = "rathemtn", l = "The Rathe Mountains" },
    { s = "southkarana", l = "The Southern Plains of Karana" },
    { s = "qey2hh1", l = "The Western Plains of Karana" },
  },
  [6] = {  -- Cabilis
    { s = "kurn", l = "Kurn's Tower" },
    { s = "lakeofillomen", l = "Lake of Ill Omen" },
    { s = "dalnir", l = "The Crypt of Dalnir" },
    { s = "fieldofbone", l = "The Field of Bone" },
    { s = "warslikswood", l = "The Warsliks Woods" },
    { s = "veksar", l = "Veksar" },
  },
}

M.REGION_NAME = {
  [1] = "Kelethin", [2] = "Freeport", [3] = "Thurgadin",
  [4] = "Firiona Vie", [5] = "Qeynos", [6] = "Cabilis",
}

-- monotonic day number; changes at local midnight (rolls the set each calendar day)
local function day_seed()
  local t = os.date("*t")
  return t.year * 366 + t.yday
end

-- Today's hot set: exactly ONE zone from each region.
--
-- ⚠️ Each region advances through its OWN list at its own offset, seeded by the day and the region
-- id. Using the same offset for every region would march them in lockstep -- region 1 and region 5
-- would always sit at the same index -- so the rotation would repeat far sooner than the list
-- lengths suggest.
function M.today()
  local out = {}
  for rid = 1, 6 do
    local list = M.REGION_ZONES[rid]
    if list and #list > 0 then
      local idx = (day_seed() + rid * 7) % #list + 1
      local z = list[idx]
      out[#out + 1] = { s = z.s, l = z.l, region = M.REGION_NAME[rid] }
    end
  end
  return out
end

-- is the given zone short-name one of today's hot zones?
function M.is_hot(short)
  if not short or short == "" then return false end
  short = string.lower(short)
  for _, z in ipairs(M.today()) do
    if z.s == short then return true end
  end
  return false
end

return M
