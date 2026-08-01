-- aotv4_worldbuff.lua -- server-wide buffs.
--
-- A GM arms a spell; everybody online gets it, everybody who logs in or zones during the window
-- gets it, and it stops when the window lapses.
--
-- ⚠️ WHY THIS IS ARM-AND-APPLY RATHER THAN ONE BROADCAST. Lua can only enumerate clients in ITS OWN
-- zone (eq.get_entity_list():GetClientList()), and every zone is a separate process. The cross-zone
-- bindings that exist are all targeted -- cross_zone_cast_spell_by_char_id / _client_name /
-- _group_id / _guild_id / _raid_id / _expedition_id -- there is no "by everyone". So there is no
-- single call that reaches the whole server, and one has to be built out of per-zone work.
--
-- The state lives in ONE global bucket, aotv4_worldbuff = "spellid|expiry_epoch", so every zone
-- process reads the same thing and it survives restarts. Same shape as aotv4_worldboss.
--
-- Three things apply it, which between them cover everyone:
--   1. the command itself       -- everyone standing in the GM's zone, immediately
--   2. event_connect            -- anyone who logs in while the window is open
--   3. event_enter_zone         -- anyone who zones while the window is open
--   4. the "worldbuff" timer    -- a periodic sweep of each zone, which is what catches the player
--                                 parked in one spot who never zones and never relogs. Without it
--                                 that player is the one hole in the coverage.

local M = {}

M.BUCKET       = "aotv4_worldbuff"
M.WINDOW_MINS  = 30      -- how long new arrivals keep receiving it
M.SWEEP_SECS   = 30      -- how often each zone re-checks its own occupants

--------------------------------------------------------------------- state
local function now() return os.time() end

local function read()
	local raw = eq.get_data(M.BUCKET)
	if not raw or raw == "" then return nil end
	local spell, expiry = raw:match("^([^|]*)|([^|]*)$")
	if not spell then return nil end
	local s = { spell = tonumber(spell), expiry = tonumber(expiry) or 0 }
	if not s.spell or s.spell <= 0 then return nil end
	return s
end

function M.clear() eq.delete_data(M.BUCKET) end

-- Live state, or nil if nothing is armed or the window has lapsed. Lapsed state is cleared here so
-- it does not sit in the bucket forever.
function M.pending()
	local s = read()
	if not s then return nil end
	if s.expiry <= now() then
		M.clear()
		return nil
	end
	return s
end

--------------------------------------------------------------------- applying
-- ⚠️ ApplySpellBuff, NOT SpellFinished. SpellFinished runs the full cast: it can be RESISTED, it
-- checks target type, and a beneficial spell with the wrong targettype simply will not land on an
-- arbitrary player. ApplySpellBuff puts the buff straight on, which is what "the GM said everyone
-- gets this" should mean. It is also why any spell id works rather than only self/single-target ones.
local function give(client, spell)
	if not client or not client.valid then return end
	-- Already have it: leave it alone rather than resetting the duration every sweep, which would
	-- make a 10-minute buff last as long as the window.
	if client:FindBuff(spell) then return end
	client:ApplySpellBuff(spell)
end

-- One player. Safe to call from anywhere; does nothing when nothing is armed.
function M.apply_to(client)
	local s = M.pending()
	if not s then return end
	give(client, s.spell)
end

-- Everyone in THIS zone. Called by the command (so the GM's zone is covered at once) and by the
-- periodic sweep (so parked players are covered eventually).
function M.apply_zone()
	local s = M.pending()
	if not s then return 0 end

	local n = 0
	local list = eq.get_entity_list():GetClientList()
	for c in list.entries do
		if c and c.valid and not c:FindBuff(s.spell) then
			c:ApplySpellBuff(s.spell)
			n = n + 1
		end
	end
	return n
end

--------------------------------------------------------------------- arming
-- Returns the spell name on success, or nil if the id is not a real spell.
function M.arm(spell_id, minutes)
	spell_id = tonumber(spell_id)
	if not spell_id or spell_id <= 0 then return nil end

	local name = eq.get_spell_name(spell_id)
	-- get_spell_name answers for ids that do not exist; an unnamed row is the tell.
	if not name or name == "" or name == "unknown_spell" then return nil end

	minutes = tonumber(minutes) or M.WINDOW_MINS
	if minutes < 1 then minutes = 1 end

	eq.set_data(M.BUCKET, string.format("%d|%d", spell_id, now() + minutes * 60))
	return name, minutes
end

--------------------------------------------------------------------- hooks
-- From global_player.event_connect and event_enter_zone.
function M.on_player(e)
	if e and e.self then M.apply_to(e.self) end
end

-- From global_player.event_timer, timer name "worldbuff".
--
-- ⚠️ THE TIMER IS PER CLIENT, not per zone, so this deliberately does PER-CLIENT work. Calling
-- apply_zone() here would walk the whole client list once for every player in the zone every
-- SWEEP_SECS -- quadratic in occupancy for no benefit. apply_to costs one bucket read and one
-- FindBuff, and when nothing is armed it is just the bucket read.
function M.on_sweep(client)
	M.apply_to(client)
end

return M
