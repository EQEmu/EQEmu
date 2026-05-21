-- Progressive Dungeons NPC scaling: NPCs spawned in any expedition instance
-- whose `pd_inst_<id>_tier` marker is set are scaled to that tier level,
-- with HP and damage scaled proportionally.

local PD_DUNGEON_SHORTS = {
  blackburrow = true, befallen = true, crushbone = true, runnyeye = true,
  soldunga    = true, guktop   = true, gukbottom = true, dalnir   = true,
  crystal     = true, frozenshadow = true, mistmoore = true, chardok = true,
  sebilis     = true, charasis = true, citymist  = true, akheva   = true,
  sseru       = true,
}

local function pd_scale_npc(npc)
  if not npc then return end
  if npc:IsPet() or npc:IsCharmed() then return end

  local instance_id = eq.get_zone_instance_id()
  if not instance_id or instance_id <= 0 then return end

  local zone_short = eq.get_zone_short_name()
  if not PD_DUNGEON_SHORTS[zone_short] then return end

  local tier = tonumber(eq.get_data('pd_inst_' .. instance_id .. '_tier'))
  if not tier or tier < 1 then return end

  -- Raid-flagged mobs are removed entirely from PD expeditions. They can
  -- trigger engine crashes (e.g. fear-pathing on null map pointers) and
  -- are out of scope for these scaled solo/group missions.
  if npc.IsRaidTarget and npc:IsRaidTarget() then
    npc:Depop(false)
    return
  end

  local old_level = npc:GetLevel() or 1
  if old_level < 1 then old_level = 1 end

  local ratio = tier / old_level

  local old_max_hp = npc:GetMaxHP() or 1
  local new_max_hp = math.max(1, math.floor(old_max_hp * ratio))

  local old_min = npc:GetMinDMG() or 1
  local old_max = npc:GetMaxDMG() or 1
  local new_min = math.max(1, math.floor(old_min * ratio))
  local new_max = math.max(new_min, math.floor(old_max * ratio))

  -- Era-appropriate baselines derived from tier (= NPC level).
  -- These intentionally overwrite the raid-tier offensive/defensive stats so
  -- low-tier dungeons remain a low-tier experience even when the underlying
  -- spawn template is a high-end mob.
  local new_atk      = math.max(10, tier * 5)
  local new_ac       = math.max(10, tier * 5)
  local new_resist   = math.min(200, math.max(5, tier * 3))
  local new_stat     = math.max(10, tier * 3)

  npc:ModifyNPCStat('level',           tostring(tier))
  npc:ModifyNPCStat('max_hp',          tostring(new_max_hp))
  npc:ModifyNPCStat('min_hit',         tostring(new_min))
  npc:ModifyNPCStat('max_hit',         tostring(new_max))
  npc:ModifyNPCStat('atk',             tostring(new_atk))
  npc:ModifyNPCStat('ac',              tostring(new_ac))
  npc:ModifyNPCStat('accuracy',        '0')
  npc:ModifyNPCStat('avoidance',       '0')
  npc:ModifyNPCStat('slow_mitigation', '0')
  npc:ModifyNPCStat('mr',              tostring(new_resist))
  npc:ModifyNPCStat('fr',              tostring(new_resist))
  npc:ModifyNPCStat('cr',              tostring(new_resist))
  npc:ModifyNPCStat('pr',              tostring(new_resist))
  npc:ModifyNPCStat('dr',              tostring(new_resist))
  npc:ModifyNPCStat('corrup',          tostring(new_resist))
  npc:ModifyNPCStat('str',             tostring(new_stat))
  npc:ModifyNPCStat('sta',             tostring(new_stat))
  npc:ModifyNPCStat('agi',             tostring(new_stat))
  npc:ModifyNPCStat('dex',             tostring(new_stat))
  npc:ModifyNPCStat('wis',             tostring(new_stat))
  npc:ModifyNPCStat('int',             tostring(new_stat))
  npc:ModifyNPCStat('cha',             tostring(new_stat))

  -- Strip out-of-era abilities: spell list and special attacks.
  npc:ModifyNPCStat('npc_spells_id',     '0')
  npc:ModifyNPCStat('special_abilities', '')

  -- Strip all loot and money: out-of-era drops are not desired in PD missions.
  -- Rewards are intended to come from the task / mission credit, not NPC drops.
  npc:ModifyNPCStat('loottable_id', '0')
  if npc.ClearItemList then npc:ClearItemList() end
  if npc.AddCash then npc:AddCash(0, 0, 0, 0) end
end

function event_spawn(e)
    -- Progressive Dungeons scaling
    pd_scale_npc(e.self)

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
