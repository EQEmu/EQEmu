-- #worldboss -- arm the roaming world boss encounter (GM).
--
--   #worldboss              arm in a random classic dungeon
--   #worldboss <zone>       arm in a named zone (must be one of aotv4_worldboss.ZONES)
--   #worldboss status       what is armed right now
--   #worldboss clear        cancel the armed encounter
--
-- Arming only ANNOUNCES and records it -- the boss itself spawns when the first player walks into
-- that zone. See lua_modules/aotv4_worldboss.lua for why it cannot spawn immediately.
local wb = require("aotv4_worldboss")

local function worldboss(e)
	local arg = (e.args and e.args[1] or ""):lower()

	if arg == "status" then
		local s = wb.pending()
		if not s then
			e.self:Message(15, "No world boss is armed.")
		else
			e.self:Message(15, string.format(
				"World boss armed in %s, npc %d, %d minutes left.",
				s.zone, s.npc, math.max(0, math.floor((s.expiry - os.time()) / 60))))
		end
		return
	end

	if arg == "clear" then
		wb.clear()
		e.self:Message(15, "World boss cleared.")
		return
	end

	local pick = wb.arm(arg ~= "" and arg or nil)
	if not pick then
		e.self:Message(15, string.format("'%s' is not a world boss zone. Valid zones:", arg))
		local names = {}
		for _, z in ipairs(wb.ZONES) do names[#names + 1] = z.z end
		e.self:Message(15, table.concat(names, ", "))
		return
	end

	e.self:Message(15, string.format(
		"World boss armed in %s (%s). It spawns when the first player enters, and lapses in %d minutes.",
		pick.n, pick.z, wb.WINDOW_MINS))
end

return worldboss;
