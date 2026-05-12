-- Raid DZ Zone Controller
-- Handles no-respawn behavior for Raid DZ instances.
--
-- Install as ZONE_CONTROLLER.lua in each Raid DZ-enabled zone directory
-- (e.g., quests/crushbone/ZONE_CONTROLLER.lua)
--
-- This script:
-- 1. Detects when the zone is a Raid DZ instance (type 7)
-- 2. Disables all respawn timers so killed mobs do not repop
-- 3. Raid targets from monomyth_raid_dz_targets are guaranteed present
--    at normal zone startup (standard spawn2 behavior)

local raid_dz_initialized = false

local function is_raid_dz_instance()
    if not zone or zone:GetInstanceID() == 0 then
        return false
    end

    local dz = zone:GetDynamicZone()
    if not dz then
        return false
    end

    return dz:GetType() == 7
end

function event_spawn_zone(e)
    if not raid_dz_initialized and is_raid_dz_instance() then
        raid_dz_initialized = true
        eq.disable_respawn_timers()
    end
end
