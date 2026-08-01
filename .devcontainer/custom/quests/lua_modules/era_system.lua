-- era_system.lua
-- Server-wide expansion progression. The FIRST character to MAX every AA in the current era's tier
-- unlocks the next expansion for EVERYONE: the level cap rises (live), the AA pool widens (live, via
-- aa_choice's era filter), and the content-expansion rule advances. See AOTV4_EXPANSION_PLAN.md.
--
-- Source of truth: a GLOBAL data bucket "aotv4_era" (no charid) = the unlocked era index 0..8.
-- Every zone reads it, so level/spell/AA gating is consistent server-wide and survives restarts.

local aapool = require("aa_pool")

local M = {}

-- era 0..8. cap = level cap; cap_exp = GetEXPForLevel(cap) (the death-bank reference, for tuning);
-- expansion = Expansion:CurrentExpansion value for content; name = broadcast label.
-- `spent` = the AA VALUE (sum of owned rank*cost across the pool) that UNLOCKS this era server-wide.
-- AAs are NOT era-gated anymore -- the whole pool is offered randomly (prereqs still enforced) from the
-- start. Expansions open smoothly as the first player's accumulated AA value crosses these thresholds.
M.ERA = {
	[0] = { name = "Classic",                  expansion = 0, cap = 50, cap_exp = 164708600,  spent = 0    },
	[1] = { name = "The Ruins of Kunark",      expansion = 1, cap = 60, cap_exp = 616137000,  spent = 220  },
	[2] = { name = "The Scars of Velious",     expansion = 2, cap = 60, cap_exp = 616137000,  spent = 485  },
	[3] = { name = "The Shadows of Luclin",    expansion = 3, cap = 60, cap_exp = 616137000,  spent = 740  },
	[4] = { name = "The Planes of Power",      expansion = 4, cap = 65, cap_exp = 812646400,  spent = 990  },
	[5] = { name = "The Legacy of Ykesha",     expansion = 5, cap = 65, cap_exp = 812646400,  spent = 1260 },
	[6] = { name = "Lost Dungeons of Norrath", expansion = 6, cap = 65, cap_exp = 812646400,  spent = 1540 },
	[7] = { name = "Gates of Discord",         expansion = 7, cap = 65, cap_exp = 812646400,  spent = 1820 },
	[8] = { name = "Omens of War",             expansion = 8, cap = 70, cap_exp = 1018377900, spent = 2100 },
}
local MAX_ERA = 8
local BUCKET  = "aotv4_era"   -- GLOBAL (no charid) -> server-wide unlocked era

function M.current()
	return tonumber(eq.get_data(BUCKET)) or 0
end

function M.def(era)
	return M.ERA[era] or M.ERA[0]
end

-- ⚠️⚠️ THE SERVER CAP IS 30 AND THE REGION SYSTEM OWNS IT, NOT THE ERA TABLE.
-- The era table's caps (Classic 50 ... Omens 70) predate the level 30 rework and its unlock trigger
-- has been orphaned since random AA was retired (nothing advances `aotv4_era`), so it was frozen at
-- Classic/50 and was the ONLY thing actually clamping anybody. The region ceiling -- the highest
-- `max_level` among the regions a character has unlocked, all of which are 30 -- is the real design,
-- but `GetRegionMaxLevel` had no caller anywhere and did nothing.
--
-- Both are consulted and the LOWER wins, so neither can raise the other's ceiling by accident.
-- ⚠️⚠️ 30, matching `regions.max_level` from the 0.1.1 region import. level_cap() takes the MINIMUM
-- of this, the era cap and the character's region ceiling, so if these two disagree the LOWER one
-- silently becomes the real cap and the other looks like it does nothing. They are kept equal so
-- there is one answer to "what is the cap", not two that happen to agree.
-- 📌 It was 35 earlier today, to match zone/exp.cpp's stated design ("629,000 at level 35"). The
-- 0.1.1 balance pass moved it to 30. The EXPERIENCE CURVE was never re-tuned for 30 -- it still
-- describes a 35 ceiling in its own comments -- so a full climb now ends at 495,000 rather than
-- 665,000, and anything calibrated against that figure (the AA conversion, in particular) is
-- proportionally generous by about a third.
M.HARD_CAP = 30

-- ⚠️⚠️ A REGION CEILING OF 0 MEANS "NO REGIONS UNLOCKED", NOT "MAX LEVEL 0".
-- RegionManager::GetMaxLevel returns 0 when the character holds no regions, and
-- `character_regions_unlocked` is EMPTY on this server today -- nothing grants a starting region
-- yet. Treating that 0 as a cap would pin every character on the server at level 0, which is why
-- this reads it as "unrestricted" and falls back to HARD_CAP.
-- 📌 When regions do start being granted, this keeps working unchanged: every region is max_level
-- 30, so the answer is 30 either way. Revisit only if a region above 30 is ever added.
function M.region_cap(client)
	if not client or not client.GetRegionMaxLevel then return M.HARD_CAP end
	local ok, v = pcall(function() return client:GetRegionMaxLevel() end)
	if not ok or not v or v <= 0 then return M.HARD_CAP end
	return v
end

function M.level_cap(client)
	local cap = M.def(M.current()).cap or M.HARD_CAP
	if cap > M.HARD_CAP then cap = M.HARD_CAP end
	if client then
		local r = M.region_cap(client)
		if r < cap then cap = r end
	end
	return cap
end

-- AA VALUE the player has accumulated = sum of (owned rank * cost) across the whole pool. This is the
-- deterministic "spent" metric that drives expansion unlocks -- native GetSpentAA mismeasures because
-- the picker grants AAs free (ignore_cost). Robust: reflects exactly what the player owns.
function M.spent(client)
	local total = 0
	for _, aa in ipairs(aapool[1]) do
		total = total + (client:GetAAByAAID(aa.id) or 0) * (aa.cost or 0)
	end
	return total
end

-- CATCH-UP bonus applied to death-AA banking: +CATCHUP_PER_ERA for each unlocked expansion the player's
-- AA VALUE is still short of. Lets latecomers close the gap on an advanced server without touching the
-- leaders' rate (a caught-up player, or a Classic-only server, gets nothing).
local CATCHUP_PER_ERA = 0.25
function M.catchup_bonus(client)
	local cur = M.current()
	if cur < 1 then return 1.0 end
	local mine, behind = M.spent(client), 0
	for era = 1, cur do
		if mine < (M.ERA[era].spent or 0) then behind = behind + 1 end
	end
	return 1.0 + behind * CATCHUP_PER_ERA
end

-- Best-effort: point THIS zone's content-expansion rule at the unlocked era. Runtime expansion
-- checks pick it up; a FULL content open (zones/items that filter at zone BOOT) still needs the
-- rule persisted in rule_values + a world restart -- see AOTV4_EXPANSION_PLAN.md 5d / custom SQL.
function M.sync_zone()
	local d = M.def(M.current())
	eq.set_rule("Expansion:CurrentExpansion", tostring(d.expansion))
	-- AoTv4: pin the NATIVE XP level cap to the era cap so players can't ding PAST it. exp.cpp lets a
	-- character reach UP TO Character:MaxExpLevel (the level grant is gated on check_level < MaxExpLevel+1),
	-- so MaxExpLevel = cap stops leveling exactly AT the era cap (cap-1 was an off-by-one that capped a
	-- level early). clamp_level (event_level_up) stays as a safety net.
	eq.set_rule("Character:MaxExpLevel", tostring(d.cap))
end

-- Called after a successful AA pick (aa_choice.handle_say). Advance the server-wide era while the
-- player's accumulated AA VALUE has crossed the next expansion's `spent` threshold. Cascades if a
-- single player vaults past several thresholds at once. First to cross opens it for everyone.
function M.check_unlock(client)
	local era = M.current()
	local spent = M.spent(client)
	while era < MAX_ERA and spent >= (M.ERA[era + 1].spent or math.huge) do
		era = era + 1
		eq.set_data(BUCKET, tostring(era))
		local d = M.def(era)
		eq.set_rule("Expansion:CurrentExpansion", tostring(d.expansion))
		eq.set_rule("Character:MaxExpLevel", tostring(d.cap))       -- AoTv4: raise the native XP cap with the era (reach = cap)
		eq.world_emote(15, string.format(
			"%s has pushed the age forward!  %s is now open to all -- the level cap rises to %d!",
			client:GetName(), d.name, d.cap))
	end
end

-- Clamp a character to the current era's level cap (called from event_level_up). Returns true if it
-- clamped (caller skips the spell offer). We also hold exp at the cap threshold so the engine does
-- not bounce-level every kill, and so a death at the cap banks the tuned amount (not an ever-growing
-- pile from grinding at cap).
function M.clamp_level(client)
	-- ⚠️ Pass the client: the cap is now per character (region ceiling), not server wide.
	local cap = M.level_cap(client)
	if client:GetLevel() > cap then
		client:SetLevel(cap)
		-- ⚠️⚠️ THE EXP MUST BE PINNED OR THE CLAMP FIGHTS ITSELF. Experience keeps accruing at cap, so
		-- a character held at the cap without pinning re-crosses the threshold on the very next kill,
		-- fires event_level_up again, and gets clamped again -- forever.
		-- ⚠️ Pinned at the exp for the ACTUAL cap, computed from the client. The era table's `cap_exp`
		-- was hand written for ITS caps (50/60/65/70) and is far too high for 30: writing it would
		-- hold a level 30 character on the experience of a level 50 one, and since a death banks AA
		-- from total run experience, it would also pay out an enormous bank on every death.
		local cap_exp = client:GetEXPForLevel(cap)
		if cap_exp and cap_exp > 0 and (client:GetEXP() or 0) > cap_exp then
			client:SetEXP(cap_exp, client:GetAAExp())
		end
		return true
	end
	return false
end

return M
