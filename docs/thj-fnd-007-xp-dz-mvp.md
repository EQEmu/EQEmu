# THJ-FND-007: Dynamic Zone XP Instance MVP — Verification

## Manual Test Steps

### Prerequisites
1. Run the SQL migration: `utils/sql/git/optional/2026_05_11_monomyth_xp_dz_tables.sql`
2. Enable the following rules in `rule_values`:
   ```
   Monomyth.DynamicZonesEnabled = true
   Monomyth.XPDZLockoutSeconds = 3600
   Monomyth.XPDZLifetimeSeconds = 86400
   Monomyth.MaxActiveXPDZAssignments = 1
   DynamicZone.EmptyShutdownEnabled = true
   ```
3. Insert at least one row into `monomyth_xp_dz_zones` (e.g., Crushbone):
   ```sql
   INSERT INTO monomyth_xp_dz_zones (zone_id, version, min_level, max_level)
   VALUES (58, 0, 5, 20);
   ```
4. Place an NPC with type ID matching `XPDZ_NPC_TYPE_ID` in your hub zone.
5. Copy `global/xp_dz_entrance.lua` to your server's quests/global/ directory.

---

### Test 1: Create XP DZ
1. Hail the XP DZ entrance NPC.
2. Say the name of an enabled zone (e.g., "crushbone").
3. **Expected**: NPC confirms creation. Player is moved to the XP DZ instance.
4. **Verify**: Player has an expedition window showing the XP DZ name and leader.
5. **Verify**: `/dz playerlist` shows the creator as the only member and leader.

### Test 2: Normal Mobs and Respawns
1. Inside the XP DZ instance, kill a normal mob.
2. **Expected**: Mob respawns on its normal timer.
3. **Expected**: XP is awarded normally.
4. **Verify**: The zone instance ID is non-zero (`#zone_status` or GM command).

### Test 3: Lockout Enforcement
1. Leave the XP DZ (via `/dz quit` or zoning out).
2. Attempt to create another XP DZ for the same zone.
3. **Expected**: NPC or system rejects with a lockout message.
4. **Verify**: `/dz timers` shows the "XP DZ Lockout" event under the expedition name.

### Test 4: Single Active XP DZ Per Character
1. Create an XP DZ for zone A.
2. While still a member, attempt to create an XP DZ for zone B.
3. **Expected**: Rejected with "You already have an active XP Dynamic Zone assignment."
4. **Verify**: The first XP DZ still exists and is functional.

### Test 5: Membership-Empty Destroys Instance
1. Create an XP DZ (solo).
2. Quit the XP DZ via `/dz quit`.
3. **Expected**: DZ enters early shutdown (EmptyShutdownEnabled triggers).
4. **Verify**: After the shutdown delay, the DZ is destroyed.
5. **Verify**: `/dz playerlist` returns empty.
6. **Verify**: Attempting to zone back into the instance fails.

### Test 6: Offline/Linkdead Does NOT Remove Membership
1. Create an XP DZ with 2+ members.
2. Have one member go linkdead (disconnect).
3. **Expected**: The linkdead member still appears in the DZ member list.
4. **Verify**: `/dz playerlist` shows the linkdead member with Offline status.
5. **Expected**: The DZ is NOT destroyed (other members still online).
6. Reconnect the linkdead member.
7. **Expected**: Member can zone back into the XP DZ instance.

### Test 7: Raid Target Suppression (if configured)
1. Insert a raid target NPC type ID into `monomyth_xp_dz_raid_suppress`.
2. Create an XP DZ for that zone.
3. **Expected**: The raid target NPC is not present in the instance.
4. **Verify**: Normal NPCs still spawn and function normally.

### Test 8: 24-Hour Lifetime
1. Create an XP DZ.
2. **Verify**: The expedition window shows ~1440 minutes remaining.
3. **Expected**: Warning messages at 15, 5, and 1 minute remaining.
4. **Expected**: Instance expires after the configured lifetime.

---

## GM Commands
- `#dz` — Create/inspect dynamic zones (existing GM command)
- `#dz kickplayers` — Remove all members from current DZ

## Lua API Reference

### Client Methods
```lua
client:CreateXPDZ(zone_name, version, dz_name, min_players, max_players)
-- Returns: Expedition object or nil
-- Example:
local exp = client:CreateXPDZ("crushbone", 0, "XP DZ - Crushbone", 1, 6)
```

### Rules
| Rule | Default | Description |
|------|---------|-------------|
| `Monomyth.DynamicZonesEnabled` | false | Master toggle for XP DZ system |
| `Monomyth.XPDZLockoutSeconds` | 3600 | Per-zone lockout after creating XP DZ |
| `Monomyth.XPDZLifetimeSeconds` | 86400 | Instance lifetime (24 hours) |
| `Monomyth.MaxActiveXPDZAssignments` | 1 | Max concurrent XP DZs per character |

### Database Tables
| Table | Purpose |
|-------|---------|
| `monomyth_xp_dz_zones` | Zone configuration (levels, player count, safe return) |
| `monomyth_xp_dz_raid_suppress` | Raid target NPC type IDs to suppress in XP DZ |
