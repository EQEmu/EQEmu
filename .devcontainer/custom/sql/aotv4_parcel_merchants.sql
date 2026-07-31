-- ==========================================================================================
-- AoTv4: a parcel merchant in EVERY start city, so a bazaar purchase can always be collected.
--
--   npc 2000220  #Parcel_Courier
--
-- Bazaar purchases are delivered by parcel (Bazaar:EnableParcelDelivery = true), and a parcel can
-- only be collected from a parcel merchant. 20 of them ship with PEQ but they miss NINE of the 25
-- start zones, so a character created in one of those has nowhere to pick anything up without
-- leaving the city.
--
-- ⚠️⚠️ THREE THINGS ARE REQUIRED, and Handle_OP_ShopRequest (client_packet.cpp:15422) enforces them:
--   1. class = 41 (Class::Merchant). It returns immediately for any other class, so the window does
--      not open AT ALL -- not "opens without the parcel tab".
--   2. is_parcel_merchant = 1. That is what turns the tab set from SellBuy into SellBuyParcel.
--   3. an RoF2 client. The parcel tab is RoF2 only; that is the client this server targets.
-- ⚠️ merchant_id = 0 is deliberate and IS handled -- the window opens with the parcel tab and no
-- wares. These are couriers, not shopkeepers; giving them a merchantlist would put the same shop in
-- nine cities for no reason.
--
-- ⚠️ npc_faction_id = 0 so the courier is never KOS to anyone. This matters now that race and deity
-- faction are back to stock (the "all races faction agnostic" patch was removed 2026-07-28): a
-- faction-carrying vendor could refuse the very players who most need it.
--
-- ⚠️ Only zones that DO NOT already have one get a courier, computed live -- so this is safe to
-- re-run, and it will not put a second merchant next to a stock one.
--
-- npc_types / spawn2 are NOT in shared memory -- a zone restart is all this needs.
-- Idempotent: re-running rebuilds every row it owns.
-- ==========================================================================================

DELETE FROM npc_types   WHERE id = 2000220;
DELETE FROM spawnentry  WHERE spawngroupID BETWEEN 2000300 AND 2000350;
DELETE FROM spawn2      WHERE spawngroupID BETWEEN 2000300 AND 2000350;
DELETE FROM spawngroup  WHERE id          BETWEEN 2000300 AND 2000350;

-- ---------------------------------------------------------------- the courier
-- Cloned from Ren_Pinemyer (1032, the Qeynos parcel merchant) via a temp table so all columns stay
-- byte-identical to a row that is known to work. NEVER hand-list the columns.
DROP TEMPORARY TABLE IF EXISTS aotv4_npc_tmpl;
CREATE TEMPORARY TABLE aotv4_npc_tmpl LIKE npc_types;
INSERT INTO aotv4_npc_tmpl SELECT * FROM npc_types WHERE id = 1032;

UPDATE aotv4_npc_tmpl SET
    id                 = 2000220,
    name               = '#Parcel_Courier',
    lastname           = 'Bazaar Delivery',
    class              = 41,        -- REQUIRED, see above
    is_parcel_merchant = 1,         -- REQUIRED, see above
    merchant_id        = 0,         -- courier only, no wares
    npc_faction_id     = 0,         -- never KOS, never gives faction hits
    level              = 70,
    hp                 = 100000,
    race               = 1,         -- human
    gender             = 2,         -- neuter, so one row suits every city
    bodytype           = 1,
    -- Unattackable and immobile: a city vendor must not be killable, and must not wander off the
    -- safe point it was placed on.
    special_abilities  = '1,1^7,1^13,1^14,1^19,1^20,1^24,1^35,1',
    npc_spells_id      = 0,
    loottable_id       = 0,
    aggroradius        = 0,
    assistradius       = 0,
    runspeed           = 0,
    see_invis          = 1,
    see_invis_undead   = 1;

INSERT INTO npc_types SELECT * FROM aotv4_npc_tmpl;
DROP TEMPORARY TABLE IF EXISTS aotv4_npc_tmpl;

-- ---------------------------------------------------------------- one per UNSERVED start city
-- Data driven from start_zones, minus every zone that already has a parcel merchant spawned. Placed
-- at the zone SAFE POINT, which is guaranteed standable ground.
DROP TEMPORARY TABLE IF EXISTS aotv4_parcel_zones;
CREATE TEMPORARY TABLE aotv4_parcel_zones (
  n          INT,
  short_name VARCHAR(32),
  x FLOAT, y FLOAT, z FLOAT
);
INSERT INTO aotv4_parcel_zones
  SELECT ROW_NUMBER() OVER (ORDER BY z.short_name), z.short_name, z.safe_x, z.safe_y, z.safe_z
  FROM (SELECT DISTINCT start_zone FROM start_zones) sz
  JOIN zone z ON z.zoneidnumber = sz.start_zone
  WHERE z.short_name IS NOT NULL AND z.short_name <> ''
    AND NOT EXISTS (
      SELECT 1 FROM spawn2 s
      JOIN spawnentry se ON se.spawngroupID = s.spawngroupID
      JOIN npc_types n   ON n.id = se.npcID
      WHERE s.zone = z.short_name AND n.is_parcel_merchant = 1);

INSERT INTO spawngroup (id, name, spawn_limit, dist, max_x, min_x, max_y, min_y, delay, mindelay)
  SELECT 2000299 + n, CONCAT('aotv4_parcel_courier_', short_name), 1, 0, 0, 0, 0, 0, 0, 0
  FROM aotv4_parcel_zones;

INSERT INTO spawnentry (spawngroupID, npcID, chance)
  SELECT 2000299 + n, 2000220, 100 FROM aotv4_parcel_zones;

-- ⚠️ The column is `_condition` (leading underscore); `condition` is reserved and there is no
-- `enabled` column in this schema.
-- ⚠️⚠️ min_expansion / max_expansion MUST BE -1 or a Classic-locked server content-filters the spawn
-- out at zone boot with no error anywhere -- the same trap as the PoK travel books (section 11).
INSERT INTO spawn2 (spawngroupID, zone, version, x, y, z, heading, respawntime, variance, pathgrid,
                    _condition, cond_value, min_expansion, max_expansion)
  SELECT 2000299 + n, short_name, 0, x, y, z, 0, 600, 0, 0, 0, 1, -1, -1
  FROM aotv4_parcel_zones;

DROP TEMPORARY TABLE IF EXISTS aotv4_parcel_zones;
