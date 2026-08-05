# RoF2 / Dragons of Norrath achievement coverage

This is the content-coverage record for the ToB achievement resource snapshot
imported into the RoF2 server. The audited server profile is launch through
Dragons of Norrath, level 70. It does not claim that later ToB definitions are
valid RoF2 content.

For runtime behavior and operations, see [RoF2 achievement
support](achievements.md). For component identities, criteria, rewards, and the
quest API, see [Achievement content authoring](achievement_authoring.md).

## Audit result

The audit started with 1,074 state-bearing components across 362 enabled
definitions that had no active criterion. Every component is now assigned to
one of these buckets:

| Bucket | Components | Treatment |
| --- | ---: | --- |
| Server-native | 510 | Deterministic criteria emitted by the importer |
| Quest-manual | 214 | Authoritative quest hook or durable quest-state sync |
| Presentation only | 20 | DoN headings; never treated as completion facts |
| Unavailable | 330 | Required content or hook is absent; 38 definitions are retained for investigation and 99 remaining reviewed definitions are disabled |

The generated SQL contains the exact
`achievement_id/component_type/component_id` list for every bucket. Shape
digests guard the reviewed raid and Hunter definitions: if an ID, description,
sequence, type, or required count changes, the importer omits the mapping and
reports the rejection instead of guessing.

## Native trigger matrix

| Event | Component identities | Criteria rows | Authoritative source |
| --- | ---: | ---: | --- |
| `NpcNameKill` | 426 | 436 | NPC death merit, exact normalized name hash, and audited zone ID |
| `OwnItem` | 21 | 22 | Inventory ownership refresh on acquisition and login |
| `TaskComplete` | 52 | 66 | Completed-task persistence and login reconciliation |
| `AchievementComplete` | 11 | 16 | Completed prerequisite achievement state |

Multiple criteria for one component are alternatives with the same component
policy. This is used for equivalent item IDs, alternative tasks or
prerequisites, and Fishlord phase NPCs. The Fishlord phase components accept
the audited hungry/stringy/toughened, dark/wicked/foul,
superior/prime/prismatic, and king/master/supreme anglerfish names in zone 216.
Do not also fan these kills out from `global/global_npc.lua`: the native merit
event already runs for each credited local raid/group member, while an
expedition-wide hook would repeat the same world mutation once per recipient.
Expedition fan-out remains appropriate for encounter-completion flags whose
members may be in other zones.

Notable durable mappings include:

- `90007/1/3014` (`A Stone Key`) -> item 12708.
- `90022/2/3036` (`Grimror's Bracer`) -> item 11173.
- `415705/1/415705` and the `520205`, `900100`, `900110`, and `500980300`
  item components -> their audited item IDs.
- `500980500/1/59805000-59805008` -> completed BiC tasks
  402008, 402010-402015, 402017, and 402018.
- The 42 actionable DoN tier task components -> their audited task IDs. The
  faction and tier-finished rows remain quest-owned.
- Classic/expansion meta rows and Defender of Norrath -> exact prerequisite
  achievement IDs.

## Quest-manual coverage

Quest hooks set a component to `1` only after its authoritative success
condition. They do not increment it. The common helper is
`quests/lua_modules/achievement_flags.lua`. Expedition-owned completion uses
`AdvanceAchievementProgress`, which reaches members in other zones through the
world mutation path. Dynamic-zone mutations also include online characters
still inside the matching zone instance, even if they were removed from the
expedition roster after participating.

The DoN import emits an enabled Manual/Set criterion for every audited
quest-manual component. The quest hook supplies the progress; the criterion
supplies the Required or Optional policy that lets the server evaluate the
parent achievement.

| Group | Achievement/component IDs | Owning quest code |
| --- | --- | --- |
| Raid and scripted events (16 components) | `521140/1/5000284-5000286`; `573640/1/5000225,5000358-5000362`; `722940/1/7000159,7000161`; `723040/1/7000168-7000169`; `723140/1/7000172`; `723240/1/7000163`; `723340/1/7000166` | `hohonora/zone_status.lua`; `potimeb/zone_status.lua`; `potimeb/phase_two_controller.lua`; `gukg/encounters/gukgraid.lua`; `guke/encounters/gukeraid.lua`; `rujd/encounters/rujdraid.lua`; `rujg/encounters/rujgraid.lua`; `takc/encounters/takcraid.lua`; `mirb/Durgin_Skell.lua`; `mmcc/encounters/mmccraid.lua` |
| Legacy progression achievements (44 components) | `500980400/1/59804000`, `/2/59804001-59804005`; `500980500/1/59805009-59805014`; `500980530/1/59805300`, `/2/59805301-59805314`; `500980550/1/59805500`, `/2/59805501-59805510`; `500990020/1/59900200-59900205` | `lua_modules/achievement_progression.lua`, `lua_modules/mpg_helper.lua`, source quest scripts, and login sync in `global/global_player.lua` |
| DoN progression (23 components) | `500980605/1/59806050`, `/2/59806051-59806052`; `500980610-500980700` faction and tier-complete endpoints (`59806100,59806107` through `59807000,59807006`) | `lua_modules/dragons_of_norrath.lua` and login sync in `global/global_player.lua` |
| City quest achievements (121 components) | IDs `12000-12143`, excluding absent `12080` and the 22 investigation-only definitions `12007,12009,12010,12013,12014,12016,12018,12019,12022,12030,12037,12038,12046,12047,12048,12062,12081,12091,12108,12112,12120,12129`; component type 1 and component ID equal the achievement ID | Individual city NPC quest scripts using `lua_modules/achievement_flags.lua` |
| Keys and MPG (10 components) | `90020/1/3033`, `90022/1/3035`, `90023/1/3037`, `90028/1/3042`, and `900110/2/900111-900116` | `poinnovation/player.lua`, `codecay/Tarkil_Adan.lua`, `codecay/player.lua`, `postorms/player.lua`, `lua_modules/achievement_flags.lua`, and `lua_modules/mpg_helper.lua` |

`90007/1/3014` is native item ownership. Its existing turn-in hook is a harmless
fallback, not the primary source.

The city quest achievement range has 143 existing IDs in `12000-12143`; `12080` is
absent. The quest review found 122 hooks, but the owner of `12016` is unspawned.
That leaves 121 currently playable definitions and 22 unresolved definitions
that remain enabled for investigation.

## Presentation and partial cases

The 20 DoN type-2 rows below are headings. They group errands, missions, and
raids in the client and are deliberately not criteria:

- `500980610`: `59806101`, `59806105`
- `500980620`: `59806201`, `59806205`
- `500980630`: `59806301`, `59806305`
- `500980640`: `59806401`, `59806404`
- `500980650`: `59806501`, `59806504`
- `500980660`: `59806601`, `59806605`
- `500980670`: `59806701`, `59806705`
- `500980680`: `59806801`, `59806805`
- `500980690`: `59806901`, `59806904`
- `500980700`: `59807001`, `59807004`

Three optional meta components refer to unavailable later content and are not
mapped: `100105/2/1001041` (Hunter of Freeport Sewers),
`800050/2/270588` (The Forgotten Halls Traveler), and
`500080/2/5000367` (Hunter of The Plane of War). They do not turn their labels
into facts or block the remaining required meta path.

Seven required LDoN raid components have no encounter implementation or
authoritative completion hook: `722940/1/7000160`, `723040/1/7000167`,
`723140/1/7000170-7000171`, `723240/1/7000162,7000164`, and
`723340/1/7000165`. Because each containing achievement requires every step,
the five affected Conqueror definitions remain enabled for investigation but
cannot currently complete.

## Definitions enabled for investigation

The exact DoN profile deliberately keeps these 38 reviewed definitions enabled.
Their unresolved components remain in the generated coverage report and do not
receive guessed criteria.

| IDs | Resource group | Known gap |
| --- | --- | --- |
| 12007, 12009, 12010, 12013, 12014, 12016, 12018, 12019, 12022, 12030, 12037, 12038, 12046, 12047, 12048, 12062, 12081, 12091, 12108, 12112, 12120, 12129 | 22 city quest achievements | No authoritative success hook; `12016` also has an unspawned owning NPC |
| 722940, 723040, 723140, 723240, 723340 | Five LDoN Conquerors | One or more required raid variants have no authoritative encounter implementation or completion hook |
| 153940, 352640, 415440, 415840, 520440 | Five Conquerors | One or more required raid targets are absent from audited PEQ spawns |
| 138480, 151880, 153980, 154880 | Four Hunters | Required revamped-zone populations are unavailable in this server profile |
| 139380, 250880 | Two Hunters | Erg Bluntbruiser or Blood-Thirsty Racnar is absent from audited PEQ spawns |

## Definitions forcibly disabled by the DoN profile

The importer always disables these 99 IDs for an exact DoN profile, even with
`--preserve-enable-state`. This behavior is intentionally scoped to
`--enable-through-expansion don`; importing DoDH, ToB, or another later profile
does not apply the DoN denylist.

| IDs | Resource name(s) | Specific reason |
| --- | --- | --- |
| 90003 | Efreeti's Key | No canonical durable quest completion source was found |
| 90033 | Passkey of the Twelve | No canonical durable quest completion source was found |
| 90034 | Keys of War | Requires unavailable Plane of War progression |
| 90035 | Riftbreakers' Ornaments | No canonical durable quest completion source was found |
| 103250, 103500 | 3250 / 3500 Alternate Advancement Points | Exceeds the DoN AA progression profile |
| 200071 | Conqueror of The Plane of War | Plane of War content is unavailable |
| 521300-521310 | The Plane of War Traveler; Mercenary; Partisan; Hunter; Savior; Beyond the Zekarian; Castle Tamrel; The Favor of Vallon; Castle Rulnavis; The Favor of Tallon; The Antechamber of Drunder | Plane of War content is unavailable |
| 521303 | Hunter of The Plane of War | Plane of War content is unavailable |
| 555500 | Fishing (500) | Target exceeds DoN tradeskill progression |
| 556310, 556320, 556330, 556340, 556350 | Poisonmaking (310-350) | Targets exceed DoN tradeskill progression |
| 557310, 557320, 557330, 557340, 557350 | Tinkering (310-350) | Targets exceed DoN tradeskill progression |
| 558310, 558320, 558330, 558340, 558350 | Research (310-350) | Targets exceed DoN tradeskill progression |
| 559310, 559320, 559330, 559340, 559350 | Alchemy (310-350) | Targets exceed DoN tradeskill progression |
| 560310, 560320, 560330, 560340, 560350 | Baking (310-350) | Targets exceed DoN tradeskill progression |
| 561310, 561320, 561330, 561340, 561350 | Tailoring (310-350) | Targets exceed DoN tradeskill progression |
| 563310, 563320, 563330, 563340, 563350 | Smithing (310-350) | Targets exceed DoN tradeskill progression |
| 564310, 564320, 564330, 564340, 564350 | Fletching (310-350) | Targets exceed DoN tradeskill progression |
| 565310, 565320, 565330, 565340, 565350 | Brewing (310-350) | Targets exceed DoN tradeskill progression |
| 568310, 568320, 568330, 568340, 568350 | Jewelcrafting (310-350) | Targets exceed DoN tradeskill progression |
| 569310, 569320, 569330, 569340, 569350 | Pottery (310-350) | Targets exceed DoN tradeskill progression |
| 590100, 590101, 590110, 590111 | A Wandering Artisan; The Artisan's Wares; The Artisan's Work; The Artisan's Prize | Post-DoN Artisan quest line |
| 600050 | Tutorials - Out of Gloomingdeep | Post-DoN Gloomingdeep tutorial |
| 899800 | The Forgotten Halls Traveler | Required instance is unavailable |
| 100038400 | Freeport Sewers Traveler | Revamped zone is unavailable |
| 100040800 | The Commonlands Traveler | Revamped zone is unavailable |
| 100049300, 100049400 | Wedding Chapel (Light / Dark) Traveler | Wedding instances are unavailable |
| 100049500 | Dragoncrypt Traveler | Special-event instance is unavailable |
| 100071400, 100071500, 100071600, 100071700, 100071800, 100071900 | Three/one-room stucco, wood, and stone house Travelers | Housing zones are unavailable |
| 100072300 | House (Hermit's Hideaway) Traveler | Housing zone is unavailable |
| 100073700, 100073800, 100075100 | Guild Hall (Palatial / Grand / Modest) Traveler | Guild-hall housing zones are unavailable |
| 100076600 | House (Evantil's Abode) Traveler | Housing zone is unavailable |
| 100077400 | House (Bixie Hive) Traveler | Housing zone is unavailable |
| 500980100 | Serpent Seeker's Charm of Lore (Various 1+) | Later progression content has no DoN source |
| 500980200 | Wanderlust Guild Loadstone (Various 15+) | Later progression content has no DoN source |

`521303` is included in the `521300-521310` range above; it is shown separately
to make its Hunter identity explicit. The unique disabled-ID count remains 99.

## Quest hook and reconciliation contract

- Call `SetAchievementProgress(achievement_id, component_type, component_id,
  1)` only after the source quest state is durably successful.
- Use the expedition/world mutation API for group, raid, DZ, or task members
  who may be in another zone.
- Keep durable task, item, and prerequisite facts in criteria so login and
  achievement reload reconcile existing characters.
- Persist script-only facts in the owning quest/task state, then sync them at
  login. Do not replay one-time rewards to reconstruct progress.
- Use `CompleteAchievement` only when the entire achievement, not merely one
  component, has been authoritatively granted.

## Deployment

Deploy the source and quest changes together. For a clean DoN profile, rerun
the importer and apply its SQL transaction:

```bash
python3 utils/scripts/import_achievement_resources.py \
  /path/to/Resources/Achievements \
  --enable-through-expansion don \
  --exact-enable-selection \
  --output achievements_don.sql
```

The re-import is required because it authors the new criteria, explicitly
enables the reviewed top-level and investigation definitions, and applies the
remaining DoN denylist. Existing character progress and rewards are not
deleted. After the SQL succeeds, run `#reload achievements global` or restart
all zone processes.
