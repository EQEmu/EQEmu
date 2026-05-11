-- Raid Target Suppression for XP Dynamic Zone instances
-- This is a zone controller script that depops raid targets when
-- the zone is loaded as an XP DZ instance.
--
-- Install as ZONE_CONTROLLER.lua in each XP DZ-enabled zone directory
-- (e.g., quests/crushbone/ZONE_CONTROLLER.lua)
--
-- The raid target NPC type IDs are read from the
-- monomyth_xp_dz_raid_suppress table via eq.query.
-- Alternatively, hardcode them in the RAID_TARGET_IDS table below.

local RAID_TARGET_IDS = {
    -- Example: Crushbone Emperor Crush (replace with actual NPC type IDs)
    -- [68093] = true,
    -- [68094] = true,
}

local function load_raid_suppression_from_db(zone_id)
    local results = eq.query(
        string.format("SELECT npc_type_id FROM monomyth_xp_dz_raid_suppress WHERE zone_id = %d", zone_id)
    )
    if results then
        for _, row in ipairs(results) do
            RAID_TARGET_IDS[tonumber(row.npc_type_id)] = true
        end
    end
end

local function suppress_raid_targets_in_xp_dz()
    if not zone or zone:GetInstanceID() == 0 then
        return
    end

    local dz = zone:GetDynamicZone()
    if not dz then
        return
    end

    local dz_type = dz:GetType()
    -- DynamicZoneType.XPDZ = 6
    if dz_type ~= 6 then
        return
    end

    load_raid_suppression_from_db(zone:GetZoneID())

    for npc_type_id, _ in pairs(RAID_TARGET_IDS) do
        eq.depop_all(npc_type_id)
    end
end

function event_spawn_zone(e)
    suppress_raid_targets_in_xp_dz()
end
