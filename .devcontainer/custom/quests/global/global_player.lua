-- items: 67704, 72091, 62621, 62622, 62844, 62827, 62828, 62836, 62883, 62876, 47100, 62878, 62879

local don = require("dragons_of_norrath")
local spell_choice = require("spell_choice")
local aa_choice = require("aa_choice")   -- random AA picker, driven by the points a death banks
local pok_travel = require("pok_travel")
local death_loss = require("death_loss")
local era_system = require("era_system")   -- server-wide expansion unlock progression
local hotzones = require("hotzones")       -- daily Hot Zones: one per region, 1.5x EXP
local bazaar_broker = require("bazaar_broker")  -- player-shop vendor window (vpset/vshop/vclose)
local aotv4_reactions = require("aotv4_reactions")  -- custom "when you are hit" abilities (Divine Aura, Blade Turn, Counterattack, Vengeful Aura, Duel)
local aotv4_dungeon = require("aotv4_dungeon")  -- scaling dungeon ("Delve"): instanced DoN zones, layers unlock in order
local aotv4_thirst = require("aotv4_thirst")         -- Thirst line: flat heal per melee hit
local aotv4_worldboss = require("aotv4_worldboss") -- roaming world boss encounter
local aotv4_worldbuff = require("aotv4_worldbuff")  -- server-wide GM buffs (#worldbuff)
local questjournal = require("aotv4_questjournal")  -- quest catalogue + tracking, in the Allaclone window
local spell_ranks_sys = require("aotv4_spell_ranks_sys")  -- keep 2 spells through death + rank them up
local aotv4_regions = require("aotv4_regions")   -- region unlocks earned by dying at the level cap
local aotv4_reforge = require("aotv4_reforge")   -- race/class change at level 1 (Reforger Vael)
local skill_pool      = require("skill_pool")       -- combat specials: which are native, which rotate
local aotv4_aa_tank = require("aotv4_aa_tank")       -- marker AAs in the Tank tree

-- AA-on-death tuning lives at the point of use, in event_death -- see the "RANDOM AA ON DEATH" block.
-- ⚠️ A long comment here used to describe a two-regime, era-anchored scheme (DEATH_AA_AT_CAP 0.40 /
-- DEATH_AA_SUB_CAP 0.38, with a break at level 50) together with the two constants it named. None of
-- it ran: the constants were unreferenced and the formula described a 50-to-70 world that no longer
-- exists now the cap is 30. Both are deleted rather than left as "documentation", because a comment
-- confidently describing a formula the code does not use is worse than no comment at all -- it is
-- the thing a later reader trusts instead of reading the code.

-- Every starting-path task wiped on death so the tutorial restarts as if entered for the FIRST time
-- (see event_death). Mostly the Gloomingdeep tutorial set; 5745 ("New Beginnings") is the Plane of
-- Knowledge new-player task, which otherwise lingers forever (it's a Quest-type task the client keeps
-- in the DB). These get RemoveTaskByTaskID'd (active, memory+DB) AND UncompleteTask'd (completed record).
-- The list MUST cover every task the tutorial zones (tutoriala/tutorialb) hand out -- any left off
-- lingers (active or completed) and blocks a clean re-offer when the player re-runs the tutorial after
-- death. 208/505744 = Jail Break (tutoriala); 8799/8804 (Hotbars) + 15035 = tutorialb sub-tasks.
local TUTORIAL_TASKS = {
	208, 1394, 1395, 1396, 1448, 3785, 5032, 5091, 5092, 5094, 5095,
	5096, 5097, 5098, 5102, 5106, 5166, 5702, 5703, 5745, 8505,
	8799, 8804, 15035, 505744,
}

function event_enter_zone(e)
	mysterious_voice(e)
	era_system.sync_zone()                          -- point this zone's expansion rule at the unlocked era
	aotv4_worldboss.on_enter_zone(e)                -- spawn the armed world boss if it is waiting for this zone
	aotv4_worldbuff.on_player(e)                    -- pick up an armed world buff on arrival
	aotv4_dungeon.on_enter_zone(e)                  -- crossing a zone line out of a delve fails the run
	eq.set_hotzone(hotzones.is_hot(eq.get_zone_short_name()))  -- 1.5x EXP if this is one of today's hot zones
	e.self:Message(MT.NPCQuestSay, "PORTALCLOSE")   -- dismiss the Portal window on any zone change
	e.self:Message(MT.NPCQuestSay, "LOOTCLOSE")     -- and the Advanced Loot window (its corpse is gone)
	-- ...but RE-OPEN the AoT menu: it is meant to be a persistent bar, and zoning tears the UI down.
	-- Showing an already-open window is harmless, so this is safe to send on every zone.
	e.self:Message(MT.NPCQuestSay, "AOTMENUSHOW")
	e.self:SetTimer("skillsync", 2)                 -- one-shot: re-reveal earned combat abilities after the UI builds (no jump)
	e.self:SetTimer("worldbuff", aotv4_worldbuff.SWEEP_SECS)  -- repeating: catch players who never zone or relog
	e.self:SetTimer("delvescale", aotv4_dungeon.RESCALE_SECS)  -- repeating: re-measure a delve runner and rescale un-pulled mobs

	-- Gloomingdeep Guard (5150) is a Tutorial-only protective buff; strip it the moment you leave the
	-- Tutorial so our permanent-buff rule doesn't carry it out into the world. (It stays while in tutorialb.)
	if eq.get_zone_id() ~= 189 then
		e.self:BuffFadeBySpellID(5150)
	end

	-- Restore the real bind once the player LEAVES the Tutorial. Death temporarily binds them to the
	-- Tutorial (event_death) so respawn lands there; here we put their own bind back and clear the save.
	local bkey = "deathbind_" .. e.self:CharacterID()
	local saved = eq.get_data(bkey)
	if saved and saved ~= "" and eq.get_zone_id() ~= 189 then
		local z, x, y, zz, h = saved:match("^(%-?%d+),(%-?[%d.]+),(%-?[%d.]+),(%-?[%d.]+),(%-?[%d.]+)$")
		if z then
			e.self:SetBindPoint(tonumber(z), 0, tonumber(x), tonumber(y), tonumber(zz), tonumber(h))
		end
		eq.delete_data(bkey)
	end

	if eq.is_lost_dungeons_of_norrath_enabled() and eq.get_zone_short_name() == "lavastorm" and e.self:GetGMStatus() >= 80 then 
		e.self:Message(MT.DimGray, "There are GM commands available for Dragons of Norrath, use " .. eq.say_link("#don") .. " to get started")
	end
end

function mysterious_voice(e)
	if not eq.is_lost_dungeons_of_norrath_enabled() then
		return
	end
	local qglobals = eq.get_qglobals(e.self);
	if e.self:GetLevel() < 15 then
		return
	end
	if qglobals.Wayfarer ~= nil then
		return
	end
	local zone_id = eq.get_zone_id();

	local voice_zones = {
		Zone.qeynos,
		Zone.qeynos2,
		Zone.qrg,
		Zone.freportn,
		Zone.freportw,
		Zone.freporte,
		Zone.rivervale,
		Zone.ecommons,
		Zone.erudnint,
		Zone.erudnext,
		Zone.halas,
		Zone.everfrost,
		Zone.nro,
		Zone.sro,
		Zone.neriaka,
		Zone.neriakb,
		Zone.neriakc,
		Zone.qcat,
		Zone.oggok,
		Zone.grobb,
		Zone.gfaydark,
		Zone.akanon,
		Zone.kaladima,
		Zone.felwithea,
		Zone.felwitheb,
		Zone.kaladimb,
		Zone.butcher,
		Zone.paineel,
		Zone.cabwest,
		Zone.cabeast,
		Zone.sharvahl,
		Zone.poknowledge,
		Zone.freeporteast,
		Zone.freeportwest,
		Zone.northro,
		Zone.southro,
		Zone.commonlands
	};

	for _, zone in pairs(voice_zones) do
		if zone == zone_id then
			e.self:Message(MT.Yellow,
			"A mysterious voice whispers to you, \'If you can feel me in your thoughts, know this -- "
			.. "something is changing in the world and I reckon you should be a part of it. I do not know much, but I do know "
			.. "that in every home city and the wilds there are agents of an organization called the Wayfarers Brotherhood. They "
			.. "are looking for recruits . . . If you can hear this message, you are one of the chosen. Rush to your home city, or "
			.. "search the West Karanas and Rathe Mountains for a contact if you have been exiled from your home for your deeds, "
			.. "and find out more. Adventure awaits you, my friend.\'");
			return
		end
	end
end

function event_combine_validate(e)
	-- e.validate_type values = { "check_zone", "check_tradeskill" }
	-- criteria exports:
	--	["check_zone"].         = e.zone_id
	--	["check_tradeskill"]    = e.tradeskill_id (not active)
	if (e.recipe_id == 10344) then
		if (e.validate_type:find("check_zone")) then
			if (e.zone_id ~= Zone.tipt and e.zone_id ~= Zone.vxed) then
				return 1;
			end
		end
	end

	return 0;
end

function event_combine_success(e)
	if (e.recipe_id == 10904 or e.recipe_id == 10905 or e.recipe_id == 10906 or e.recipe_id == 10907) then
		e.self:Message(MT.Default,
		"The gem resonates with power as the shards placed within glow unlocking some of the stone's power. "
		.. "You were successful in assembling most of the stone but there are four slots left to fill, "
		.. "where could those four pieces be?"
		);
	elseif(e.recipe_id == 10903 or e.recipe_id == 10346 or e.recipe_id == 10334) then
		local reward = { };
		reward["melee"] =  { ["10903"] = 67665, ["10346"] = 67660, ["10334"] = 67653 };
		reward["hybrid"] = { ["10903"] = 67666, ["10346"] = 67661, ["10334"] = 67654 };
		reward["priest"] = { ["10903"] = 67667, ["10346"] = 67662, ["10334"] = 67655 };
		reward["caster"] = { ["10903"] = 67668, ["10346"] = 67663, ["10334"] = 67656 };

		local ctype = eq.ClassType(e.self:GetClass());
		e.self:SummonItem(reward[ctype][tostring(e.recipe_id)]);
		e.self:SummonItem(67704); -- Item: Vaifan's Clockwork Gemcutter Tools
		e.self:Message(MT.Default, "Success");
	--cleric 1.5
	elseif(e.recipe_id == 19460) then
		e.self:AddEXP(25000);
		e.self:AddAAPoints(5);
		e.self:Ding();
		e.self:Message(MT.Yellow,'You have gained 5 ability points!');
		eq.set_global("cleric_epic","7",5,"F");
	--rogue 1.5
	elseif(e.recipe_id == 13402 or e.recipe_id == 13403 or e.recipe_id == 13404 or e.recipe_id == 13405) then
		e.self:Message(MT.Yellow,"The piece of the metal orb fuses together with the blue diamonds under the intense heat of the forge. As it does, a flurry of images flash through your mind... A ranger and his bear side by side, stoic and unafraid, in a war-torn forest. A bitter tattooed woman with bluish skin wallowing in misery in a waterfront tavern. An endless barrage of crashing thunder and lightning illuminating a crimson brick ampitheater. Two halflings locked in a battle of wits using a checkered board. The images then fade from your mind");
	--ranger 1.5 tree
	elseif(e.recipe_id ==13412) then
		eq.set_global("ranger_epic","3",5,"F");
		if(eq.get_zone_short_name()=="jaggedpine") then
			e.self:Message(MT.Yellow,"The seed grows rapidly the moment you push it beneath the soil. It appears at first as a mere shoot, but within moments grows into a stout sapling and then into a gigantic tree. The tree is one you've never seen before. It is the coloration and thick bark of a redwood with the thick bole indicative of the species. The tree is, however, far too short and has spindly branches sprouting from it with beautiful flowers that you would expect on a dogwood. You take all of this in at a glance. It takes you a moment longer to realize that the tree is moving.");			
			eq.spawn2(181222, 0, 0, e.self:GetX()+3,e.self:GetY()+3,e.self:GetZ(),0); -- NPC: Red_Dogwood_Treant
		else
			e.self:Message(MT.Yellow,"The soil conditions prohibit the seed from taking hold");
			e.self:SummonItem(72091); -- Item: Fertile Earth
			e.self:SummonItem(62621); -- Item: Senvial's Blessing
			e.self:SummonItem(62622); -- Item: Grinbik's Blessing
			e.self:SummonItem(62844); -- Item: Red Dogwood Seed
		end
	--ranger 1.5 final
	elseif(e.recipe_id ==13413) then
		e.self:AddEXP(25000);
		e.self:AddAAPoints(5);
		e.self:Ding();
		e.self:Message(MT.Yellow,'You have gained 5 ability points!');
		eq.set_global("ranger_epic","5",5,"F");
	--ranger 2.0
	elseif(e.recipe_id ==19914 or e.recipe_id==19915) then
		e.self:Message(MT.Yellow,'Very Good. Now we must attune the cage to the specific element we wish to free. You will need two items, one must protect from the element and the other must be able to absorb an incredible amount of that element. This is not a simple task. You must first discover the nature of the spirit that you wish to free and then find such items that will allow you to redirect its power. You must know that each spirit represents a specific area within their element and that is what you must focus on, not their element specifically. For example, Grinbik was an earth spirit, but his area of power was fertility. Senvial was a spirit of Water, but his power was in mist and fog.');
		eq.set_global("ranger_epic","8",5,"F");
	elseif(e.recipe_id ==19916) then
		e.self:Message(MT.Yellow,"The Red Dogwood Treant speaks to you from within your sword. 'Well done. This should allow me to free a spirit with power over cold and ice. Now you need to find the power that binds the spirit and unleash it where that spirit is bound.'");	
	elseif(e.recipe_id ==19917) then
		if(eq.get_zone_short_name()=="anguish") then
			eq.spawn2(317113, 0, 0, e.self:GetX(),e.self:GetY(),e.self:GetZ(),0); -- NPC: #Oshimai_Spirit_of_the_High_Air
		end
	-- paladin 1.5 final
	elseif(e.recipe_id ==19880) then
		e.self:AddEXP(25000);
		e.self:AddAAPoints(5);
		e.self:Ding();
		e.self:Message(MT.Yellow,'You have gained 5 ability points!');	
		eq.set_global("paladin_epic","8",5,"F");
		e.self:Message(MT.Gray,"As the four soulstones come together, a soft blue light eminates around the dark sword. The soulstones find themselves at home within the sword. A flash occurs and four voices in unison speak in your mind, 'Thank you for saving us and giving us a purpose again. You are truly our savior and our redeemer, and we shall serve you from now on. Thank you, noble knight!")
	--bard 1.5 final	
	elseif(e.recipe_id == 19882) then
		e.self:AddEXP(25000);
		e.self:AddAAPoints(5);
		e.self:Ding();
		e.self:Message(MT.Yellow,'You have gained 5 ability points!');	
		eq.set_global("bard15","6",5,"F");
	--druid 1.5 feerrott
	elseif(e.recipe_id == 19888) then
		if(eq.get_zone_short_name()=="feerrott") then
			eq.spawn2(47209, 0, 0, e.self:GetX()+10,e.self:GetY()+10,e.self:GetZ(),0); -- NPC: corrupted_spirit
			e.self:Message(MT.White,"compelled spirit screams as his essences is forced back into the world of the living. 'What is this? Where am I? Who are you? What do you want from me?");
		else
			e.self:SummonItem(62827); -- Item: Mangled Head
			e.self:SummonItem(62828); -- Item: Animating Heads
			e.self:SummonItem(62836); -- Item: Soul Stone
		end
	-- druid 1.5 final
	elseif(e.recipe_id ==19892) then
		e.self:AddAAPoints(5);
		e.self:Ding();
		e.self:Message(MT.Yellow,'You have gained 5 ability points!');	
		eq.set_global("druid_epic","8",5,"F");	
		e.self:SendMarqueeMessage(MT.Yellow, 510, 1, 100, 10000, "You plant the Mind Crystal and the Seed of Living Brambles in the pot. The pot grows warm and immediately you see a vine sprouting from the soil. The vine continues to grow at a tremendous rate. Brambles grow into the heart of the crystal where the core impurity is and split it. They continue to grow at an astounding speed and soon burst the pot and form the Staff of Living Brambles");
	--druid 2.0 sub final
	elseif(e.recipe_id ==19908) then
		if(eq.get_zone_short_name()=="anguish") then
			eq.spawn2(317115, 0, 0, e.self:GetX()+3,e.self:GetY()+3,e.self:GetZ(),0); -- NPC: #Yuisaha
			e.self:SummonItem(62883); -- Item: Essence of Rainfall
			e.self:SummonItem(62876); -- Item: Insulated Container
		else
			e.self:Message(MT.Yellow,"The rain spirit cannot be reached here");
			e.self:SummonItem(47100); -- Item: Globe of Discordant Energy
			e.self:SummonItem(62876); -- Item: Insulated Container
			e.self:SummonItem(62878); -- Item: Frozen Rain Spirit
			e.self:SummonItem(62879); -- Item: Everburning Jagged Tree Limb
		end
	--druid 2.0 final
	elseif(e.recipe_id ==19909) then	
		e.self:AddEXP(50000);
		e.self:AddAAPoints(10);
		e.self:Ding();
		e.self:Message(MT.Yellow,'You have gained 10 ability points!');	
		eq.set_global("druid_epic","13",5,"F");	
		--e.self:SendMarqueeMessage(MT.Yellow, 510, 1, 100, 10000, "You plant the Mind Crystal and the Seed of Living Brambles in the pot. The pot grows warm and immediately you see a vine sprouting from the soil. The vine continues to grow at a tremendous rate. Brambles grow into the heart of the crystal where the core impurity is and split it. They continue to grow at an astounding speed and soon burst the pot and form the Staff of Living Brambles");
	--warrior 2.0
	elseif(e.recipe_id ==19902) then	
		e.self:AddEXP(50000);
		e.self:AddAAPoints(10);
		e.self:Ding();
		e.self:Message(MT.Yellow,'You have gained 10 ability points!');	
		eq.set_global("warrior_epic","21",5,"F");		
	-- CLR 2.0
	elseif (e.recipe_id == 19893) then
		e.self:Message(MT.Red, "Omat should probably see this.");
	--ench 2.0
	elseif (e.recipe_id == 19919) then
		eq.set_global("ench_epic","9",5,"F");
		e.self:Message(MT.Yellow,"Your Oculus of Persuasion gleams with a blinding light for a moment, dimming quickly to its previous understated beauty. The light has left an image burned into your mind, a strangely tattooed woman chanting by a waterfall.");
	--ench 2.0 final
	elseif (e.recipe_id == 19920) then
		e.self:Message(MT.Yellow,"The discordant energy shoots through the staff, sending a shower of sparks through the air. The crystal shatters before you, and as the sparks fade away you notice the changes in your staff.");
		e.self:AddEXP(50000);
		e.self:AddAAPoints(10);
		e.self:Ding();
		e.self:Message(MT.Yellow,'You have gained 10 ability points!');
		eq.set_global("ench_epic","10",5,"F");
	--pal 2.0 final
	elseif (e.recipe_id == 19925) then
		e.self:Message(MT.Yellow,"As you combine all six tokens in the scabbard with Redemption, you feel a tugging at your soul. An energy flows through you as you feel the virtues of your inner self being tugged and tempered into the weapon. For a second you feel drained, but now that feeling has subsided. A final flash of light occurs and a new sword is tempered; Nightbane, Sword of the Valiant");
		e.self:AddEXP(50000);
		e.self:AddAAPoints(10);
		e.self:Ding();
		e.self:Message(MT.Yellow,'You have gained 10 ability points!');
		eq.set_global("paladin_epic","11",5,"F");
		eq.delete_global("paladin_epic_mmcc");
		eq.delete_global("paladin_epic_hollowc");
	elseif (e.recipe_id == 2182) then -- Pumpkin Pie
		if (eq.is_task_activity_active(8013, 0)) then -- The Hungry Halfling
			eq.update_task_activity(8013, 0, 1);
		end
	elseif (e.recipe_id == 2181) then -- Pumpkin Bread
		if (eq.is_task_activity_active(8013, 1)) then -- The Hungry Halfling
			eq.update_task_activity(8013, 1, 1);
		end
	elseif (e.recipe_id == 7811) then -- Spiced Pumpkin Cider
		if (eq.is_task_activity_active(8013, 2)) then -- The Hungry Halfling
			eq.update_task_activity(8013, 2, 1);
		end
	elseif (e.recipe_id == 2183) then -- Pumpkin Shake
		if (eq.is_task_activity_active(8013, 3)) then -- The Hungry Halfling
			eq.update_task_activity(8013, 3, 1);
		end
	end
end

function event_command(e)
	return eq.DispatchCommands(e);
end

--[[ the main key is the ID of the AA
--   the first set is the age required in seconds
--   the second is if to ignore the age and grant anyways live test server style
--   the third is enabled
--]]
vet_aa = {
    [481]  = { 31536000, true, true}, -- Lesson of the Devote 1 yr
    [482]  = { 63072000, true, true}, -- Infusion of the Faithful 2 yr
    [483]  = { 94608000, true, true}, -- Chaotic Jester 3 yr
    [484]  = {126144000, true, true}, -- Expedient Recovery 4 yr
    [485]  = {157680000, true, true}, -- Steadfast Servant 5 yr
    [486]  = {189216000, true, true}, -- Staunch Recovery 6 yr
    [487]  = {220752000, true, true}, -- Intensity of the Resolute 7 yr
    [511]  = {252288000, true, true}, -- Throne of Heroes 8 yr
    [2000] = {283824000, true, true}, -- Armor of Experience 9 yr
    [8081] = {315360000, true, true}, -- Summon Resupply Agent 10 yr
    [8130] = {346896000, true, true}, -- Summon Clockwork Banker 11 yr
    [453]  = {378432000, true, true}, -- Summon Permutation Peddler 12 yr
    [182]  = {409968000, true, true}, -- Summon Personal Tribute Master 13 yr
    [600]  = {441504000, true, true}, -- Blessing of the Devoted 14 yr
}


-- Auto-grant casting/singing/utility + the PASSIVE combat skills whose skill_caps cap is already > 0 at
-- the player's current level. Combat ACTIVATED skills stay reward-gated (skill_pool.lua). The cap curve
-- is the level gate: Dual Wield (22) caps from level 1, Double Attack (20) from 32, Triple Attack (76)
-- from 58 -- so a skill is only granted once the player is high enough (MaxSkill > 0). Run on BOTH connect
-- (so a level-1 char has Dual Wield immediately, not only after their first ding) and level up.
local FREE_SKILLS = {
	4,5,13,14,18,24,          -- casting (Abjuration/Alteration/Channeling/Conjuration/Divination/Evocation)
	43,44,45,46,47,           -- casting specializations
	12,41,49,54,70,           -- singing + instruments (Bard)
	9,17,25,27,29,31,32,39,42,67,-- utility (Bind Wound/Disarm Traps/Feign Death/Forage/Hide/Meditate/Mend/Safe Fall/Sneak/Begging)
	0,1,2,3,36,28,7,51,       -- weapon proficiencies (1H/2H Blunt+Slash, 1H Pierce, H2H, Archery, Throwing)
	56,57,59,                 -- race/class-locked tradeskills (Make Poison/Tinkering/Alchemy) -- granted so they SHOW in the skill window + are usable (server gates are patched; everyone is a Bard)
	33,15,19,34,37,11,20,76,22,71 -- passive combat: Offense/Defense/Dodge/Parry/Riposte/Block/Double(20)+Triple(76) Attack/Dual Wield(22)/Intimidation
}
local function grant_free_skills(c)
	for _, v in ipairs(FREE_SKILLS) do
		if c:MaxSkill(v) > 0 and c:GetRawSkill(v) < 1 and c:CanHaveSkill(v) then
			c:SetSkill(v, 1)
		end
	end
end

-- AoTv4: every class starts with the ACTIVATED combat specials it gets natively -- a Rogue has
-- Backstab, a Monk has its strikes, a Warrior has Kick and Taunt. The ones a class does NOT get
-- natively are what the level-up picker offers, so nothing is both granted and rotated.
--
-- ⚠️ The native map lives in skill_pool.NATIVE and NOT in skill_caps. Every class now has a cap for
-- all twelve (custom/sql/aotv4_open_spells_and_skills.sql) because a picked reward cannot otherwise
-- be granted, which means CanHaveSkill answers true for everything and the database can no longer
-- tell you what was native. That table is the only surviving copy.
local function grant_native_combat_skills(c)
	local class = c:GetClass()
	for id, _ in pairs(skill_pool.SKILLS) do
		if skill_pool.is_native(id, class) and c:MaxSkill(id) > 0 and c:GetRawSkill(id) < 1 then
			c:SetSkill(id, 1)
		end
	end
end

-- AoTv4: every character keeps each TRADESKILL at a floor of 20. New characters start there on their
-- first connect; existing characters are raised to 20 on login if lower. Only RAISES -- a tradeskill
-- already trained above 20 is left alone. (Caps are 300 from level 1 for all of these, so the floor
-- always sticks; the MaxSkill guard is just a safety net.)
local TRADESKILLS = {
	55, 56, 57, 58, 59, 60, 61, 63, 64, 65, 68, 69,   -- Fishing, Make Poison, Tinkering, Research, Alchemy,
	                                                  -- Baking, Tailoring, Blacksmithing, Fletching, Brewing,
	                                                  -- Jewelry Making, Pottery
}
local function floor_tradeskills(c)
	for _, v in ipairs(TRADESKILLS) do
		if c:CanHaveSkill(v) and c:MaxSkill(v) >= 20 and c:GetRawSkill(v) < 20 then
			c:SetSkill(v, 20)
		end
	end
end

function event_connect(e)
	-- Delve: drop a journal entry left over from a camp or a client crash. No-op if the run is still
	-- live (camped and came back while the instance is still up).
	aotv4_dungeon.on_connect(e.self)

	-- AoTv4: ALL CLASSES UNLOCKED. The Bard-force is permanently removed -- every character keeps its
	-- created class. The four pure-melee classes (Warrior/Monk/Rogue/Berserker) are made casters
	-- client-side by dinput8.dll (core_allcasters: IsSpellcaster + Max_Mana + mana-gauge patches) and
	-- server-side by CalcMaxMana (level*40 mana). The reward pool's custom spells are opened to all 16
	-- classes; they are real spells (skill 98, mana cost) -- IsBardSong is skill-gated so only genuine
	-- Singing/instrument songs get song behavior. (Existing pre-unlock characters remain Bards -- a valid
	-- class; no forced reclass.)

	grant_veteran_aa(e)
	don.fix_invalid_faction_state(e.self)
	pok_travel.send_list(e.self, true)   -- warm the dll Portal window with discovered zones

	-- tell the client which combat skills are already earned (so the unlock hook reveals them)
	spell_choice.send_unlocks(e.self)

	-- Open the AoT menu (core_aotmenu.cpp). The dll swallows this line.
	--
	-- ⚠️ THE SERVER TELLS THE CLIENT IT IS IN THE WORLD; the dll must NOT work that out for itself.
	-- Both client-side tests were tried and both are wrong there: `pinstLocalPlayer != null` is ALSO
	-- true at character select (it renders the selected character), and `gGameState` is only ever
	-- assigned by MQ2's detour machinery, which that dll runs with disabled -- so it never updates.
	-- A chat line cannot arrive before you are in the world, which makes this both simpler and
	-- strictly more reliable. Same pattern as every other window in the dll.
	e.self:Message(MT.NPCQuestSay, "AOTMENUSHOW")

	-- hand over coin earned while the player's (permanent) shop sold items offline
	bazaar_broker.pay_escrow(e.self)
	aotv4_worldbuff.on_player(e)                    -- and on login

	grant_free_skills(e.self)            -- level-1 chars get Dual Wield etc. now, not only after first ding
	grant_native_combat_skills(e.self)  -- a Rogue has Backstab, a Monk its strikes; the rest are picker rewards
	floor_tradeskills(e.self)           -- every tradeskill floored to 20 (new chars start there; existing raised on login)

	-- daily Hot Zone welcome. Computed live from the date (hotzones.lua) so it always matches the actual
	-- 1.5x apply (event_enter_zone -> eq.set_hotzone) and the #hotzone popup -- no bucket/cron to drift.
	-- Printed per-line because the RoF2 MOTD control won't render line breaks.
	local hz_today = hotzones.today()
	if #hz_today > 0 then
		e.self:Message(MT.LightBlue, "----- Welcome to AoTv4!  Today's Hot Zones (1.5x EXP) -----")
		for _, z in ipairs(hz_today) do
			-- ⚠️ Name the REGION too. There is exactly one hot zone per region now, so "which region is hot
			-- today" is meaningless -- what a player needs is which zone in the region they can reach.
			e.self:Message(MT.Lime, string.format("   %s  -  %s", z.l, z.region or "?"))
		end
	end
end

function event_timer(e)
	if e.timer == "skillsync" then
		-- one-shot: fire ~2s after zone-in, once the client has finished building its Combat Abilities
		-- list, then (re)send the earned-skill set + nudge a rebuild so abilities show without jumping.
		e.self:StopTimer("skillsync")
		spell_choice.send_unlocks(e.self)
	elseif e.timer == "delvescale" then
		-- Re-measure a player inside a delve and rescale anything not already pulled, so entering
		-- stripped and then gearing up does not leave the dungeon soft. No-op outside a delve.
		aotv4_dungeon.on_tick(e.self)
	elseif e.timer == "delveclose" then
		-- one-shot: the grace period after the reward chest is opened has run out, so shut the
		-- instance down. ⚠️ StopTimer FIRST -- a client timer REPEATS, and without this the delve
		-- would try to close every two minutes for the rest of the session.
		e.self:StopTimer("delveclose")
		aotv4_dungeon.on_close_timer(e.self)
	elseif e.timer == "worldbuff" then
		-- Catches the player who is parked somewhere and never zones or relogs -- the one hole the
		-- connect and enter-zone hooks leave. Per CLIENT, so it only ever touches this one player.
		aotv4_worldbuff.on_sweep(e.self)
	end
end

function grant_veteran_aa(e)
	if not eq.is_dragons_of_norrath_enabled() then
		return
	end

    local age = e.self:GetAccountAge();
    for aa, v in pairs(vet_aa) do
        if v[3] and (v[2] or age >= v[1]) then
            e.self:GrantAlternateAdvancementAbility(aa, 1)
        end
    end
end

--[[
0  /*13855*/ Skill1HBlunt = 0,
1  /*13856*/ Skill1HSlashing,
2  /*13857*/ Skill2HBlunt,
3  /*13858*/ Skill2HSlashing,
4  /*13859*/ SkillAbjuration,
5  /*13861*/ SkillAlteration,
6  /*13862*/ SkillApplyPoison, X
7  /*13863*/ SkillArchery, X
8  /*13864*/ SkillBackstab,
9  /*13866*/ SkillBindWound,
10 /*13867*/ SkillBash,
11 /*13871*/ SkillBlock,
12 /*13872*/ SkillBrassInstruments,
13 /*13874*/ SkillChanneling,
14 /*13875*/ SkillConjuration,
15 /*13876*/ SkillDefense,
16 /*13877*/ SkillDisarm,
17 /*13878*/ SkillDisarmTraps, 
18 /*13879*/ SkillDivination,
19 /*13880*/ SkillDodge,
20 /*13881*/ SkillDoubleAttack,
21 /*13882*/ SkillDragonPunch,
21 /*13924*/ SkillTailRake = SkillDragonPunch, // Iksar Monk equivilent
22 /*13883*/ SkillDualWield,
23 /*13884*/ SkillEagleStrike,
24 /*13885*/ SkillEvocation,
25 /*13886*/ SkillFeignDeath,
26 /*13888*/ SkillFlyingKick,
27 /*13889*/ SkillForage, X
28 /*13890*/ SkillHandtoHand,
29 /*13891*/ SkillHide,
30 /*13893*/ SkillKick,
31 /*13894*/ SkillMeditate,
32 /*13895*/ SkillMend,
33 /*13896*/ SkillOffense,
34 /*13897*/ SkillParry,
35 /*13899*/ SkillPickLock, X
36 /*13900*/ Skill1HPiercing,        // Changed in RoF2(05-10-2013)
37 /*13903*/ SkillRiposte,
38 /*13904*/ SkillRoundKick,
39 /*13905*/ SkillSafeFall, 
40 /*13906*/ SkillSenseHeading, X
41 /*13908*/ SkillSinging,
42 /*13909*/ SkillSneak,
43 /*13910*/ SkillSpecializeAbjure,      // No idea why they truncated this one..especially when there are longer ones...
44 /*13911*/ SkillSpecializeAlteration,
45 /*13912*/ SkillSpecializeConjuration,
46 /*13913*/ SkillSpecializeDivination,
47 /*13914*/ SkillSpecializeEvocation,
48 /*13915*/ SkillPickPockets, X
49 /*13916*/ SkillStringedInstruments,
50 /*13917*/ SkillSwimming, X
51 /*13919*/ SkillThrowing,
52 /*13920*/ SkillTigerClaw,
53 /*13921*/ SkillTracking, X
54 /*13923*/ SkillWindInstruments,
55 /*13854*/ SkillFishing, X
56 /*13853*/ SkillMakePoison, X
57 /*13852*/ SkillTinkering, X
58 /*13851*/ SkillResearch, X
59 /*13850*/ SkillAlchemy, X
60 /*13865*/ SkillBaking, X
61 /*13918*/ SkillTailoring, X
62 /*13907*/ SkillSenseTraps, X
63 /*13870*/ SkillBlacksmithing, X
64 /*13887*/ SkillFletching, X
65 /*13873*/ SkillBrewing, X
66 /*13860*/ SkillAlcoholTolerance, X
67 /*13868*/ SkillBegging, 
68 /*13892*/ SkillJewelryMaking, X
69 /*13901*/ SkillPottery, X
70 /*13898*/ SkillPercussionInstruments,
71 /*13922*/ SkillIntimidation,
72 /*13869*/ SkillBerserking,
73 /*13902*/ SkillTaunt,
74 /*05837*/ SkillFrenzy,
75 /*03670*/  SkillRemoveTraps,  X
76 /*13049*/  SkillTripleAttack,
]]--

function event_level_up(e)
  -- Era level cap: hold the character at the current expansion's cap. Clamping returns early so we
  -- don't offer spells above the cap (the spell picker is already level-bounded, so the cap also
  -- gates the spell pool). At the cap, XP keeps flowing into the death-bank (held at the threshold).
  if era_system.clamp_level(e.self) then
    return
  end

  -- Auto-grant casting/singing/utility + passive combat skills newly unlocked at this level (their
  -- skill_caps cap just crossed 0 -- e.g. Double Attack @32, Triple Attack @58). Combat ACTIVATED
  -- skills stay reward-gated (skill_pool.lua). Same grant used on connect. See grant_free_skills above.
  grant_free_skills(e.self)
  grant_native_combat_skills(e.self)   -- cap curves for some specials only open above level 1

  if e.self:GetLevel() == 5 then
    eq.popup("", "<c \"#F0F000\">Welcome to level 5.</c><br><br>You have just been granted a new ability called '<c \"#F0F000\">Origin</c>' which allows you to teleport back to your starting city.<br><br>Open the Alternate Advancement window by pressing the '<c \"#F0F000\">V</c>' key, look in the '<c \"#F0F000\">General' tab</c>, and find the '<c \"#F0F000\">Origin</c>' ability and select it.<br><br>Now press the '<c \"#F0F000\">Hotkey</c>' button to create a hotkey you can place on your hot bar.");
  end

  if e.self:GetLevel() == 10 and eq.is_dragons_of_norrath_enabled() then
    eq.popup("", "<c \"#F0F000\">Welcome to level 10.</c><br><br>You are now able to begin the new player armor and weapon quests.  Speak with Castlen and Barrenzin or V`Lynn Renloe in the <c \"#66CCFF\">Plane of Knowledge</c> to begin.  One additional quest will become available to you at each level past level 10, so be sure to check back with these NPCs as you continue to gain experience.");
  end

  -- offer a choice of 3 level-appropriate spells to learn
  spell_choice.offer(e)
end

-- On death, bank ALL experience as Alternate Advancement and restart at level 1 (roguelite).
-- The random AA window then pops; banked AA scales with how far the run got.
function event_death(e)
  local client = e.self
  local death_level = client:GetLevel()   -- capture BEFORE the roguelite reset to level 1 below
  local run_xp      = client:GetEXP()     -- XP earned THIS run (effort); also captured pre-reset

  -- server-wide death announcement: who died, what killed them, at what level, and lifetime deaths.
  -- Death count persists in the per-character bucket "deaths_<charid>".
  local cid    = client:CharacterID()
  local deaths = (tonumber(eq.get_data("deaths_" .. cid)) or 0) + 1
  eq.set_data("deaths_" .. cid, tostring(deaths))
  local killer = (e.other and e.other:GetCleanName()) or "the world"
  local times  = deaths == 1 and "time" or "times"
  -- server-wide broadcast; MT.Yellow reliably renders in the main chat window (MT.Red can get filtered)
  eq.world_emote(MT.Yellow, string.format("%s has been slain by %s at level %d!  They have now died %d %s.",
    client:GetCleanName(), killer, death_level, deaths, times))

  -- a duel ends the moment one of the participants dies
  aotv4_reactions.end_duel(client:GetID())

  -- Delve: dying ends the run. The roguelite reset below puts this character back to level 1, so the
  -- mission cannot meaningfully continue -- the task is FAILED and the instance is closed. Called
  -- BEFORE the level reset so the module still sees the run as it was.
  aotv4_dungeon.on_death(e)

  -- ⚠️ Credit a capped death toward the region-unlock chain BEFORE the reset to level 1 below --
  -- afterwards the character is level 1 and no death would ever count.
  aotv4_regions.on_death(e)

  -- ROGUELITE: every death banks AA (scaled by the level reached) and restarts the run at level 1.
  -- SetEXP takes (normal_exp, aa_exp) -- zero normal XP, keep AA exp.
  client:SetLevel(1)
  client:SetEXP(0, client:GetAAExp())
  -- turn off active tribute on death (a level-1 fresh run shouldn't keep raid-tier tribute buffs).
  -- pcall-guarded: ToggleTribute is a NEW Lua binding that only exists in the rebuilt zone binary.
  -- Until that binary is deployed, calling it raises a nil-method error that would ABORT the rest of
  -- event_death (including the AA banking below). The guard lets death processing continue now, and
  -- the tribute-off starts working automatically once the rebuilt zone is live.
  pcall(function() client:ToggleTribute(false) end)

  -- RANDOM AA ON DEATH (restored 2026-07-31). The run's experience is converted into AA points and
  -- the picker offers three RANDOM abilities the player can afford; picking one deducts its cost and
  -- re-offers from whatever the remainder can still buy, until nothing is affordable and it stops.
  --
  -- ⚠️⚠️ THE CONVERSION IS THE SHEET'S OWN RATE -- one AA point per AA_EXP_PER_POINT of run
  -- experience, the same 200,000 the `AA:ExpPerPoint` rule uses (exp.xlsx). It has to stay in step
  -- with that rule and with the level curve: a full climb to the level 35 cap is 665,000 experience,
  -- so a death at cap banks 3 points. That is deliberately small -- three points buys one 2-cost
  -- ability and then a 1-cost one, which is the intended shape of a single death's reward, not a
  -- shopping trip.
  -- ⚠️ Lua cannot read a rule directly (there is no get_rule binding), so the constant is duplicated
  -- here. If `AA:ExpPerPoint` is ever retuned, THIS MUST MOVE WITH IT or deaths will pay out at a
  -- rate that no longer matches what experience is worth anywhere else.
  -- ⚠️ run_xp was captured at the TOP of this function, before SetEXP(0, ...) above wiped it. Reading
  -- it here would always return 0.
  local AA_EXP_PER_POINT = 200000

  -- ⚠️⚠️ A LITERAL 1:1 CONVERSION -- ALL run experience becomes AA experience. This is the balance
  -- sheet's own model (exp.xlsx, the "AA %" column), which is simply
  --     AA percent = total experience / 2000       i.e. one point per 200,000
  -- against the curve zone/exp.cpp implements:
  --     xp to next level (L) = 1000 * (L + 1)
  --     total xp through  L  = 1000 * L * (L + 3) / 2      -> 665,000 at the level 35 cap
  -- so a full climb to cap banks 665,000 / 200,000 = 3.32 points. Sample rows, all matching the
  -- sheet exactly: L5 20,000 xp = 10 percent; L20 230,000 = 115 percent; L35 665,000 = 333 percent.
  --
  -- ⚠️⚠️ DO NOT REINTRODUCE A LEVEL-SCALED RATE. A (level/cap)^2 share was tried here to stop safe
  -- early deaths paying as well per unit of effort as deep ones. It was solving a problem that did
  -- not exist: it had been sized against a cumulative-experience figure that was ~10x too high,
  -- because GetEXPForLevel returns the TOTAL to reach a level and had been summed as though it were
  -- the per-level increment. Against the real curve the whole run to cap is 665,000, and scaling it
  -- down as well leaves nothing to award. Effort is already superlinear in level because the curve
  -- itself is quadratic -- level 35 costs 3.2x what level 20 does -- so 1:1 on experience is ALREADY
  -- depth-weighted. Squaring it a second time double counts.
  local rate = 1.0

  -- ⚠️⚠️ BANK AA *EXPERIENCE* AND CARRY THE REMAINDER -- do NOT floor run experience into whole
  -- points and discard the rest. A full run to cap is only 3.32 points, so flooring throws away up
  -- to a third of it, and every death below level 26 (200,000 xp) would pay literally nothing. The
  -- leftover is kept in `aa_xp_<charid>` and added to the next death, so nothing earned is ever lost
  -- and early runs accumulate toward a point instead of evaporating. This is also what makes the
  -- sheet's fractional "AA percent" meaningful rather than decorative.
  local gained = math.floor((run_xp or 0) * rate)
  local pool   = (tonumber(eq.get_data("aa_xp_" .. cid)) or 0) + gained
  local banked = math.floor(pool / AA_EXP_PER_POINT)
  eq.set_data("aa_xp_" .. cid, tostring(pool - banked * AA_EXP_PER_POINT))

  if banked > 0 then
    client:Message(MT.Yellow, string.format(
      "Your journey is distilled into %d point%s of advancement.", banked, banked == 1 and "" or "s"))
    aa_choice.grant_picks(e, banked)
  elseif gained > 0 then
    -- Tell them it counted even when it did not reach a whole point, or a real run reads as wasted.
    client:Message(MT.Yellow, string.format(
      "Your journey leaves its mark. (%d of %d toward your next point of advancement.)",
      pool, AA_EXP_PER_POINT))
  end
  -- ⚠️ SetEXP above keeps AA exp intact, so anything earned through ordinary AA experience is
  -- untouched by this -- the banked points are a SEPARATE private pool (aa_bank_<charid>) that only
  -- the picker can spend, which is what stops a player bypassing it via the native AA window.

  -- ROGUELITE LOSS: destroy carried gear/inventory/money + wipe spells (bank + epics are safe),
  -- then report what was lost (chat + the "what you lost" dll window via LOSTDATA).
  local lost = death_loss.process(client)
  death_loss.announce(client, lost, killer)   -- killer is captured above, for the Death Book
  spell_choice.clear_pending(client)  -- drop any un-picked offers so the new run starts clean
  spell_choice.send_unlocks(client)   -- re-hide the now-reset combat skills on the client

  -- RESET QUESTS: fully wipe the starting-quest state so a death restarts the tutorial as if entered for
  -- the FIRST time (the tutorial is the roguelite's way out -- a dead player MUST be able to re-run it).
  -- CancelAllTasks only clears the CLIENT'S in-memory task slots (it's built for #task reloadall) -- it
  -- does NOT delete the character_tasks DB rows, so Quest-type tasks (e.g. 5745 New Beginnings, 5166) just
  -- reload on the next zone and appear "stuck". RemoveTaskByTaskID deletes from memory AND the DB, and
  -- UncompleteTask clears any completion record. Remove the specific tasks FIRST (they must still be in
  -- the active list to be found), then CancelAllTasks to refresh the client's view of anything else.
  for _, tid in ipairs(TUTORIAL_TASKS) do
    client:RemoveTaskByTaskID(tid)   -- active task -> gone from memory + DB (no fail popup / lockout)
    client:UncompleteTask(tid)       -- clear any completed record so it can be taken again
  end
  client:CancelAllTasks()

  -- ROGUELITE START: every death sends the player back to the Tutorial, whose refreshed quests + free
  -- (Mythic) loot are the intended "get started". Clear the Tutorial's gate qglobals so its tasks are
  -- re-handed on arrival (tutorialb/player.pl only assigns while tutpop is unset).
  eq.delete_global("tutpop"); eq.delete_global("tutbind")
  eq.delete_global("amote");  eq.delete_global("bmote")

  -- TELEPORT-ONLY (bind untouched): respawn reads bind slot 0, so point slot 0 at the Tutorial for THIS
  -- respawn while preserving the player's real bind -- saved once here, restored when they next leave the
  -- Tutorial (see event_enter_zone). Save only if not already saved, so repeated Tutorial deaths don't
  -- overwrite the real bind with the Tutorial one.
  local bkey = "deathbind_" .. client:CharacterID()
  if (eq.get_data(bkey) or "") == "" then
    eq.set_data(bkey, string.format("%d,%.2f,%.2f,%.2f,%.2f",
      client:GetBindZoneID(), client:GetBindX(), client:GetBindY(), client:GetBindZ(), client:GetBindHeading()))
  end
  client:SetBindPoint(189, 0, 2.0, -146.0, 19.6, 303.75)   -- tutorialb start (EVENT_ENTERZONE re-positions)

  -- Personal death recap: the dying player's own client can swallow the world_emote during its death
  -- sequence, so guarantee they see their own line here (after the reset settles).
  client:Message(MT.Yellow, string.format("You were slain by %s at level %d.  You have now died %d %s.",
    killer, death_level, deaths, times))

  -- NOTE: the STARTER WEAPON is granted in event_death_complete, NOT here. event_death fires BEFORE
  -- the death corpse is built (attack.cpp: EVENT_DEATH ~1833, `new Corpse` ~2065) -- anything summoned
  -- to the cursor here gets swept into that corpse and lost. death_complete fires after the corpse.
end

-- STARTER WEAPON: the wipe (death_loss) strips equipped weapons (epics excepted), so a fresh run would
-- otherwise be fists-only. Grant a basic weapon whenever the Primary slot ends up empty. Done in
-- death_complete (fires AFTER the corpse is built) so the summoned item lands on the cursor and stays.
function event_death_complete(e)
  local client = e.self
  -- GetItemIDAt returns INVALID_ID (-1), NOT 0, for an empty slot -- test `<= 0`, not `== 0`.
  local primary = client:GetItemIDAt(13)       -- slot 13 = Primary
  if not primary or primary <= 0 then
    client:SummonItem(9998)                    -- Short Sword* (dmg 4 / dly 29, no req) -- the exact sword Absor's
                                               -- tutorial quest accepts (check_handin 9998 -> Sharpened Short Sword).
                                               -- Stays BASE (9998 is excluded from gear tiers) so the hand-in matches;
                                               -- Absor then hands back the Mythic Sharpened Short Sword.
  end

  -- REFRESH THE TUTORIAL for a DIED-IN-THE-TUTORIAL player. event_death already cancelled all tasks and
  -- cleared the gate qglobals, but a same-zone respawn (their death bind is now the Tutorial) does NOT
  -- re-fire EVENT_ENTERZONE, so we re-hand the tasks directly here and re-gate tutpop. Players who died
  -- ELSEWHERE respawn INTO the Tutorial (a real zone change), so tutorialb/player.pl's EVENT_ENTERZONE
  -- re-assigns them there instead.
  if eq.get_zone_id() == 189 then
    client:AssignTask(1448)                    -- Basic Training
    client:AssignTask(5166)                    -- paired Tutorial task
    eq.set_global("tutpop", "1", 5, "D30")     -- re-gate so EVENT_ENTERZONE won't re-assign/re-popup
  end
end

function event_say(e)
  -- GM-only: buff every online player server-wide. Usage: /say buffall <spellid>
  -- world_wide_cast_spell casts the spell on all online clients across all zones (uses the spell's
  -- own duration). Gated to GM (status >= 80) so ordinary players can't buff the server.
  local buff_id = e.message:match("^buffall (%d+)$")
  if buff_id then
    if e.self:GetGMStatus() >= 80 then
      local sid = tonumber(buff_id)
      eq.world_wide_cast_spell(sid)
      e.self:Message(MT.Yellow, string.format("[GM] Cast spell %d on all online players.", sid))
    end
    return
  end

  -- Delve window: "delve" / "delveenter <level>" / "delveexit". Returns true when it consumed the
  -- line, so the rest of event_say is skipped for its own commands.
  if aotv4_dungeon.handle_say(e) then return end

  -- consume "spellpick <N>" from the level-up reward window (the only reward picker now)
  spell_choice.handle_say(e)
  -- ⚠️ The AA picker owns "/say aapick <n>". Both handlers are called because they match different
  -- triggers and each ignores anything that is not its own -- do NOT early-return from one of them.
  aa_choice.handle_say(e)
  spell_choice.handle_journal_say(e)  -- "sjpool <level>" -- Spell Journal window's Pool tab

  -- "lostlog" -- the permanent death-loss history for the You Lost window. The dll swallows the reply.
  if (e.message or ""):lower():match("^lostlog%s*$") then
    death_loss.send_log(e.self)
    return
  end
  pok_travel.handle_say(e)        -- "portals" (list) + "portalgo <short>" (travel)
  bazaar_broker.handle_global_say(e)  -- vendor window: "vpset .../vshop/vclose"

  -- AoTv4 in-game search ("allaclone") -- backs the /search overlay. All swallowed by the dll.
  --   "srch <item|npc|spell> <term>"  -> SRCHDATA <kind>^id|name^id|name^...   (result list)
  --   "srchdet <item|npc|spell> <id>" -> SRCHDET <kind>|<id>|<detail ~-line-separated>
  -- AoTv4 quest journal: "qtrack <n>" / "quntrack <n>" (§ aotv4_questjournal.lua)
  if questjournal.handle_say(e) then return end

  -- Spell ranks: "spellkeep <id>" / "spellrelease <id>" / "spellrank <id>" / "spellkept"
  if spell_ranks_sys.handle_say(e) then return end

  -- Region unlocks: "regions" / "openregion <n>"
  if aotv4_regions.handle_say(e) then return end

  -- Reforge: "reforgerace <n>" / "reforgeclass <n>"
  if aotv4_reforge.handle_say(e) then return end

  local skind, sterm = e.message:match("^srch (%a+) (.+)$")
  if skind then
    -- ⚠️⚠️ OUR KINDS ARE TRIED FIRST AND THE NATIVE ONES ARE LEFT ALONE. `quest` and `tracked` are
    -- served from the Lua catalogue; item/npc/spell/recipe still go to the C++ SearchList exactly as
    -- before, so the search window's original four modes are untouched by this feature.
    local ours = questjournal.search_kind(e.self, skind, sterm)
    e.self:Message(MT.NPCQuestSay, "SRCHDATA " .. skind .. "^" ..
      (ours or e.self:SearchList(skind, sterm) or ""))
    return
  end
  local dkind, did = e.message:match("^srchdet (%a+) (%d+)$")
  if dkind then
    local ours = questjournal.detail_kind(e.self, dkind, tonumber(did))
    local body = ours or e.self:SearchDetail(dkind, tonumber(did)) or ""
    -- Cross reference: an ITEM lookup also lists the quests that want it, which is the question
    -- people actually have about an unfamiliar drop. Appended to the NATIVE detail rather than
    -- replacing it, so the stats block is untouched.
    if dkind == "item" then body = body .. questjournal.item_usage(tonumber(did)) end
    e.self:Message(MT.NPCQuestSay, "SRCHDET " .. dkind .. "|" .. did .. "|" .. body)
    return
  end
end

-- Discover a Plane of Knowledge portal by clicking that zone's PoK book (a door to poknowledge).
-- Return 1 to CANCEL the door's normal teleport -- the book only attunes/discovers; actual travel
-- is via the Portal window. (Handle_OP_ClickDoor skips HandleClick when the event returns non-zero.)
function event_click_door(e)
  if e.door and e.door:GetDestinationZoneName() == "poknowledge" then
    pok_travel.discover(e.self, eq.get_zone_short_name(), e.door:GetDoorID())  -- attune (first click); doorid splits multi-book zones
    pok_travel.open(e.self)                                -- open the Portal window
    return 1
  end
  -- AoTv4: the "BFIRE" fire in Oasis of Marr ports to the Plane of Hate (zone 76) at the normal port-in.
  -- Done in Lua (MovePC) like the PoK travel books, so it never depends on the native door-teleport path.
  if e.door and e.door:GetDestinationZoneName() == "hateplane" then
    e.self:MovePC(76, -353.08, -374.8, 3.75, 0)
    return 1
  end
end

test_items = {
    [Class.WARRIOR]			= {38000, 38020}, -- Warrior
    [Class.CLERIC]			= {38168, 38188}, -- Cleric
    [Class.PALADIN]			= {38084, 38104}, -- Paladin
    [Class.RANGER]			= {38105, 38125}, -- Ranger
    [Class.SHADOWKNIGHT]	= {38063, 38083}, -- Shadowknight
    [Class.DRUID]			= {38189, 38209}, -- Druid
    [Class.MONK]			= {38021, 38041}, -- Monk
    [Class.BARD]			= {38147, 38167}, -- Bard
    [Class.ROGUE]			= {38042, 38062}, -- Rogue
    [Class.SHAMAN]			= {38210, 38230}, -- Shaman
    [Class.NECROMANCER]		= {38294, 38314}, -- Necromancer
    [Class.WIZARD]			= {38231, 38251}, -- Wizard
    [Class.MAGICIAN]		= {38252, 38272}, -- Magician
    [Class.ENCHANTER]		= {38273, 38293}, -- Enchanter
    [Class.BEASTLORD]		= {38126, 38146}, -- Beastlord
    [Class.BERSERKER]		= {38315, 38332}, -- Berserker
}
 
function event_test_buff(e)
    if (e.self:GetLevel() < 25) then
        e.self:SetLevel(25)
        eq.scribe_spells(25,1)
        eq.train_discs(25,1)
        for class_id, v in pairs(test_items) do
            if e.self:GetClass() == class_id then
                for item_id = v[1], v[2] do
                    e.self:SummonItem(item_id);
                end
            end
        end
    end
end

-- ⚠️ Delve: the trash goal completing is what SPAWNS THE BOSS. Every delve task has a second step
-- (kill the warden) that the task system opens only once step 1 is done -- so without this hook the
-- player clears the dungeon and is left with an objective nothing can ever satisfy.
function event_task_stage_complete(e)
  aotv4_dungeon.on_task_stage_complete(e)
end

function event_task_complete(e)
  don.on_task_complete(e.self, e.task_id)
  -- Delve system: clearing a layer unlocks the next one and drops the reward chest where the last
  -- objective ticked over. Ignores every task that is not one of its six.
  aotv4_dungeon.on_task_complete(e)
end

-- Custom "when you are hit" abilities: Divine Aura, Blade Turn, Counterattack,
-- Vengeful Aura. The RETURN VALUE matters here -- zone/attack.cpp:4404 uses it
-- as damage_override (negative negates the hit, positive replaces it, 0 leaves
-- it alone), so this must return what the module hands back.
function event_damage_taken(e)
  return aotv4_reactions.on_damage_taken(e)
end

-- Open Wounds banks a share of every hit on a marked target as pending bleed, and the Thirst line
-- heals a flat amount per melee hit (it has no spell effect of its own -- see aotv4_thirst.lua).
-- Reactions run FIRST and own the return value: a negative override negates the hit, and a hit that
-- never landed must not pay out a leech.
function event_damage_given(e)
  local override = aotv4_reactions.on_damage_given(e)
  if not (override and override < 0) then
    aotv4_thirst.on_damage_given(e, e.self)
    aotv4_aa_tank.on_damage_given(e, e.self)   -- Bloodied Bash: Bash/Slam leeches
  end
  return override
end

-- Delve: camping out (/q) inside a delve ends the run, exactly like dying or walking a zone line.
-- Without this the character is saved standing inside the instance, the instance is orphaned, and the
-- run bucket keeps claiming they are in a dungeon they have left. See aotv4_dungeon.M.on_disconnect.
function event_disconnect(e)
  aotv4_dungeon.on_disconnect(e.self)
end
