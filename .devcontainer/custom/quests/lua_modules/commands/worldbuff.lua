-- #worldbuff -- put a spell on everybody, everywhere (GM).
--
--   #worldbuff <spellid>            arm for the default window
--   #worldbuff <spellid> <minutes>  arm for a specific window
--   #worldbuff status               what is armed right now
--   #worldbuff clear                cancel it
--
-- Arming buffs everyone standing in YOUR zone immediately. Everyone else picks it up when they log
-- in, when they zone, or on the next periodic sweep of the zone they are sitting in -- there is no
-- single call that reaches the whole server, so it is assembled out of those four.
-- See lua_modules/aotv4_worldbuff.lua for why.
local wb = require("aotv4_worldbuff")

local function worldbuff(e)
	local arg = (e.args and e.args[1] or ""):lower()

	if arg == "status" then
		local s = wb.pending()
		if not s then
			e.self:Message(15, "No world buff is armed.")
		else
			e.self:Message(15, string.format(
				"World buff: %s (%d), %d minutes left.",
				eq.get_spell_name(s.spell), s.spell,
				math.max(0, math.floor((s.expiry - os.time()) / 60))))
		end
		return
	end

	if arg == "clear" then
		wb.clear()
		e.self:Message(15, "World buff cleared. Buffs already given are left alone; they run out on their own.")
		return
	end

	if arg == "" then
		e.self:Message(15, "Usage: #worldbuff <spellid> [minutes] | status | clear")
		return
	end

	local name, minutes = wb.arm(arg, e.args and e.args[2])
	if not name then
		e.self:Message(15, string.format("'%s' is not a valid spell id.", arg))
		return
	end

	-- Everyone here gets it now; everyone else on arrival or on the next sweep.
	local n = wb.apply_zone()

	eq.world_emote(15, string.format("A blessing settles over Norrath: %s.", name))
	e.self:Message(15, string.format(
		"World buff armed: %s (%s) for %d minutes. %d player(s) in this zone buffed immediately; " ..
		"everyone else on login, on zoning, or within %d seconds.",
		name, arg, minutes, n, wb.SWEEP_SECS))
end

return worldbuff;
