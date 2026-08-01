local aotv4_worldboss = require("aotv4_worldboss")  -- roaming world boss: loot rights on death
local aotv4_dungeon = require("aotv4_dungeon")  -- scaling dungeon ("Delve")

function event_spawn(e)
    -- Delve: scale this mob to the layer's level if it is spawning inside a delve instance.
    -- ⚠️ FIRST THING IN THE HOOK and before any of the seasonal string matching below: it
    -- early-outs on the instance id, so the normal world pays one integer compare for it.
    aotv4_dungeon.on_npc_spawn(e)

    -- peq_halloween
    if (eq.is_content_flag_enabled("peq_halloween")) then
        -- exclude mounts and pets
        if (e.self:GetCleanName():findi("mount") or e.self:IsPet()) then
            return;
        end

        -- soulbinders
        -- priest of discord
        if (e.self:GetCleanName():findi("soulbinder") or e.self:GetCleanName():findi("priest of discord")) then
            e.self:ChangeRace(eq.ChooseRandom(14,60,82,85));
            e.self:ChangeSize(6);
            e.self:ChangeTexture(1);
            e.self:ChangeGender(2);
        end

        -- Shadow Haven
        -- The Bazaar
        -- The Plane of Knowledge
        -- Guild Lobby
        local halloween_zones = eq.Set { 202, 150, 151, 344 }
        local not_allowed_bodytypes = eq.Set { 11, 60, 66, 67 }
        if (halloween_zones[eq.get_zone_id()] and not_allowed_bodytypes[e.self:GetBodyType()] == nil) then
            e.self:ChangeRace(eq.ChooseRandom(14,60,82,85));
            e.self:ChangeSize(6);
            e.self:ChangeTexture(1);
            e.self:ChangeGender(2);
        end
    end
end

-- ⚠️ There is deliberately NO event_damage_given here any more. It existed solely to drive the
-- summoned-pet behaviours of the retired 43000-43112 ability set (Leopard backstab, Skeleton
-- lifetap-heal, Willowisp mana leech, Fire Imp burn) and did nothing else, so it was removed with
-- them. The player-side damage hooks (Thirst, Sinew, reactions) live in global_player.lua and are
-- untouched -- do not re-add this one expecting to find them here.

-- AoTv4: the roaming world boss can die in any zone, so its loot rights are granted from the global
-- NPC hook rather than a per-zone script. Everyone on its hate list may loot -- see
-- lua_modules/aotv4_worldboss.lua.
function event_death_complete(e)
  aotv4_worldboss.on_death(e)

  -- Delve: bank what this creature was worth into the run's difficulty ledger. Keyed off the mob's
  -- OWN scaled level, stamped when it spawned, so no later change of gear can revalue it.
  aotv4_dungeon.on_npc_death(e)

  -- Delve: opening the reward chest ends the run and closes the instance.
  -- ⚠️ Keyed on the chest's npc type id, not on its name -- a_delve_reward_chest is a clone of
  -- a_gilded_chest and there are several similarly named chests in the world.
  -- ⚠️ e.other is a Lua_Mob; on_chest_looted needs a real Lua_Client (CharacterID is not on Lua_Mob).
  if e.self and e.self.valid and e.self:GetNPCTypeID() == aotv4_dungeon.CHEST_NPC then
    aotv4_dungeon.on_chest_looted(aotv4_dungeon.as_client(e.other))
  end
end
