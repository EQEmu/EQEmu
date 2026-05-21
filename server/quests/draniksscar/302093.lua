-- Recruiter NPC (Instance) for Progressive Dungeons.
-- Picks a random dungeon from a pool, creates a tiered expedition, and stores
-- the tier in a per-instance data bucket so global_npc.lua can scale NPCs.

-- Display name -> zone short name (must match `zone.short_name` in DB).
local DUNGEON_POOL = {
  { name = 'Blackburrow',             short = 'blackburrow'  },
  { name = 'Befallen',                short = 'befallen'     },
  { name = 'Crushbone',               short = 'crushbone'    },
  { name = 'Runnyeye',                short = 'runnyeye'     },
  { name = "Solusek's Eye",           short = 'soldunga'     },
  { name = 'Upper Guk',               short = 'guktop'       },
  { name = 'Lower Guk',               short = 'gukbottom'    },
  { name = 'Crypt of Dalnir',         short = 'dalnir'       },
  { name = 'Crystal Caverns',         short = 'crystal'      },
  { name = 'Tower of Frozen Shadow',  short = 'frozenshadow' },
  { name = 'Castle Mistmoore',        short = 'mistmoore'    },
  { name = 'Chardok',                 short = 'chardok'      },
  { name = 'Old Sebilis',             short = 'sebilis'      },
  { name = 'Charasis',                short = 'charasis'     },
  { name = 'City of Mist',            short = 'citymist'     },
  { name = 'Akheva Ruins',            short = 'akheva'       },
  { name = 'Sanctus Seru',            short = 'sseru'        },
}

local MAX_TIER = 65 -- scales NPC level; per-dungeon task is reused

-- One task per dungeon so the quest journal shows the correct zone/name.
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

local KEY_HIGHEST         = 'pd_highest_completed'
local KEY_TOTAL           = 'pd_total_completed'
local KEY_ACTIVE_TIER     = 'pd_active_tier'
local KEY_ACTIVE_INSTANCE = 'pd_active_instance_id'
local KEY_ACTIVE_ZONE     = 'pd_active_zone'

local SAFE_RETURN = { zone = 'draniksscar', x = -447, y = 1722, z = -247, h = 324 }
local EXPED_TTL_SECONDS = eq.seconds('2h')

local function ensure_progress(client)
  local highest = tonumber(client:GetBucket(KEY_HIGHEST) or '0') or 0
  local total   = tonumber(client:GetBucket(KEY_TOTAL)   or '0') or 0
  local active  = tonumber(client:GetBucket(KEY_ACTIVE_TIER) or '0') or 0
  return { highest = highest, total = total, active = active }
end

local function set_progress(client, highest, total, active)
  client:SetBucket(KEY_HIGHEST,     tostring(highest or 0))
  client:SetBucket(KEY_TOTAL,       tostring(total   or 0))
  client:SetBucket(KEY_ACTIVE_TIER, tostring(active  or 0))
end

local function max_unlocked_tier(progress)
  local unlocked = progress.highest + 1
  if unlocked < 1 then unlocked = 1 end
  if unlocked > MAX_TIER then unlocked = MAX_TIER end
  return unlocked
end

local function pick_random_dungeon()
  return DUNGEON_POOL[math.random(1, #DUNGEON_POOL)]
end

local function assign_task_for_dungeon(client, dungeon_short, npc_id)
  local task_id = TASK_BY_DUNGEON[dungeon_short]
  if not task_id then return end
  if client:IsTaskActive(task_id) then return end
  if npc_id then
    client:AssignTask(task_id, npc_id)
  else
    client:AssignTask(task_id)
  end
  client:Message(MT.NPCQuestSay, 'Your quest journal has been updated.')
end

local function show_missions(e)
  local progress = ensure_progress(e.other)
  local unlocked_tier = max_unlocked_tier(progress)

  local lines = {}
  table.insert(lines, 'Available missions (dungeon is rolled randomly when you accept):')
  table.insert(lines, eq.say_link('random mission', false, '[Random Mission]'))

  for tier = 1, unlocked_tier do
    table.insert(lines, eq.say_link('mission ' .. tier, false, '[Mission ' .. tier .. ']'))
  end

  if progress.active > 0 then
    table.insert(lines, 'Active mission tier: ' .. progress.active .. ' (use the expedition window to abandon).')
  end

  e.other:Message(MT.NPCQuestSay, table.concat(lines, ' '))
end

local function create_mission_expedition(e, tier)
  if not tier or tier < 1 or tier > MAX_TIER then
    e.other:Message(MT.Red, 'Mission tier does not exist.')
    return
  end

  local progress = ensure_progress(e.other)
  local unlocked_tier = max_unlocked_tier(progress)
  if tier > unlocked_tier then
    e.other:Message(MT.Red, 'That mission is not unlocked yet.')
    return
  end

  local dungeon = pick_random_dungeon()
  local exped = {
    expedition = {
      name        = string.format('Progressive Dungeon Tier %d - %s', tier, dungeon.name),
      min_players = 1,
      max_players = 6,
    },
    instance   = { zone = dungeon.short, version = dungeon.version or 1, duration = EXPED_TTL_SECONDS },
    safereturn = SAFE_RETURN,
    zonein     = { x = 0, y = 0, z = 0, h = 0 },
  }

  local dz = e.other:CreateExpedition(exped)
  if not dz.valid then
    e.other:Message(MT.Red, 'Unable to create expedition. Try again in a moment.')
    return
  end

  local instance_id = tonumber(dz:GetInstanceID()) or 0
  if instance_id <= 0 then
    e.other:Message(MT.Red, 'Expedition was created but no instance id was returned.')
    return
  end

  -- Per-instance markers used by global_npc.lua scaling hook.
  -- Lifetime is intentionally a bit longer than the expedition so all NPCs see it.
  local ttl = 'D2' -- 2 days
  eq.set_data('pd_inst_' .. instance_id .. '_tier',  tostring(tier),     ttl)
  eq.set_data('pd_inst_' .. instance_id .. '_zone',  dungeon.short,      ttl)

  set_progress(e.other, progress.highest, progress.total, tier)
  e.other:SetBucket(KEY_ACTIVE_INSTANCE, tostring(instance_id))
  e.other:SetBucket(KEY_ACTIVE_ZONE,     dungeon.short)
  assign_task_for_dungeon(e.other, dungeon.short, e.self:GetID())

  e.other:Message(MT.NPCQuestSay,
    string.format('Your expedition has been created: %s (Tier %d). All enemies inside will be scaled to level %d.',
      dungeon.name, tier, tier))
  e.other:Message(MT.NPCQuestSay, 'Click the dungeon portal to enter your active mission.')
end

local function abandon_active_mission(e)
  local progress = ensure_progress(e.other)
  if progress.active <= 0 then
    e.other:Message(MT.Red, 'You do not have an active mission to abandon.')
    return
  end

  -- Clear active state only; do NOT credit highest/total. Tier must be earned
  -- by actually completing the in-zone task (handled by progressive_dungeons.lua).
  set_progress(e.other, progress.highest, progress.total, 0)
  e.other:SetBucket(KEY_ACTIVE_INSTANCE, '0')
  e.other:SetBucket(KEY_ACTIVE_ZONE, '')

  -- Drop any in-progress PD task so the journal is clean.
  for _, task_id in pairs(TASK_BY_DUNGEON) do
    if e.other:IsTaskActive(task_id) then
      e.other:FailTask(task_id)
    end
  end

  e.other:Message(MT.NPCQuestSay,
    'Your mission has been abandoned. You may request a new one, but tier ' .. progress.active .. ' is not credited.')
end

function event_say(e)
  if e.message:findi('hail') then
    e.other:Message(MT.NPCQuestSay,
      'I can assign scaling dungeon expeditions. Each mission rolls a random dungeon and scales all enemies to your tier level. Ask for [Missions].')
    return
  end

  if e.message:findi('missions') then
    show_missions(e)
    return
  end

  if e.message:findi('random mission') then
    local progress = ensure_progress(e.other)
    local unlocked_tier = max_unlocked_tier(progress)
    create_mission_expedition(e, math.random(1, unlocked_tier))
    return
  end

  local mission_tier = e.message:match('mission%s+(%d+)')
  if mission_tier then
    create_mission_expedition(e, tonumber(mission_tier))
    return
  end

end
