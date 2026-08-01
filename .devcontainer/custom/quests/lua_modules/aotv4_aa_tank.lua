-- aotv4_aa_tank.lua -- behaviour for the MARKER AAs in the Tank tree.
--
-- Most of the tank tree is native: its ranks carry SPA rows in aa_rank_effects and the engine
-- applies them with no code at all (Shield Oath 192/185, Stonestride 162, Unyielding 172). Only the
-- abilities with no usable SPA live here.
--
-- The pattern: the AA carries NO effect rows and exists purely as something to read. Mob::GetAA
-- resolves a rank id to the ability and returns the rank YOU own, so passing the FIRST rank id
-- always answers "what rank of this do I have", 0 if none.
--
-- ⚠️ Keep the rank ids in step with custom/sql/aotv4_aa_tank_hosted.sql. They are the join between the two
-- halves, and nothing checks them -- a wrong id silently reads rank 0 forever, i.e. the AA does
-- nothing and looks bought.

local M = {}

-- Bloodied Bash (host AA 4) -- Bash/Slam heals a share of the damage it deals.
-- Not native: SPA 178 MeleeLifetap is a percentage of ALL melee with no way to gate it to one
-- skill, so it is done here off the skill id instead.
local BLOODIED_BASH_RANK1 = 17      -- host AA 4, ranks 17-21
local BASH_SKILL          = 10        -- SkillBash; SLAM (bash with no shield) reports the same skill
local BASH_HEAL_PCT       = { 25, 40, 55, 70, 85 }

-- Called from global_player event_damage_given. Runs on EVERY damage event for every player, so the
-- cheap rejections come first and GetAA is only touched on an actual Bash landing real damage.
function M.on_damage_given(e, client)
	if not e or not e.damage or e.damage <= 0 then return end
	if e.skill_id ~= BASH_SKILL then return end
	if e.is_damage_shield or e.is_buff_tic then return end
	if not client or not client.valid then return end

	local rank = client:GetAA(BLOODIED_BASH_RANK1)
	if rank < 1 then return end
	if rank > #BASH_HEAL_PCT then rank = #BASH_HEAL_PCT end

	local heal = math.floor(e.damage * BASH_HEAL_PCT[rank] / 100)
	if heal > 0 then
		client:HealDamage(heal)
	end
end

return M
