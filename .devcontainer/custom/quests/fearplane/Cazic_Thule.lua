-- Call All mobs to aid Cazic Thule if they're still alive
-- items: 18898, 8226, 18899, 7836

function event_spawn(e)
	eq.set_timer("Shout",600000);
	-- use a timer for the zone repop so that the entire zone is not popping twice immediately on zone bootup.
	eq.set_timer("RepopZone",5000);
end

function event_say(e)
	if(e.message:findi("gandan has failed in his task")) then
		e.self:Emote("'s thoughts begin to pervade your own, they creep into your mind with great force. You feel pressure as if your head will explode. You see his thoughts becoming your own. You see in these visions a tome bound in flesh dropped to the ground. You then open your eyes to see that same book, and take it knowing that it was meant for you.");
		e.other:SummonItem(18898); -- Item: Flayed Skin Tome
	end
end

function event_death_complete(e)
	-- leave out broken golem, The Tempest Reaver, and Irak_Altil
	send_signal_to_all_npc_in_zone(3, {72078,72074,72012});
end

function event_trade(e)
	local item_lib = require("items");
	if(item_lib.check_turn_in(e.trade, {item1 = 8226, item2 = 18898, item3 = 18899})) then
		e.self:Emote("seems pleased with the amount of pain that you have been able to inflict. Cazic Thule then grabs your hands and begins to infuse them with his power. Your hands burn like they were placed in lava for a moment, then feel cool as ice. You can feel the sheer power flowing through your new weapons of pain.");
		e.other:SummonItem(7836); -- Item: Whistling Fists
		e.other:AddEXP(100000);
		e.other:Ding();
	end
	item_lib.return_items(e.self, e.other, e.trade)
end

-- ⚠️⚠️ AoTv4: CAZIC THULE NO LONGER CALLS THE ZONE. Stock sent signal 2 to every NPC in fearplane
-- when he entered combat, and signal 2 in each of the 24 trash scripts is `eq.follow(Cazic_Thule)`
-- -- so the whole plane pathed onto him and joined the fight. That is the classic behaviour and it
-- assumes classic trash: many weak mobs on a normal respawn.
--
-- It is removed because the trash here is being reworked into rare-spawn named mobs, which are
-- individually beefy. A zone-wide call that gathers a hundred ordinary mobs is a spectacle; the same
-- call gathering a hundred NAMED mobs is unkillable, and with normal respawn timers on top there is
-- no realistic way to keep the plane clear enough to fight him at all.
--
-- ⚠️ The SHOUT is kept but reworded. Stock bellowed "come forth to his aid" and then actually
-- delivered; leaving that line in place while sending no signal would promise reinforcements that
-- never arrive, which reads as a broken encounter rather than a changed one.
-- ⚠️ Signal 3 (stop following) is deliberately still sent on combat end and on death. Nothing should
-- be following him any more, so it is a no-op -- but it is the only thing that releases a mob that
-- got attached some other way, and it costs nothing to leave as a safety net.
-- ⚠️ Signal 1, the periodic shout on the 10 minute timer, is untouched: it only makes the denizens
-- answer him with a line of flavour text. It moves nobody.
function event_combat(e)
	if (e.joined) then
		e.self:Shout("Tremble, infidel. You stand in MY plane, and it will drink your fear!");
		eq.stop_timer("Shout");
	else
		-- leave out broken golem, The Tempest Reaver, and Irak_Altil
		send_signal_to_all_npc_in_zone(3, {72078,72074,72012});
		eq.set_timer("Shout",600000);
	end
end

function event_timer(e)
	if(e.timer == "Shout") then
		e.self:Shout("Beware all infidels who dare to taint my plane, for I shall rend your minds with fright, dread, and terror!");
		-- most mobs have an answer to my shout
		-- leave out broken golem, The Tempest Reaver, and Irak_Altil
		send_signal_to_all_npc_in_zone(1, {72078,72074,72012});
	elseif(e.timer == "RepopZone") then
		eq.spawn_condition("fearplane",0,1,0);
		eq.spawn_condition("fearplane",0,1,1);
		eq.stop_timer("RepopZone");
	end
end

function send_signal_to_all_npc_in_zone(signal_to_send,exclude_table)
	-- set to true for debugging messages
	local show_debug = false;
	-- create list of NPCID that will not be signalled.
	local exclude_npc_list = Set(exclude_table);
	-- create empty table to track the NPCID that have had signals sent already
	local signal_sent_to = {};
	-- grab the entity list
	local entity_list = eq.get_entity_list();
	-- get the list of npcs currently spawned in the zone
	local npc_list = entity_list:GetNPCList();
	-- do not do anything if there are no NPC's spawned. should be an impossible check because this is in an NPC script
	if(npc_list ~= nil) then
		for npc in npc_list.entries do
			if (exclude_npc_list[npc:GetNPCTypeID()] == nil and signal_sent_to[npc:GetNPCTypeID()] == nil) then
				-- make sure the npc is valid (again, should never fail, but better to be certain.
				if (npc.valid) then
					if (show_debug) then eq.zone_emote(MT.LightBlue,"NPCID: " .. npc:GetNPCTypeID() .. " is being sent signal " .. tostring(signal_to_send) .. "."); end
					-- send signal to this NPCID
					eq.signal(npc:GetNPCTypeID(),signal_to_send);
					-- add this NPCID to the list of NPCID that we have already signalled
					signal_sent_to[npc:GetNPCTypeID()] = true;
				end
			elseif signal_sent_to[npc:GetNPCTypeID()] then
				if (show_debug) then eq.zone_emote(MT.LightBlue,"NPCID: " .. npc:GetNPCTypeID() .. " has already been sent signal " .. tostring(signal_to_send) .. "."); end
			elseif exclude_npc_list[npc:GetNPCTypeID()] then
				if (show_debug) then eq.zone_emote(MT.LightBlue,"NPCID: " .. npc:GetNPCTypeID() .. " is excluded and will not be sent signal " .. tostring(signal_to_send) .. "."); end
			end
		end
	end
end

-- Set function example from Programming In Lua
-- http://www.lua.org/pil/11.5.html
function Set (list)
  local set = {}
  for _, l in ipairs(list) do set[l] = true end
  return set
end
