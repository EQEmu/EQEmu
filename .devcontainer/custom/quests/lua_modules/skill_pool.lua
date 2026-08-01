-- skill_pool.lua
-- The ONLY skills that rotate through the level-up reward picker: class-specific ACTIVATED
-- combat abilities (signature moves inherited from other classes). Everything passive --
-- weapon proficiencies, Offense/Defense, Dodge/Parry/Riposte/Block, Double/Triple Attack,
-- Dual Wield, Intimidation -- is auto-learned instead (see global_player.lua free_skills) and
-- never appears here.
--
--   key  = EQ skill id (see the skill table in global_player.lua)
--   name = display name shown in the choice window
--   icon = spellbook icon index (203 = combat/fist, 139 = bash, 202 = disarm, 49 = frenzy)
--
-- These are reward-GATED on the client (areSkillsUnlocked): a skill here stays hidden until
-- the player picks it. MUST stay in sync with SKILLUNLOCK_COMBAT_IDS in
-- eq-core-dll/src/core_spellwindow.cpp.
local M = {}

M.SKILLS = {
	[8]  = { name = "Backstab",     icon = 203 },
	[10] = { name = "Bash",         icon = 139 },
	[30] = { name = "Kick",         icon = 203 },
	[16] = { name = "Disarm",       icon = 202 },
	[73] = { name = "Taunt",        icon = 203 },
	[72] = { name = "Berserking",   icon = 203 },
	[74] = { name = "Frenzy",       icon = 49 },
	[21] = { name = "Dragon Punch", icon = 203 },
	[23] = { name = "Eagle Strike", icon = 203 },
	[26] = { name = "Flying Kick",  icon = 203 },
	[38] = { name = "Round Kick",   icon = 203 },
	[52] = { name = "Tiger Claw",   icon = 203 },
}

-- ⚠️⚠️ THE ONLY SURVIVING RECORD OF WHICH CLASSES HAD THESE NATIVELY.
--
-- custom/sql/aotv4_open_spells_and_skills.sql gives EVERY class a skill_caps entry for all twelve,
-- because without a cap Client::CanHaveSkill refuses and a picked reward silently does nothing. The
-- side effect is that the database can no longer answer "which classes had this natively" -- the
-- answer there is now "all of them". This table was read out of skill_caps on 2026-07-27, BEFORE
-- that SQL ran, and it cannot be recovered from the DB again.
--
-- Class ids: 1 War 2 Cle 3 Pal 4 Ran 5 SK 6 Dru 7 Mnk 8 Brd 9 Rog 10 Sha 11 Nec 12 Wiz 13 Mag
--            14 Enc 15 Bst 16 Ber
--
-- Bard (8) is deliberately absent from every row: the pre-existing Bard-only server had already
-- been given every combat skill by hand (CLAUDE.md section 7), so its presence in skill_caps was
-- OUR edit and not what the class natively has.
M.NATIVE = {
	[8]  = { 9 },                        -- Backstab     Rogue
	[10] = { 1, 2, 3, 5 },               -- Bash         War, Cleric, Paladin, SK
	[16] = { 1, 3, 4, 5, 7, 9, 16 },     -- Disarm
	[21] = { 7 },                        -- Dragon Punch Monk
	[23] = { 7 },                        -- Eagle Strike Monk
	[26] = { 7 },                        -- Flying Kick  Monk
	[30] = { 1, 4, 7, 15, 16 },          -- Kick
	[38] = { 7 },                        -- Round Kick   Monk
	[52] = { 7 },                        -- Tiger Claw   Monk
	[72] = { },                          -- Berserking   no class had a cap at all
	[73] = { 1, 3, 4, 5 },               -- Taunt
	[74] = { 16 },                       -- Frenzy       Berserker
}

-- Does this class get the skill as part of being that class?
function M.is_native(skill_id, class_id)
	local list = M.NATIVE[skill_id]
	if not list then return false end
	for _, c in ipairs(list) do
		if c == class_id then return true end
	end
	return false
end

-- The ones this class does NOT get natively -- i.e. what is worth offering as a reward. Everything
-- else it already has, so offering it would be a wasted pick.
function M.rotatable_for(class_id)
	local out = {}
	for id, info in pairs(M.SKILLS) do
		if not M.is_native(id, class_id) then
			out[id] = info
		end
	end
	return out
end

-- Backwards compatibility: this module used to BE the plain { [id] = {name,icon} } table, and
-- spell_choice/other callers still index it that way.
setmetatable(M, { __index = M.SKILLS })

return M
