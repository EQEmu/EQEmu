-- aotv4_sinewtap.lua -- shared logic for the Sinew tap line (43318-43323).
--
-- A damage spell weighted 75 percent damage / 25 percent ENDURANCE return: a tier that deals 120
-- hands the caster 40 endurance. Structurally the mirror of lua_modules/aotv4_moonfire.lua, which
-- splits the same way but returns HEALTH.
--
-- The line pairs with the endurance cost on combat specials (section 22): specials now cost
-- endurance in proportion to the damage they deal, and this is what refills the bar.
--
-- ⚠️⚠️ THE SPELL ROWS ARE PLAIN NUKES (targettype 5) AND MUST STAY THAT WAY. targettype 13 is what
-- IsLifetapSpell keys on (common/spdat.cpp:108), and it makes Mob::Damage heal the caster 1x the
-- damage in HEALTH -- `int64 healed = damage;` (zone/attack.cpp:4287). That is free health nobody
-- asked for, on top of the endurance. Moonfire WANTS that engine-paid 1x and is a genuine tap;
-- this line does not, so every point of its return is paid right here.
--
-- ⚠️ Why any Lua is needed: there is no endurance-tap SPA. SPA 189 (SE_CurrentEndurance) applies to
-- the SPELL'S TARGET, and the target of a nuke is the thing you are hitting -- it would hand the
-- monster the endurance. A recourse spell could aim it at the caster, but only for a FLAT amount
-- unrelated to the tier's damage.
--
-- ⚠️ CLIENTS ONLY, and that is the shape of the mechanic rather than a guard against a rare case.
-- Mob::GetEndurance is a virtual returning 0 and Mob::SetEndurance a no-op (zone/mob.h:674, :677);
-- only Client overrides them (zone/client.h:751, :756). A pet or a charmed NPC casting this gets
-- nothing, which is correct -- endurance is a player resource.
--
-- ⚠️ The return is a FLAT amount while the damage actually dealt follows the resist roll. Same
-- accepted approximation as aotv4_moonfire.lua: a partially resisted cast returns full endurance
-- for reduced damage. The event does not hand us post-resist damage, so it cannot be re-derived
-- here without reimplementing the resist maths.

local M = {}

-- Called from each tier's spell script. `endurance` is that tier's return, which is the tier's
-- damage / 3 (the 75/25 split). Keep it in step with effect_base_value1 in
-- custom/sql/aotv4_sinew_line.sql -- the two numbers are only related by hand.
function M.tap_bonus(e, endurance)
	if not e or not endurance or endurance <= 0 then
		return
	end
	if not e.caster_id or e.caster_id == 0 then
		return
	end

	-- GetClientByID, not GetMobID: endurance exists only on Client, and this resolves to an invalid
	-- handle for a pet or NPC caster, which is exactly the case to skip.
	local caster = eq.get_entity_list():GetClientByID(e.caster_id)
	if not caster or not caster.valid then
		return          -- caster zoned, died mid-cast, or is not a player
	end

	-- ⚠️ There is no AddEndurance binding -- only Get/Set (zone/lua_client.cpp:1304-1321) -- so the
	-- cap is applied by hand. Without the clamp a cast at a full bar would set endurance ABOVE max.
	local cur = caster:GetEndurance()
	local max = caster:GetMaxEndurance()
	if cur >= max then
		return
	end

	local restored = cur + endurance
	if restored > max then
		restored = max
	end
	caster:SetEndurance(restored)

	-- ⚠️ The message is not decoration. Endurance is a thin bar most players never watch, so without
	-- it the whole mechanic is invisible -- which is exactly why "it did not return any endurance"
	-- was indistinguishable from the event never firing at all. Report what was ACTUALLY gained
	-- (restored - cur), not the tier amount, or a cast against a nearly-full bar overstates itself.
	caster:Message(15, string.format("You draw %d endurance from your target.", restored - cur))
end

return M
