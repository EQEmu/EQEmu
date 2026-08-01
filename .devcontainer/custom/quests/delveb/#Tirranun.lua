local tir			= nil;
local tir_min_hit	= 568;
local tir_max_hit	= 1841;
local banish		= true;
local has_died		= false;
local can_die		= false;
local phaseone		= true;

function event_combat(e)
	if e.joined and banish then
		e.self:SetSpecialAbility(SpecialAbility.summon, 0);
		-- AoTv4: 45s "banish_top" eject timer removed -- it threw the top-aggro player out of the
		-- instance, making the fight unwinnable solo. (The `banish` flag stays -- it's also this
		-- encounter's phase-1 marker, gating the phase-2 regen in event_hp.)
		can_die = false;					-- make sure I can't die.
	elseif not e.joined and phaseone then -- If I lose combat, depop myself, depop my adds, stop all timers, tell myself to respawn in 10 seconds with all variables default.
		eq.set_timer("Heal", 5000);
		has_died = false;
		banish   = true;
		can_die  = false;
	elseif not e.joined and not phaseone then
		eq.depop(342066);									-- Depop my old self
		eq.depop_all(342065);								-- Repop my new  due to my increased HP pool gain from phase 2.
		eq.spawn2(342066,0,0,-742.57,2866.09,41.52,9.0);	-- NPC: #Tirranun
		phaseone = true;									-- Set my phase back to 1.
	end
end

function event_death(e)
    if not has_died then											-- If I "Die"
		e.self:CastSpell(6538, e.self:GetID());						-- Spell: Expulsion of Flame
		e.self:SetAppearance(3);										-- Fall over
		e.self:WipeHateList();										-- Wipe my hate list.
		e.self:SetSpecialAbility(SpecialAbility.immune_aggro, 1);
		e.self:SetSpecialAbility(SpecialAbility.immune_aggro_on, 1);	-- Set variables immune agro, agro on, no harm, immune magic.
		e.self:SetSpecialAbility(SpecialAbility.no_harm_from_client, 1);
		e.self:SetSpecialAbility(SpecialAbility.immune_magic, 1);
		e.self:TempName("Tirranun`s ashen form");						-- set my name
		e.self:ModifyNPCStat("hp_regen", "8250");						-- set my new regen stat
		eq.stop_timer('banish_top');									-- stop the banish timer.
		eq.set_timer("Aoe",60 * 1000);								-- start my aoe timer in this phase until db can pick it back up.
		eq.set_timer("Adds", 20 * 1000);								-- start spawning adds every 20 seconds.
		banish = false;												-- tell banish to not happen when I respawn
		eq.set_next_inc_hp_event(80);									-- set something to happen when I regen to 80
		has_died = true;												-- let myself know I can die now for real.
		eq.zone_emote(MT.Yellow,"As Tirranun is struck down, every last ember of flame is expelled from his volcanic body. His broken husk smolders quietly on the rock. Alarmingly, it appears to be getting warmer. He is recovering.");
		return 1 -- force myself to stay alive.
	end

	if can_die then
		return 0;
	else
		return 1;
	end
end

function event_spawn(e)
	tir = e.self;
end

function event_timer(e)
	if e.timer == "banish_top" then
		-- AoTv4: player-eject removed. The timer is no longer started (see event_combat), so this
		-- branch never fires; kept as a no-op so the elseif chain and any stray
		-- eq.stop_timer('banish_top') calls remain valid.
	elseif e.timer == "Form" then
		e.self:CastSpell(6540, e.self:GetID()); -- Spell: Form of Lava
	elseif e.timer == "Aoe" then
		e.self:CastSpell(6536, e.self:GetID()); -- Spell: Tirranun's Incinerating Aura
	elseif e.timer == "Aoe_two" then
		e.self:CastSpell(6537, e.self:GetID()); -- Spell: Breath of Tirranun
	elseif e.timer == "Heal" then
		eq.stop_all_timers();				-- Stop all timers
		e.self:SetHP(e.self:GetMaxHP());	-- if no one is on my hate list... heal to full.
	elseif e.timer == "Adds" then
		eq.spawn2(342065,0,0, e.self:GetX(), e.self:GetY(), e.self:GetZ(), e.self:GetHeading());	-- NPC: #whorl_of_searing_ashes
		e.self:AddToHateList(e.self:GetTarget(),1);													-- Spawn adds / add them to hate list.
	end
end

function event_hp(e)
	if e.inc_hp_event == 80 and not banish then			-- If I've regened to 80.
		e.self:SetAppearance(0);						-- Stand myself up
		e.self:TempName("Tirranun`s Blazing Body ");	-- Change my name
		e.self:CastSpell(6540, e.self:GetID());			-- cast Form of Lava
		eq.set_timer("Form", 25000);					-- Maintain this buff every 25 seconds.
		eq.set_timer("Adds", 15000);					-- Set adds to spawn faster, every 15 seconds now.
		eq.set_next_inc_hp_event(90);					-- do something at 90%
	elseif e.inc_hp_event == 90 and not banish then								-- If I've regened to 90.
		e.self:TempName("Tirranun the Ancient Protector");						-- Change my name
		e.self:ModifyNPCStat("min_hit", tostring(tir_min_hit));					-- modify my min hit.
		e.self:ModifyNPCStat("max_hit", tostring(tir_max_hit));					-- modify my max hit.
		e.self:ModifyNPCStat("max_hp", tostring(e.self:GetMaxHP() + 800000));	-- add 800,000HP to my max pool.
		e.self:SetHP(e.self:GetMaxHP());										-- Heal myself to 100.
		e.self:ModifyNPCStat("hp_regen", "6250");								-- reset my regen back to default.
		e.self:SetSpecialAbility(SpecialAbility.immune_aggro, 0);
		e.self:SetSpecialAbility(SpecialAbility.immune_aggro_on, 0);
		e.self:SetSpecialAbility(SpecialAbility.no_harm_from_client, 0);		-- allow everyone to agro, and allow myself to agro.
		e.self:SetSpecialAbility(SpecialAbility.immune_magic, 0);
		e.self:SetSpecialAbility(SpecialAbility.summon, 1);
		e.self:AddToHateList(e.self:GetTarget(),1); 
		eq.set_timer("Adds", 10 * 1000);										-- set add timers to max timer of 10sec.
		eq.set_timer("Aoe_two", 45 * 1000);										-- start my new aoe timer.
		can_die = true;															-- let myself know I can die properly now.
		phaseone = false;
	end
end

function event_death_complete(e)
	eq.spawn2(342067,0,0,e.self:GetX(),e.self:GetY(),e.self:GetZ(),e.self:GetHeading()); -- Spawn a chest with loot at my location upon death.
	eq.stop_all_timers(); -- stop timers.
end
