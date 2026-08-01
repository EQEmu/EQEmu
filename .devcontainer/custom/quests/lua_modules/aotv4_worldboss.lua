-- aotv4_worldboss.lua -- roaming world boss encounter.
--
-- A boss is armed for a random classic dungeon, the whole server is told, and everyone who helps
-- kill it can roll on its corpse. There is no raid: loot rights are granted directly to every
-- player who did damage, which is simpler, disturbs nobody's existing group, and covers people who
-- turn up late.
--
-- ⚠️ WHY IT SPAWNS LAZILY. You cannot spawn an NPC into a zone nobody is standing in: zones here are
-- dynamic (eqlaunch boots them on demand and idle ones self-terminate), so an empty zone has no
-- process to spawn into, and eq.spawn2 is zone-local anyway. So arming and spawning are separate:
-- the command records WHERE the boss will be and announces it, and the first player through the
-- door actually creates it. If nobody goes before the expiry, the encounter quietly lapses.
--
-- Data lives in ONE global bucket, aotv4_worldboss = "zoneshort|npcid|expiry_epoch", so every zone
-- process reads the same state and it survives restarts.

local M = {}

M.BUCKET      = "aotv4_worldboss"
M.NPC_ID      = 2000200        -- #The_Nameless (custom/sql/aotv4_worldboss.sql)
M.WINDOW_MINS = 60             -- how long the boss stays armed before the encounter lapses

-- Classic dungeons only. Derived from the DB: expansion 0, 25+ spawn points, ZERO merchant NPCs
-- (a good proxy for "not a city"), minus the gated planes. Cities are excluded because a boss in
-- the middle of Freeport is a guard-fight, not an encounter.
-- ⚠️ Region-locked progression (regions.cpp) still applies -- players who have not unlocked the
-- region cannot reach the boss. Keep this list inside the regions that are actually open.
M.ZONES = {
	{ z = "befallen",   n = "Befallen" },
	{ z = "beholder",   n = "Gorge of King Xorbb" },
	{ z = "blackburrow", n = "Blackburrow" },
	{ z = "cazicthule", n = "Accursed Temple of Cazic-Thule" },
	{ z = "crushbone",  n = "Crushbone" },
	{ z = "gukbottom",  n = "The Ruins of Old Guk" },
	{ z = "guktop",     n = "The City of Guk" },
	{ z = "hole",       n = "The Hole" },
	{ z = "kedge",      n = "Kedge Keep" },
	{ z = "mistmoore",  n = "The Castle of Mistmoore" },
	{ z = "najena",     n = "Najena" },
	{ z = "paw",        n = "The Lair of the Splitpaw" },
	{ z = "permafrost", n = "The Permafrost Caverns" },
	{ z = "unrest",     n = "The Estate of Unrest" },
}

--------------------------------------------------------------------- state
local function now() return os.time() end

local function read()
	local raw = eq.get_data(M.BUCKET)
	if not raw or raw == "" then return nil end
	local zone, npc, expiry = raw:match("^([^|]*)|([^|]*)|([^|]*)$")
	if not zone then return nil end
	return { zone = zone, npc = tonumber(npc), expiry = tonumber(expiry) or 0 }
end

local function write(zone, npc, expiry)
	eq.set_data(M.BUCKET, string.format("%s|%d|%d", zone, npc, expiry))
end

function M.clear() eq.delete_data(M.BUCKET) end

-- What is armed right now, or nil. Lapsed encounters are cleaned up on read so nothing has to poll.
function M.pending()
	local s = read()
	if not s then return nil end
	if s.expiry > 0 and now() > s.expiry then
		M.clear()
		return nil
	end
	return s
end

--------------------------------------------------------------------- arming
-- zone_short optional; picks at random when omitted. Returns the zone entry, or nil if the name
-- given is not one of ours.
function M.arm(zone_short)
	local pick
	if zone_short and zone_short ~= "" then
		for _, z in ipairs(M.ZONES) do
			if z.z == zone_short then pick = z break end
		end
		if not pick then return nil end
	else
		pick = M.ZONES[math.random(#M.ZONES)]
	end

	write(pick.z, M.NPC_ID, now() + (M.WINDOW_MINS * 60))

	eq.world_emote(15, string.format(
		"A terrible presence stirs in %s. Something nameless has awoken -- gather your strength and go put it down.",
		pick.n))

	return pick
end

--------------------------------------------------------------------- spawning
-- Called from global_player event_enter_zone. Spawns on top of whoever arrives first: it guarantees
-- valid coordinates in any zone (no per-zone spawn point table to maintain) and means the boss is
-- always found rather than sitting unseen in a corner.
function M.on_enter_zone(e)
	local s = M.pending()
	if not s or s.zone ~= eq.get_zone_short_name() then return end

	-- Someone else may have beaten us in; clear FIRST so two simultaneous arrivals cannot both spawn.
	M.clear()

	local c = e.self
	eq.spawn2(s.npc, 0, 0, c:GetX(), c:GetY(), c:GetZ(), c:GetHeading())

	eq.zone_emote(15, "The air splits open. Something nameless drags itself into the world.")
end

--------------------------------------------------------------------- loot rights
-- Called from global_npc event_death_complete. Everyone who landed a hit may loot, up to the 72
-- MAX_LOOTERS a corpse can hold. No raid is involved -- this is what makes "everyone rolls" work.
function M.on_death(e)
	if not e.self or e.self:GetNPCTypeID() ~= M.NPC_ID then return end

	local corpse = eq.get_entity_list():GetCorpseByName(e.self:GetCleanName())
	if not corpse or not corpse.valid then return end

	local granted, hate = 0, e.self:GetHateList()
	for _, h in ipairs(hate) do
		local m = h.ent
		if m and m.valid and m:IsClient() and granted < 72 then
			corpse:AddLooter(m)
			granted = granted + 1
			m:Message(15, "You have earned a share of the spoils. Roll for what it carried.")
		end
	end

	eq.world_emote(15, string.format(
		"The nameless horror has fallen in %s. %d champions share the spoils.",
		eq.get_zone_long_name(), granted))
end

return M
