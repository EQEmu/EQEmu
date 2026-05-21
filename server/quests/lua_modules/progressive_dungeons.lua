-- Progressive Dungeons shared logic, used by global_player.lua.
-- Keeps tier/task bookkeeping in one place so it applies in every dungeon zone
-- (Blackburrow, Befallen, Crushbone, etc.) without per-zone player.lua copies.

local M = {}

local PD_DUNGEON_SHORTS = {
  blackburrow = true, befallen = true, crushbone = true, runnyeye = true,
  soldunga    = true, guktop   = true, gukbottom = true, dalnir   = true,
  crystal     = true, frozenshadow = true, mistmoore = true, chardok = true,
  sebilis     = true, charasis = true, citymist  = true, akheva   = true,
  sseru       = true,
}

local TASK_BY_DUNGEON = {
  blackburrow  = 990201,
  befallen     = 990202,
  crushbone    = 990203,
  runnyeye     = 990204,
  soldunga     = 990205,
  guktop       = 990206,
  gukbottom    = 990207,
  dalnir       = 990208,
  crystal      = 990209,
  frozenshadow = 990210,
  mistmoore    = 990211,
  chardok      = 990212,
  sebilis      = 990213,
  charasis     = 990214,
  citymist     = 990215,
  akheva       = 990216,
  sseru        = 990217,
}

local PD_TASK_SET = {}
for _, task_id in pairs(TASK_BY_DUNGEON) do
  PD_TASK_SET[task_id] = true
end

local KEY_HIGHEST         = 'pd_highest_completed'
local KEY_TOTAL           = 'pd_total_completed'
local KEY_ACTIVE_TIER     = 'pd_active_tier'
local KEY_ACTIVE_INSTANCE = 'pd_active_instance_id'

local REWARD_PLATINUM = 25 -- placeholder reward per mission completion

local function get_active_tier(client)
  return tonumber(client:GetBucket(KEY_ACTIVE_TIER) or '0') or 0
end

local function in_dungeon_instance()
  local instance_id = eq.get_zone_instance_id()
  if not instance_id or instance_id <= 0 then return false end
  local zone_short = eq.get_zone_short_name()
  return PD_DUNGEON_SHORTS[zone_short] == true
end

-- Called from global_player.lua event_enter_zone.
function M.on_enter_zone(client)
  if not in_dungeon_instance() then return end

  local dz = client:GetExpedition()
  if not dz or not dz.valid then return end

  local task_id = TASK_BY_DUNGEON[eq.get_zone_short_name()]
  if not task_id then return end

  if not client:IsTaskActive(task_id) then
    client:AssignTask(task_id)
    client:Message(MT.NPCQuestSay, 'Your quest journal has been updated with your mission objective.')
  end
end

-- Called from global_player.lua event_task_complete.
function M.on_task_complete(client, task_id)
  if not PD_TASK_SET[task_id] then return end

  local active_tier = get_active_tier(client)
  if active_tier <= 0 then return end

  local highest = tonumber(client:GetBucket(KEY_HIGHEST) or '0') or 0
  local total   = tonumber(client:GetBucket(KEY_TOTAL)   or '0') or 0

  if active_tier > highest then highest = active_tier end
  total = total + 1

  client:SetBucket(KEY_HIGHEST,         tostring(highest))
  client:SetBucket(KEY_TOTAL,           tostring(total))
  client:SetBucket(KEY_ACTIVE_TIER,     '0')
  client:SetBucket(KEY_ACTIVE_INSTANCE, '0')

  if REWARD_PLATINUM and REWARD_PLATINUM > 0 then
    client:AddMoneyToPP(0, 0, 0, REWARD_PLATINUM, true)
    client:Message(MT.Yellow,
      string.format('You receive %d platinum as a mission reward.', REWARD_PLATINUM))
  end

  client:Message(MT.Yellow,
    'Mission complete. Return to the recruiter to begin your next tier.')
end

return M
