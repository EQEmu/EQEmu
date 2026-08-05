# RoF2 achievement support

This implementation is based on the RoF2 client. The newer ToB resource
files are optional presentation data; explicit
progression selection can additionally author criteria only for narrow,
independently validated level, level-locked progression, class skill-cap,
item-ownership, travel, dependency, tradeskill, spent-AA, and zone-scoped
named-kill shapes. Native Slayer race-kill policy is a separate explicit
opt-in. Nothing that is unique to ToB's packet protocol is sent to RoF2.

See [Achievement content authoring](achievement_authoring.md) for the database
relationships, criterion examples, rewards, and quest API recipes.
See [RoF2 / Dragons of Norrath achievement coverage](achievement_coverage.md)
for the audited native, quest-owned, presentation-only, and unavailable IDs.

## Runtime flow

At zone startup, `AchievementManager` loads enabled definitions, categories,
components, evaluation criteria, rewards, and achievement-backed spell
restrictions. It keeps definitions in deterministic achievement-ID order and
builds the compressed definition packet once. Invalid or conflicting server
policy fails zone startup instead of leaving a partially active system.

At character connect, `ClientAchievementState`:

1. creates one state entry for every loaded definition;
2. loads completion and component progress from the character database;
3. resets version-mismatched definitions when their reset policy is enabled;
4. retries rewards whose delivery API explicitly failed;
5. reconciles durable facts such as level, zone, completed tasks, skills,
   inventory, spent AA points, and prerequisite achievements;
6. evaluates completion, visibility, and lock policy; and
7. sends definitions, the dense primary state, and progress in that exact
   order.

Runtime events update only criteria indexed for that event. A component update
is persisted before it is exposed to the client or used to complete an
achievement. A failed write is rolled back in memory.

Durable source facts are ordered before achievement persistence. Task and
skill hooks run after their source write succeeds. Level facts run only after
character data commits; spent AA facts run only after successful AA-state saves.
Inventory writes instead mark ownership dirty
after every persistence attempt, including failed or ambiguous attempts.
The next client process boundary reads the durable final rows, so a multi-step
move is never evaluated while an item temporarily exists in both source and
destination.

All other achievement mutations, including Lua or Perl progress/completion
calls made by equip and unequip events, queue behind a dirty ownership fact or
an active inventory transaction. They replay only after authoritative
ownership succeeds; a failed read retains both the dirty flag and the queue for
retry. The disconnect path performs this flush before destroying client state,
but deliberately leaves reward delivery to the recoverable login ledger.

RoF2 item moves, multi-item moves, pet moves, stacked reward placement,
disarm, bandolier swaps, and GM snapshot restoration use checked atomic
persistence where a partial relocation could otherwise create false ownership.
No statement reconnects or retries after a strict transaction starts. A failed
statement rolls the transaction back. A lost COMMIT response remains
ambiguous, so runtime inventory is disconnected and reloaded rather than being
used for another achievement-sensitive mutation.

Achievement state is available to quest events during connect, but RoF2
incremental packets and queued rewards remain suppressed until the initial
definition, primary-state, and progress packets have been sent.

Completions discovered while reconciling durable facts at login are persisted
before those packets and queued for presentation. Once the initial three-packet
model is installed, newly completed achievements are announced in completion
order at a bounded rate. Achievements already present in
`character_achievements` are loaded without being announced again.

Each announcement uses RoF2's native earned-achievement packet. The client
builds the localized clickable achievement link from the supplied serialized
state metadata and its locally loaded definition, and plays its
`Achievement.wav` asset. A separate narrow inter-zone guild message carries
the same metadata so other RoF2 guild members receive the native
guild-achievement line and link. The earning client is excluded from that
guild delivery because it already receives the more verbose local completion
line. Guild membership is captured when the completion occurs, so a throttled
announcement cannot leak into a guild joined afterward. Notification delivery
is best effort after durable completion; a lost world connection never rolls
back or repeats achievement persistence.

## RoF2 wire protocol

The supported RoF2 opcodes are:

| Purpose and direction | Emulated opcode | RoF2 opcode |
| --- | --- | --- |
| Definitions, server to client | `OP_AchievementDefinitions` | `0xDAB0` |
| Full comparison/summary snapshot, server to client | `OP_AchievementState` | `0x059D` |
| Primary state (dense initialization and sparse updates), server to client | `OP_AchievementUpdate` | `0x618F` |
| Earned notification, server to client | `OP_AchievementEarned` | `0x17F8` |
| Progress counts, server to client | `OP_AchievementProgress` | `0x2B42` |
| Window request, client to server | `OP_AchievementRequest` | `0x47CB` |
| Open the normal achievement window, server to client | `OP_AchievementWindow` | `0x171E` |
| Compare request, client to server | `OP_AchievementCompareRequest` | `0x1665` |
| Local-link inspection request, client to server | `OP_AchievementLinkRequest` | `0x675D` |
| Authoritative link/inspection reply, server to client | `OP_AchievementComparisonReply` | `0x28E0` |
| Claimable/pending Select Reward manager, bidirectional | `OP_AchievementReward` | `0x6411` |
| Read-only Select Reward preview manager, bidirectional | `OP_RewardSelection` | `0x6471` |

RoF2 does not use the newer `Resources/Achievements` definition files. The
zone sends its read-only definitions, primary character state, and progress
during zone entry and resends that same state when the achievement window
requests it.

The definitions packet is a little-endian uncompressed-size prefix followed by
a zlib stream. Its inflated body is:

```text
u32 category_count
Category[category_count]
u32 achievement_count
Achievement[achievement_count]
```

The content database uses `parent_id = 0` for top-level categories. RoF2 reads
that wire field as signed and only enumerates roots whose parent is `-1`, so
serialization translates database root `0` to wire value `0xFFFFFFFF`. Nested
category parent IDs are sent unchanged.

RoF2 renders every transmitted root and child even when it has no achievements.
The zone therefore sends only categories with associations to enabled
definitions plus the complete ancestor chain required to reach their roots.
Child lists are rebuilt from that retained hierarchy. An enabled definition
without a valid category association, or an active hierarchy with a missing
parent, reserved ID zero, or cycle, prevents achievement content from loading.

An achievement definition contains four component vectors in type order
0, 1, 2, 3. The byte immediately before those vectors is RoF2's `persistent`
flag and is written as `1`; the following dword is `definition_version`. The
final two fields are RoF2 points and reward-display values.

The comparison/summary snapshot has no count or achievement IDs. It has exactly
one state in the same order as the definition vector:

```text
i16 status                 // 0 completed, 1 open, 2 locked, 3 hidden
u16 type_1_bits[ceil(n/16)]
u16 type_2_bits[ceil(n/16)]
u16 type_0_bits[ceil(n/16)]
if completed:
    u32 completion_time
```

Bits are least-significant-bit first. RoF2 deliberately ignores component type
3 in state, comparison counts, and progress processing. The server therefore
keeps type 3 only as definition/presentation text: it cannot have an enabled
criterion, receive scripted progress, affect lock or completion state, persist
progress, emit a progress record, or grant a reward through completion.

Primary state uses a serial, a dense/sparse flag, and a count. The zone sends
one dense record for every definition during initialization; dense record `i`
targets definition `i`. Later sparse records carry their definition index.
RoF2's main achievement list and summary are built from this primary model.
The separate `0x059D` snapshot is comparison-only: its handler enables Compare
mode and explicitly shows the achievement window. It must therefore be sent
only in response to a validated Compare request.

Each progress record is:

```text
u32 achievement_id
u32 component_id
u32 requirement_id
u32 requirement_type
u32 current_count
```

The stable aggregate key used by this server is component sequence plus
component type. RoF2 stores distinct requirement records but sums every record
matching an achievement and component. Changing that key between initial and
live updates would therefore double the displayed count.

The earned notification follows its persisted state update and contains:

```text
u32 achiever_spawn_id
u32 achievement_id
u32 sound_id             // 3695, RoF2 Achievement.wav
char achievement_link_data[] // NUL-terminated
```

The final string is not visible notification text. It is RoF2's native
caret-delimited link state:

```text
player_name^achievement_id^status^
type_1_signed_bit_words^
type_2_signed_bit_words^
type_0_signed_bit_words^
[completion_timestamp^ when status is completed]
type_1_counts^type_2_counts^type_0_counts^
```

Every scalar ends in `^`. Each bit word packs 16 components least-significant
bit first, is cast to signed `i16`, and is printed in decimal. The client looks
up the achievement ID and supplies the visible definition name when it wraps
this metadata as:

```text
\x12 + "3" + achievement_link_data + "'" + achievement_name + \x12
```

The server must send only the caret-delimited metadata in the earned and guild
packets; adding that raw chat-link envelope server-side would wrap it twice.
When a player clicks the resulting link, RoF2 first resolves
`achievement_id` in its loaded definition catalog. If the linked player is
still local to the zone, the client sends `OP_AchievementLinkRequest`
(`0x675D`). That variable-length request carries the NUL-terminated player
name, achievement ID, serialized state/count data in component order 1, 2, 0,
and a trailing flag. The server validates the exact size against the loaded
definition, discards the client-supplied state as untrusted presentation data,
looks up the named local player, and replies with that player's authoritative
state on `OP_AchievementComparisonReply` (`0x28E0`). For a nonlocal linked
player, the client opens the state embedded in the original link without this
round trip.

Other guild members receive `OP_GuildUpdate` action `3`, with the achiever name
in its fixed 64-byte field, the achievement ID at offset `0x50`, and the same
NUL-terminated link metadata at offset `0x54`. Its packet size is therefore
`84 + metadata length + 1`, not a fixed 85 bytes. The server sends this form
only to other RoF2 clients in the matching guild.

When the hidden achievement window is opened, RoF2 sends an empty (0-byte) or
one-byte `0x30` payload on `OP_AchievementRequest`. The server responds by
idempotently resending definitions, the dense primary state, and progress in
that order, then sends the zero-payload `OP_AchievementWindow` (`0x171E`) to
show the normal view. Requests are bounded to one response per client per
second.

The Compare button sends exactly eight bytes on
`OP_AchievementCompareRequest` (`0x1665`):

```text
u32 target_spawn_id
u32 requester_spawn_id
```

The server requires the requester ID to match the sending client and the target
to be that client's current nearby player target. It returns the target's full
definition-ordered snapshot on `OP_AchievementState` (`0x059D`). This is
the only path that sends `0x059D`; RoF2 uses it to enter Compare mode and open
the window. It is separate from the per-achievement `0x28E0` link/inspection
reply and from the reward action family.

RoF2 has two Select Reward managers using the same payload family.
`OP_AchievementReward` (`0x6411`) is claim-capable: it displays **Choose** and
permits action `3`. `OP_RewardSelection` (`0x6471`) is read-only: it displays
the full reward choices, details, Inspect, and Preview controls but hides
**Choose** and refuses to dispatch a claim.

| Action | Direction | Purpose |
| --- | --- | --- |
| `0` | Server to client | Populate the native reward window; the opcode selects claimable or read-only behavior |
| `1` | Bidirectional | Inspect an item entry in the reward window; this is a distinct item-inspection exchange and must not be treated as Compare |
| `3` | Bidirectional on `0x6411` only | Claim a selected option; the server echoes the identities and sets the reply success byte only after validated delivery |
| `5` | Client to server on `0x6411` | View Reward request carrying the zero-based definition index |
| `6` | Client to server on either manager | Restore all eligible pending rewards on `0x6411` |
| `7` | Server to client | Replace the manager with zero or more tabbed reward displays |

Action `5` is eight bytes: the action followed by the zero-based definition
index used by the definition and dense-state packets. Although the request
arrives on `0x6411`, the response is conditional. An earned, outstanding
selection is returned on `0x6411`, exposing **Choose**; an unearned,
already-claimed, automatic-only, or otherwise non-pending reward is returned
on `0x6471`, which removes **Choose** while preserving the complete preview.
Action `7` carries a count and a complete action-`0` reward record for each
tab. The server uses it on the claimable lane so outstanding achievement and
task rewards can coexist in the stock tabbed window. An empty action `7`
clears either manager when no display remains.

Action `3` carries the pending reward ID, reward-set ID, and selected option
ID; its reply has the same identities plus the success byte. The action-`1`
request is exactly five `u32` values: action, reward-set ID, option ID,
reward-entry ID, and item ID. Every displayed reward entry uses its nonzero
canonical `achievement_rewards.reward_id` as the 32-bit reward-entry ID. The
server verifies all three identities and requires the item to belong to that
exact loaded option. Its reply is action `1`, the item ID, then the ordinary
RoF2 serialized item body. A RoF2-only outbound encoder reuses the existing
item serializer while keeping actions `0`, `3`, and `7` as raw reward
payloads; sending a normal `OP_ItemLinkResponse` would not populate the reward
window's item cache.

Database option IDs are scoped to their reward set, but RoF2 indexes option
detail records by option ID across the entire displayed manager. Before an
action-`7` replacement, the zone assigns every displayed option a unique wire
ID. Inspect and claim requests are translated back to the canonical
`achievement_reward_options.option_id` before provider validation and ledger
updates. This permits different achievement and task sets to use ordinary
local option IDs such as `1`, `100`, and `110` without cross-populating tabs.

The five optional client resource files do not contain reward-set, option, or
grant contents. Their imported reward-display field is therefore not treated
as proof that a usable reward exists. Runtime clears that display flag and
enables View Reward only for definitions with loaded server-authored reward
rows or a valid selectable reward set.

## Content and state tables

Presentation and server policy are intentionally separate:

- `achievement_categories`
- `achievements`
- `achievement_category_associations`
- `achievement_components`
- `achievement_component_counts`
- `achievement_criteria`
- `achievement_rewards`
- `achievement_reward_sets`
- `achievement_reward_options`
- `achievement_reward_option_entries`
- `achievement_cast_restrictions`
- `character_achievement_pending_mutations`
- `character_achievements`
- `character_achievement_progress`
- `character_achievement_rewards`
- `character_achievement_reward_selections`

Completion, progress, reward claims, and cast-restriction checks are
character-scoped. The definition catalog and its zero-based indices remain
global and identical for every client. When positive Own Item and Skill Cap
criteria that carry a valid class ID resolve to one EQ class, a character of a
different class receives state `Hidden` (`3`) for that definition. Blocker,
optional, and display-only criteria do not establish class applicability, and
contradictory required class criteria make the content snapshot fail closed.
RoF2 then omits hidden definitions from the window while the global index is
preserved for dense state, links, View Reward, and reloads. Thus the UI shows
only class-applicable Epics and skill families without making their completion
account-wide.

Shared-bank rows are deliberately an account-accessible input to Own Item, but
any resulting completion and reward still belong to the active character. A
future account-wide mode needs an explicit per-definition scope and separate
reward policy; it must not be implemented by simply OR-ing character completion
rows.

Scripted group, raid, dynamic-zone, and shared-task updates are expanded by
world into `character_achievement_pending_mutations`. Each target character
applies that durable row in its current zone or on a later login. Progress
requests are monotonic floors and completion requests are idempotent, so a row
left behind after an interrupted delete can be replayed safely. Definition
version mismatches and invalid components are retained as blocked rows with a
diagnostic instead of being applied to different content. A character-scoped
database lock serializes zone-handoff consumers. Per-row claim tokens and a
60-second processing lease recover work left by a stopped zone process, while
a 64-row client-tick budget prevents an accumulated raid backlog from stalling
login.

Component identity is `(achievement_id, component_type, component_id)`.
Sequence is display order, not identity. This matters because the supplied ToB
data contains repeated `(achievement_id, component_type, sequence)` tuples.
Definition components may use the four RoF2 wire types `0` through `3`, but
server-evaluated criteria may use only types `0` through `2`. Type `3` is
presentation-only because RoF2 has no state or count channel for it.

Imported component counts are presentation defaults only. Every enabled
server criterion must provide an explicit, nonzero `required_count`; the
manager rejects zero and uses the server value for both evaluation and the
RoF2 definition sent for that evaluated component. ToB data therefore cannot
silently become completion policy.

`definition_version` is sent to RoF2 and also controls server persistence. When
it changes and `reset_on_version_change` is enabled, completion, progress, and
the reward ledger are reset together. Deleting completion last makes an
interrupted reset discoverable on the next zone entry.

Enabled runtime content is fail-closed. Invalid component/criterion/reward
types, missing evaluated components, conflicting or duplicate criterion
identities, duplicate definition/component/reward identities, and duplicate
cast restrictions prevent the zone from starting. The schema migrations also
verify the critical content keys and all character-state idempotency keys,
adding a missing unique key when legacy rows permit it. Conflicting duplicate
legacy rows make the migration fail for operator repair rather than allowing
doubled progress or reward claims.

## Evaluation criteria

The client resources describe what to display, not what game action satisfies
it. Automatic progression therefore requires explicit `achievement_criteria`
rows. The importer authors policy only for the narrow resource shapes that can
be mapped independently to a stable server fact: level milestones, leaf
travelers, exact child-achievement dependencies, zone-scoped Hunter names,
class skill caps at a milestone level, exact item-name ownership, and
explicitly opted-in direct tradeskill and AA-spent milestones. Everything else
remains server-authored.

Event values are:

| Value | Event |
| --- | --- |
| 0 | Manual |
| 1 | Level |
| 2 | NPC type kill |
| 3 | NPC race kill |
| 4 | Task complete |
| 5 | Zone enter |
| 6 | Loot item |
| 7 | Own item |
| 8 | Tradeskill success |
| 9 | Skill value |
| 10 | Spent/assigned alternate-advancement points |
| 11 | Achievement complete |
| 12 | Canonical NPC-name kill, scoped by zone |
| 13 | DB-backed class skill cap at a milestone level |

`target_id` is an exact-match filter. Zero is normally a wildcard, except for
two deliberate cases: task completion must name a specific task so it can be
replayed from durable task history, and Skill Value criteria use `4294967295`
(`UINT32_MAX`) as their wildcard because skill ID zero is the valid 1H Blunt
skill. Skill Cap criteria always name an exact valid skill; the same sentinel is
used only by the runtime to request a full Skill Cap reconciliation.

`target_id2` is normally zero. For NPC Name Kill it is the exact zone ID, with
zero as the explicit zone wildcard. For Own Item it may be an EQ class ID;
zero means any class. Skill Cap requires an EQ class ID. A positive
`target_value` is normally a minimum event value. For Skill Cap it is instead
the milestone level used to query `skill_caps`; the resulting DB cap is the
value compared with the character's persisted raw skill.

NPC-name kill criteria use the FNV-1a 32-bit hash of a deliberately narrow
canonical name in `target_id` and the exact zone ID in `target_id2`. Canonical
names retain ASCII letters only, lowercase them, collapse underscores and
spaces to one space, and remove digits, punctuation, and non-ASCII characters.
A zero hash is invalid. Hash collisions between different canonical names are
audited within each zone; repeated names in different zones remain distinct.

Runtime trigger and replay coverage is:

| Event | Live source | Login/zone replay | Importer policy |
| --- | --- | --- | --- |
| Manual | Lua/Perl progress or completion calls | Persisted achievement state | Never inferred |
| Level | Successful character save after a level change | Current level | Validated Level and level-locked Progression milestones |
| NPC type/race kill | Credited NPC death for raid, group, or solo recipients | None; no durable named-kill history | Reviewed server IDs only |
| NPC name kill | Same credited death, canonical original name plus base zone | None | Validated Hunter leaves; reviewed raid/boss criteria |
| Task complete | After completed-task state is durably saved | Recorded completed tasks, including shared tasks | Reviewed task IDs only |
| Zone enter | Destination zone achievement load/reconciliation | Current zone only | Validated Traveler leaves |
| Loot item | Successful NPC-corpse transfer | None | Reviewed item IDs only |
| Own item | Durable inventory/shared-bank/keyring mutation | Authoritative inventory, shared bank, and keyring | Exact-name Key and class-gated Epic items; reviewed item IDs |
| Tradeskill success | Successful manual or auto-combine | None | Reviewed recipe IDs only |
| Skill value | Successful raw-skill persistence | All persisted raw skills | Opt-in direct tradeskill milestones |
| Skill cap | Successful raw-skill persistence or level change | All persisted raw skills against DB caps | Validated class proficiency families |
| AA points spent | Successful AA-state save | Purchased ranks plus durable expended-AA history | Opt-in validated spent-AA milestones |
| Achievement complete | Persisted completion propagation | Completed prerequisite achievements | Unique exact-name dependencies |

Named raid bosses can use the same NPC-name path when a server author supplies
the reviewed name hash and zone. The importer does not infer Raid or
`Conqueror` components from display prose: many describe phases, trials, or
scripted events rather than killable NPC names. Raid trials, scripted encounter
wins, faction/access flags, and quests whose client text does not identify a
server task remain explicit task-ID criteria or quest-script calls. Historical
kills, visits other than the current zone, loot, recipes, and combines are not
fabricated at login.

Normal skill gains and quest `SetSkill` calls persist through `Client::SetSkill`
before emitting Skill Value. The legacy Lua/Perl `IncreaseSkill` helper mutates
only the in-memory profile and does not persist or emit an immediate
achievement event; achievement-bearing scripts must use `SetSkill`. Login
reconciliation always evaluates the persisted raw skill, never item-modified
skill. Skill Cap criteria additionally query `skill_caps` for the criterion's
class, skill, and milestone level. They do not embed a cap copied from the ToB
client.

Progress modes are:

| Value | Mode | Effect |
| --- | --- | --- |
| 0 | Increment | Add the event value |
| 1 | Highest | Keep the highest observed value |
| 2 | Set | Replace with the event value |
| 3 | Boolean | Satisfy the component on a matching event |

Component behaviors are:

| Value | Behavior |
| --- | --- |
| 0 | Required for completion |
| 1 | Optional |
| 2 | Unlock gate; locked until satisfied |
| 3 | Visibility gate; state is Hidden until satisfied |
| 4 | Display only |
| 5 | Blocker; locks when satisfied |

Every criterion that targets the same component must use the same behavior and
effective required count, event type, and progress mode. The manager rejects
conflicting alternate policy instead of depending on row order. Increment mode
is also rejected for reconciled absolute facts (level, owned-item count, raw
skill, DB-backed skill cap, and spent AA), for specifically targeted one-time
tasks, and for all
achievement-completion dependency criteria; use Highest, Set, or Boolean for
those. Boolean criteria over absolute facts
must also specify a positive `target_value`, so an owned count or skill value
of zero cannot satisfy them during login reconciliation. Set and Boolean
absolute facts are actively cleared when they fall below the threshold.
Alternate absolute criteria for one component are aggregated by their highest
valid value, so database or inventory iteration order cannot change the
result. A definition with no required criteria remains open and cannot
accidentally auto-complete.

Owned-item reconciliation reads the persisted `inventory`, `sharedbank`, and
`keyring` rows rather than trusting possibly newer in-memory mutations. It
covers equipment, general inventory, bank, shared bank, persisted bag contents,
socketed augments, every durable buffered cursor item rather than only the
visible front item, and the retained item identity for each acquired key.
A keyring entry satisfies one copy without double-counting a physical copy
still held. Non-ringable keys remain covered while physically present in any
included item location; keyring is an additional durable source, not an
exclusive path. Reconciliation applies the same expansion-slot, parent container,
bag-size, item-definition, augment-definition, and legacy charge normalization
rules as inventory loading, so stale database rows are not achievement credit.
A wildcard owned-item criterion uses the largest quantity held for any single
item ID. Cursor overflow is reported as a persistence failure; an item reward
also checks cursor capacity before mutation. A nonzero Own Item `target_id2`
restricts that criterion to the matching EQ class; this is used for Epic
definitions while Key definitions remain class-neutral.

Before any non-ownership event evaluates a definition whose completion,
visibility, unlock, or blocker policy depends on Own Item, the server performs
one fresh ownership pass. This is also the final completion gate. It prevents
stale-high rewards and stale-low lock state when another concurrently online
character on the same account changes the account-wide shared bank, without
requiring ToB behavior or cross-zone achievement messages. If any component of
that authoritative pass cannot be persisted, no definition is evaluated from
the partial result.

Clients with configured Own Item criteria also perform a periodic authoritative
pass. This makes an account-wide shared-bank change visible to an otherwise idle
character in another zone, including Own-only achievements, without adding a
new cross-zone message dependency. The interval is configurable and the pass is
coalesced with any local inventory reconciliation already pending. The first
deadline is deterministically jittered per character, linkdead clients do not
poll, and polling stops after every Own-based definition is complete. A failed
background pass waits for the next interval; it does not enter the foreground
per-process retry path used to protect an active inventory mutation.

Example server-authored criteria:

```sql
-- Kill 100 of NPC type 12345.
INSERT INTO achievement_criteria
    (achievement_id, component_type, component_sequence, component_id,
     event_type, progress_mode, behavior, target_id, required_count)
VALUES
    (900001, 0, 0, 700001, 2, 0, 0, 12345, 100);

-- Reach level 60.
INSERT INTO achievement_criteria
    (achievement_id, component_type, component_sequence, component_id,
     event_type, progress_mode, behavior, target_value, required_count)
VALUES
    (900002, 0, 0, 700002, 1, 3, 0, 60, 1);

-- Acquire or retain reviewed item ID 12345. A matching keyring entry also
-- satisfies one copy.
INSERT INTO achievement_criteria
    (achievement_id, component_type, component_sequence, component_id,
     event_type, progress_mode, behavior, target_id, target_value,
     required_count)
VALUES
    (900003, 1, 0, 700003, 7, 3, 0, 12345, 1, 1);
```

The achievement, component type, and component ID in these rows must match the
associated presentation component. Display sequence is mutable presentation
ordering and is not part of runtime component identity.

## Rewards

Reward types are item `0`, experience `1`, AA points `2`, copper `3`,
alternate currency `4`, and title `5`. Each `achievement_rewards` row remains
the canonical grant identity and is guarded by
`character_achievement_rewards` before delivery.

For experience rows, `reward_data_id = 0` uses the established experience
path, including the character's AA allocation and configured modifiers.
`reward_data_id = 1` adds the raw amount to normal experience while preserving
AA experience, matching rewards explicitly described as **No AA Exp**. Title
rows use a nonzero `titles.title_set`, not a row's `titles.id`; every eligible
prefix or suffix sharing that set is unlocked together.

An enabled reward row that is not present in
`achievement_reward_option_entries` is an automatic completion grant. Mapping
that row through `achievement_reward_option_entries` removes it from automatic
delivery and places it in the server-authored selectable model:

- `achievement_reward_sets` assigns at most one enabled set and window title to
  an achievement.
- `achievement_reward_options` orders the set's options. An option with
  `common_to_all = 1` is granted with every selection; every valid set must also
  contain at least one non-common selectable option.
- `achievement_reward_option_entries` maps each canonical
  `achievement_rewards.reward_id` to one option. Disabled or incomplete
  mappings fail closed and never fall back to an automatic grant.

For example, this creates one common AA-point grant plus a choice between two
items for achievement `900001`:

```sql
INSERT INTO achievement_reward_sets
    (reward_set_id, achievement_id, title, enabled)
VALUES
    (700001, 900001, 'Choose Your Reward', 1);

INSERT INTO achievement_reward_options
    (reward_set_id, option_id, sequence, label, common_to_all, flags, enabled)
VALUES
    (700001, 1, 0, 'Included with every choice', 1, 0, 1),
    (700001, 2, 1, 'First item', 0, 0, 1),
    (700001, 3, 2, 'Second item', 0, 0, 1);

INSERT INTO achievement_rewards
    (reward_id, achievement_id, sequence, reward_type, reward_data_id,
     amount, description, enabled)
VALUES
    (800001, 900001, 0, 2, 0, 1, '1 Alternate Advancement Point', 1),
    (800002, 900001, 1, 0, 12345, 1, 'First item', 1),
    (800003, 900001, 2, 0, 23456, 1, 'Second item', 1);

INSERT INTO achievement_reward_option_entries
    (reward_set_id, option_id, reward_id)
VALUES
    (700001, 1, 800001),
    (700001, 2, 800002),
    (700001, 3, 800003);
```

View Reward serializes non-pending rows as action `0` on the read-only
`OP_RewardSelection` manager, so the stock RoF2 window displays the common
bundle and choices without offering **Choose**. If that exact selectable
reward is earned and outstanding, the same request reopens it on the
claim-capable manager instead. The five imported client resource files do not
supply any of these contents: reward sets, labels, item/currency identities,
quantities, and grant policy must all be authored in the server database.

Outstanding selectable rewards are not serialized as a single active claim.
The zone rebuilds the complete claimable collection from the achievement and
task ledgers and sends it as an action-`7` replacement. Each tab retains its
own pending ID, reward-set ID, source identity, and claim validation. Earning a
second reward while the first window is open adds another tab; claiming one
removes only that occurrence and leaves the others available.

A completed achievement with a valid selectable set and no whole-selection
ledger row is an outstanding pending reward. The server opens it on the
claimable `OP_AchievementReward` manager after initial achievement state,
immediately after completion when no other pending selection owns that lane,
or in response to action `6`. A status-`0` row with no selected option is also
pending. Status `2` is restored with the original option locked; status `0`
with an option is first recovered as status `2` when the per-entry ledger proves
that replay is safe, or quarantined as status `3` when any entry is still
in-flight. Status `1` and status `3` are not presented as claimable.

For a selectable set, the action-`3` request must identify the pending
achievement, loaded reward set, and one enabled non-common option. The server
requires that achievement to be completed, grants every common option plus the
selected option, and replies success only after the selection and every
per-entry grant have been durably finalized.

`character_achievement_reward_selections` records the selected option and
whole-claim state: status `0` with option `0` is pending, status `0` with a
nonzero option is in progress or interrupted, `1` is fully granted, `2` is an
explicit retryable failure, and `3` is ambiguous delivery. Only status `2` may
be relocked and retried automatically. Interrupted status-`0` rows are promoted
to `2` only when no per-entry row remains in-flight; otherwise they become `3`.
Individual entries remain independently idempotent in
`character_achievement_rewards`, so resuming a partially successful selectable
claim does not duplicate entries already finalized.

- For item rewards, `amount` is the charge/stack count passed to one item
  summon. Use multiple reward rows when multiple non-stackable copies are
  intended.
- In `character_achievement_rewards`, status `1` means delivered and prevents
  duplicate grants.
- Entry status `2` means the delivery API explicitly failed and is retryable.
- Entry status `0` is an in-flight/ambiguous claim. It is not automatically
  retried after a process interruption because doing so could duplicate an
  item or currency grant. An operator can inspect it and change it to `2` when
  retry is known to be safe.

Completion is reconciled with the ledger on login, so a crash after persisting
completion but before creating an automatic reward claim does not lose that
reward. Automatic delivery is queued until the next client-processing tick.
This lets the game event that completed the achievement finish mutating level,
experience, inventory, or AA state before any chained reward invokes those
systems again. Selectable rows wait for a validated client selection instead.
Experience and AA delivery is synchronously saved before the claim is finalized;
item, currency, and title delivery likewise checks the available persistence
result. A failed post-mutation persistence result remains status `0`, since
retrying that uncertain delivery would risk duplication.

## Quest scripting

Lua and Perl clients expose the same focused methods:

- `HasCompletedAchievement(achievement_id)`
- `GetAchievementStatus(achievement_id)`
- `GetAchievementProgress(achievement_id, component_type, component_id)`
- `SetAchievementProgress(achievement_id, component_type, component_id, value)`
- `SetAchievementProgress(achievement_id, component_type, component_id, value, additive)`
- `CompleteAchievement(achievement_id)`

Group, Raid, and Expedition objects expose
`AdvanceAchievementProgress(achievement_id, component_type, component_id,
value)` and `CompleteAchievement(achievement_id)`. Client also exposes
`AdvanceSharedTaskAchievementProgress(...)` and
`CompleteSharedTaskAchievement(...)` for the client's active shared-task
instance. These scope operations are routed through world and stored per
character, so recipients can be in another zone or offline. Advance is a
replay-safe "set at least" operation, not an additive delta. The Boolean return
confirms handoff to the connected world process; it does not wait for member
rows to be committed or applied. World retries transient pre-commit failures
in a bounded memory queue and preserves a resolved membership snapshot.

`GetAchievementStatus` returns `0` for completed, `1` for open, `2` for locked,
`3` for hidden, or `-1` when the state is unavailable. `GetAchievementProgress`
returns the current component count or `-1` when the achievement, component
type, or component ID is invalid. These `Client` methods require a live player
whose achievement state has loaded; they do not address an arbitrary offline
character ID.

Component type must be in the state-bearing RoF2 range `0` through `2`; type
`3` is definition/presentation-only, and wider script numeric values are
validated before conversion. The non-additive `SetAchievementProgress`
overload sets an exact clamped count and can lower existing progress; the
additive overload adds to the current count and clamps at the required count.
Both persist before sending packets or evaluating completion.
`CompleteAchievement` deliberately bypasses criteria, persists completion,
queues normal notifications and rewards, and fires achievement-dependency
events. Automatic criteria should be preferred for engine-observable facts,
while these methods cover bespoke quest conditions. Calls made inside an
inventory transaction, or while an ownership reconciliation is pending, are
accepted into the client-local deferred queue and become durable only after
that ownership boundary succeeds.

See [Achievement content authoring](achievement_authoring.md#quest-scripting)
for complete Lua and Perl recipes for player, group, raid, expedition, personal
task, and shared-task state.

## Spell restrictions

`achievement_cast_restrictions` maps an existing spell restriction number to
one or more achievement requirements. All rows for one restriction number must
pass. Unmapped values continue through the existing spell-restriction switch.

The verified RoF2 mappings for achievement `2100243` are:

- restriction `39281`: the achievement must not be completed;
- restriction `42280`: the achievement must be completed.

A restriction row becomes active only when its referenced achievement exists
and is enabled. The seeded mappings therefore do not change stock-server spell
behavior before achievement `2100243` content is actually deployed.

## Importing optional ToB presentation data

The importer validates exactly five resource files. Without progression
selection options, it writes only the five presentation tables:

```powershell
python utils\scripts\import_achievement_resources.py `
  "F:\EQ1\client\EQ TOB Client\Resources\Achievements" `
  --output achievements-ui.sql
```

Use `--validate-only` to inspect the resource set without producing SQL.
`--strict-references` turns the known dangling references in the source files
into errors. Every required resource file must contain at least one data row;
an empty snapshot is rejected before SQL generation.

To enable a progression-era slice while importing the complete presentation
snapshot, pass `--enable-through-expansion`. Expansion names and common
abbreviations are accepted case-insensitively:

```powershell
python utils\scripts\import_achievement_resources.py `
  "F:\EQ1\client\EQ TOB Client\Resources\Achievements" `
  --enable-through-expansion don `
  --output achievements-ui-don.sql
```

This example enables definitions categorized under each expansion from
EverQuest through Dragons of Norrath. It also enables the unambiguous
`General / Level`, level-locked `General / Progression`, and class-specific
`General / Skills` milestone definitions through DoN's level cap of 70.
Era-appropriate `General / Class` Epic definitions are included as well: the
original classes' Epic 1.0 rows at launch, Beastlord at Luclin, Berserker at
Gates of Discord, and Epic 1.5/2.0 at Omens of War. The level cap may be
overridden with `--max-level`; using `--max-level` without an expansion selects
only the three level-derived General families and does not infer an Epic era.

Level criteria are generated only when the definition has the exact validated
`General / Level` shape in the resource snapshot: achievement ID and
`Level N` name agree, and it has exactly one state-bearing component with
sequence `1`, type `1`, component ID `N`, description `Reach Level N`, and
required count one. A mismatch aborts the import instead of guessing. Type `3`
presentation-only components do not affect this validation.

The level-locked `Reach Level N` rows are a separate Progression family. They
receive the same absolute Level fact only when the definition has the exact
client shape: matching name and description, one sequence-0/type-1 `Reach level
N` component whose component ID is the achievement ID, required count one, and
the `On a Level Locked Server` type-3 marker. Through DoN this adds the level
60, 65, and 70 rows. It intentionally does not infer the neighboring
level-limited raid challenges.

Class proficiency rows under `General / Skills` use the Skill Cap event rather
than a static Skill Value. Every definition must identify one reviewed EQ class
in its type-3 marker and every state component must exactly say `Reach the
maximum skill in Skill at level N.` The importer maps those client names to
stable EQ class and skill IDs. At runtime, the criterion verifies the current
class and level, resolves the server's DB-backed maximum for that class, skill,
and milestone level, rejects a zero/unavailable cap, and compares the raw skill
against that resolved maximum. A malformed name, unknown skill, duplicate
skill, class mismatch, or displayed-count mismatch rejects the complete
definition instead of partially enabling it.

The selected expansion definitions also receive criteria for these validated
shapes:

- A leaf name ending in ` Traveler` with exactly one sequence-1, type-1
  `Visit ...` component and required count one receives a Boolean Zone Enter
  criterion. Its zone is decoded as `(achievement_id // 100) % 1000`.
  Muramite Proving Grounds Trials achievement `930400` is the one validated
  multi-component exception; its six ordered components map to zones 304
  through 309.
- A type-1 or type-2 state component whose description exactly names one
  uniquely selected, different achievement receives an Achievement Complete
  criterion. Type 1 is Required and type 2 is Optional. Duplicate names and
  self references are reported and skipped; a generated dependency cycle
  aborts the import. A structurally validated direct event mapping owns its
  component and takes precedence over this name-only inference. Suppressed
  dependency identities are emitted as narrowly scoped `enabled = 0` updates
  so rerunning the importer also repairs rows generated by an older version.
- An exact `Hunter of X` definition under a Hunter or Hunts category receives
  zone-scoped NPC Name Kill criteria only when `X` resolves to a validated
  `X Traveler`. Duplicate Hunter names must also resolve through one matching
  structural ID family, which selects the original zone definition instead of
  a newer enhanced copy. Exact child-achievement components are excluded,
  required counts must be one, and duplicate canonical identities or name-hash
  collisions are reported and skipped. Raid and `Conqueror` prose is never
  converted automatically.
- Selected `General / Keys` state components and era-appropriate Epic
  components receive Own Item criteria by an exact, case-insensitive item-name
  join against the destination server's `items` table. This resolves every
  duplicate item ID with the same name as an OR alternative, which covers both
  legacy and newer copies such as the Blade of Tactics and Blade of Strategy.
  Epic rows also carry the validated EQ class ID in `target_id2`; keys use zero.
  Ownership reconciliation checks carried inventory, bank, shared bank, cursor,
  and the durable keyring, so retaining a keyring entry satisfies its item
  criterion. Item-name joins never use client component IDs as item IDs.
  Components with the same item name twice cannot be assigned safely and are
  rejected as a whole; the Beastlord Epic 1.0 pair is the known DoN case.

Servers imported before these criteria were available must rerun the importer
with their normal `--enable-through-expansion` and `--max-level` selection,
apply the generated SQL, and use `#reload achievements global` (or restart
zones). Changing `enabled` alone cannot synthesize the Skill Cap or item-name
rows.

If the database's enabled flags have already been curated manually, add
`--preserve-enable-state`. Selection options still generate the corresponding
criteria, and a selected definition missing from the database is inserted
enabled, but the SQL emits no enabled-state update for definitions that already
exist:

```powershell
python utils\scripts\import_achievement_resources.py `
  "F:\EQ1\client\EQ TOB Client\Resources\Achievements" `
  --enable-through-expansion don `
  --max-level 70 `
  --max-tradeskill-skill 300 `
  --max-aa-spent 150 `
  --preserve-enable-state `
  --output achievements-ui-don-refresh.sql
```

This option cannot be combined with `--exact-enable-selection`, whose purpose
is to replace the imported snapshot's enabled-state selection.

Direct global tradeskill milestones are intentionally not pulled into an
expansion selection. They require an explicit skill-cap opt-in:

```powershell
python utils\scripts\import_achievement_resources.py `
  "F:\EQ1\client\EQ TOB Client\Resources\Achievements" `
  --enable-through-expansion don `
  --max-tradeskill-skill 300 `
  --exact-enable-selection `
  --output achievements-ui-don.sql
```

Only exact `Skill (N)` definitions under the top-level Tradeskill hierarchy
with a matching `Reach N skill in Skill` component, matching definition text,
component ID equal to achievement ID, and required count one are enabled by
this option. An explicit table maps the client names to stable EQ skill IDs;
other Tradeskill definitions are not selected by this option.

Spent-AA milestones are also global General definitions rather than
expansion-category definitions, so they require a separate explicit cutoff:

```powershell
python utils\scripts\import_achievement_resources.py `
  "F:\EQ1\client\EQ TOB Client\Resources\Achievements" `
  --enable-through-expansion don `
  --max-aa-spent 150 `
  --exact-enable-selection `
  --output achievements-ui-don.sql
```

The value `150` is only an operator-selected example; the newer client
snapshot does not identify the expansion in which an AA-spent milestone was
introduced. The importer never derives an AA cutoff from
`--enable-through-expansion`. This option selects only exact
`N Alternate Advancement Points` definitions under `General / Advancement`
whose description says they are completed by spending `N` points and which
have exactly one sequence-1, type-1 `Spend N Alternate Advancement Points`
state component with required count one. It emits an absolute Boolean
Alternate Advancement criterion with target value `N`; malformed shapes are
reported and left disabled. The component ID is retained only as its client
presentation identity; newer milestones intentionally stop using the
achievement ID as their component ID.

If the General / Advancement definitions were imported or manually enabled
before this option was available, rerun the importer with the intended
`--max-aa-spent` cutoff and apply its SQL. Enabling definitions alone does not
create evaluation criteria. The generated SQL upserts only the validated
AA-spent criterion identities, so a presentation replacement is not required:

```powershell
python utils\scripts\import_achievement_resources.py `
  "F:\EQ1\client\EQ TOB Client\Resources\Achievements" `
  --max-aa-spent 150 `
  --preserve-enable-state `
  --output achievements-aa-spent.sql
```

Use `--preserve-enable-state` on this rerun when retaining manually adjusted
`enabled` flags; exact selection would disable other imported definitions not
selected by this command. After applying the SQL, use
`#reload achievements global` (or restart active zone processes) to load the
new criteria.

Slayer is also global rather than expansion-scoped and is never inferred from
`--enable-through-expansion`. On the initial activation, or when existing
imported Slayer definitions are still disabled, enable it with the explicit
switch and omit `--preserve-enable-state`:

```powershell
python utils\scripts\import_achievement_resources.py `
  "F:\EQ1\client\EQ TOB Client\Resources\Achievements" `
  --enable-slayer `
  --output achievements-slayer.sql
```

For a later criteria-only refresh of Slayer definitions that are already
enabled, add `--preserve-enable-state` to retain their current enabled flags.
That option deliberately emits no enabled-state update for definitions already
in the database, so it cannot perform the initial Slayer activation.

Direct Slayer components use native `NpcRaceKill` criteria in Increment mode.
Each credited raid, group, or solo NPC death emits the NPC's stable base race
ID through the ordinary achievement event path and increments
`character_achievement_progress`. This implementation does not read, update,
replace, or depend on the server's separate custom `account_kill_counts`
system, and it does not fabricate historical race kills during login replay.

The importer maps an exact, reviewed Slayer component vocabulary to RoF2 race
IDs from `common/races.h`. A direct Slayer definition is rejected as a whole
when any component has an unsupported type, invalid required count, unknown
race term, or empty mapping; it is never partially enabled. Quoted Slayer
meta-achievements are selected only when every required name resolves uniquely
to another safely selected Slayer definition, and their generated
Achievement Complete dependencies remain cycle-checked. Rejection reasons are
written into the generated SQL comments and validation report for review.

`--enable-slayer` applies to the complete validated top-level Slayer hierarchy,
not the DoN category boundary; combining it with
`--enable-through-expansion don` does not restrict Slayer selection to DoN.
The accepted definition and criterion counts depend on the client snapshot and
the current reviewed race map, so inspect the generated validation report
rather than relying on a fixed expected count. Ambiguous race terms and
later-client terms without a reviewed RoF2 race mapping remain rejected, along
with any dependent meta-achievements. Combine the switch with other selection
options only when that whole Slayer scope is intended.

The item join is deliberately exact rather than fuzzy. Some newer-client Key
rows describe non-item zone/progression flags, and some old key display names
do not exist in a destination server's `items` table. Those rows do not gain a
false item criterion merely because a similarly named item exists; they still
need a reviewed server-authored criterion or quest-script completion call.
For the reported concrete cases, Bone Crafted Key resolves to the destination
item ID (PEQ ID 6378), while Warrior Epic 1.0 resolves all exact Blade of
Tactics and Blade of Strategy item-ID variants.

The selection is derived from the client category hierarchy, not achievement
ID ranges. By default, definitions outside the selection are inserted disabled,
while pre-existing definitions outside the selection retain their current
enable state. Selected pre-existing definitions are explicitly enabled. To
make the imported snapshot's enable state exactly match the selection, add
`--exact-enable-selection`. That option disables only unselected achievement
IDs present in the validated resource snapshot, so custom achievements absent
from those files are untouched. The importer still writes the complete
category and presentation snapshot. It cannot be combined with
`--replace-existing`, whose global deletion semantics would contradict the
custom-ID guarantee.

Enablement makes definitions available to the runtime and client. The
validated shapes above are the only definitions for which the importer creates
evaluation policy. Task/quest completion, recipe success, non-item key flags,
and bespoke raid mechanics still need reviewed `achievement_criteria` rows or
quest-script calls. Client component IDs remain presentation identities and
are never treated as task, NPC type, zone, item, skill, or
dependent-achievement IDs.

The category cutoff identifies the expansion branch in which content is
presented; it is not generally a reliable achievement-release chronology. For
the audited DoN profile, the importer therefore applies an additional
fail-closed RoF2 boundary to this newer client snapshot: Traveler zone IDs above
345 and reserved later-use zone 213 are removed, their structural ID families
are removed, newer enhanced availability rows are removed, and parents that
require a removed exact-name child are pruned recursively. Optional removed
children do not prune a parent. Guild Lobby and Guild Hall zones 344 and 345
remain in the DoN slice.

The supplied ToB set currently validates as 286 categories, 6,203
achievements, 6,358 associations, 30,939 components, and 3,953 displayed
counts. Through DoN it selects 982 definitions and emits 4,658 fixed criteria
plus 107 exact-item-name mappings: 14 ordinary level, 3 level-locked
Progression, 1,629 class Skill Cap, 279 Traveler, 545 dependency, and 2,188
zone-scoped Hunter rows. The item mappings cover 38 selected Key definitions
and 47 additionally selected Epic definitions; one duplicate-name Epic shape
is rejected. Adding the explicit tradeskill cap of 300 selects 1,057
definitions and emits 4,733 fixed criteria plus the same item mappings,
including 75 Skill Value rows. The DoN compatibility boundary removes 38 newer
or outlier definitions from otherwise older category branches. It skips 30
self dependencies, supersedes three false name-inferred dependencies where
ordinary Level 60/65/70 components collide with level-locked Progression
names, and skips 22 Hunter definitions whose location does not resolve exactly;
no selected level-derived, Traveler, ambiguous or invalid dependency, named
component, or within-zone hash collision is rejected. The complete resource
set also contains one missing category reference, 98 association references to
missing achievements, and 99 component references to missing achievements.
Those warnings are preserved rather than guessed around.

Without progression selection options, new definitions created by this
importer are disabled and existing definitions retain their current `enabled`
value. Review the RoF2 presentation, author explicit criteria and rewards, and
then enable only the definitions intended for production.

`--replace-existing` performs a global exact presentation snapshot using
temporary key tables. Do not use it on a mixed/custom content database unless
that deletion scope is intended. It removes stale presentation rows while
preserving server-authored fields such as definition version, reset policy,
enable state, component secondary text, criteria, rewards, restrictions, and
character state.

Generated criteria are deliberately upsert-only: the importer does not delete
other criterion identities because it cannot distinguish older generated rows
from server-authored policy without provenance. Before deploying output from a
later importer policy or a materially changed resource snapshot, review and
remove any obsolete previously generated identities explicitly. The current
DoN generator should therefore be deployed from a clean criteria import or
with a reviewed, narrowly scoped cleanup; it never issues a broad criteria
delete.

The optional crosswalk imports ToB field 6 into the shared reward-display
concept. That is an importer choice, not evidence about the RoF2 wire layout.
ToB field 7 is retained as `world_display_flag` for provenance but is never
serialized to RoF2. The narrowly documented structural mappings above do not
infer rewards or packet layouts. No ToB packet layout or reward behavior is
used by the runtime implementation;

## Operational controls

- `Achievements:EnableAchievements` enables loading, evaluation, persistence,
  and RoF2 packets.
- `Achievements:GrantRewards` enables configured reward delivery.
- `Achievements:GuildMemberNotifications` (default `true`) controls whether
  completion announcements are sent to other online guild members. Local
  completion sound, chat, and link delivery are unchanged.
- `Achievements:NearbyPlayerNotifications` (default `true`) controls whether
  the native completion announcement is also sent to nearby RoF2 players in
  the same zone. This path is independent of guild announcements, so a nearby
  guild member can receive both when both rules are enabled.
- `Achievements:NearbyPlayerNotificationDistance` (default `200`) sets the
  maximum three-dimensional distance for nearby completion announcements. A
  value of `0` disables proximity delivery even when the Boolean rule is on.
- `Achievements:CompletionNotificationIntervalMS` is the minimum delay between
  queued completion announcements. The default is 750 milliseconds; values
  below one millisecond are clamped to one.
- `Achievements:OwnershipReconcileIntervalMS` controls periodic authoritative
  owned-item reconciliation for live account-wide shared-bank changes. The
  default is 30 seconds; `0` disables only the periodic pass. Existing clients
  sample this rule when their achievement state loads, so reconnect them after
  changing it.
- Non-shared task-completion criteria require
  `TaskSystem:RecordCompletedTasks`, because the durable history row is the
  replay and reward-safety boundary.
- `#reload achievements` reloads achievement content in the current zone
  process. `#reload achievements global` reloads every zone process, including
  sleeping processes that may boot a zone later.
- `#achievement` is an online-player administration command. It targets the
  selected player, or the issuing GM when no player is selected. `find`,
  `list`, and `inspect` report loaded definitions and current component state;
  `set`, `add`, and `complete` use the normal persistent mutation path.
  `reset <achievement_id>` removes completion and progress while preserving
  reward ledgers. `reset <achievement_id> rewards` explicitly clears those
  ledgers too, allowing the rewards to be granted again after recompletion.
- Changing an `achievements.enabled` row needs only the achievement reload.
  After changing the `Achievements:EnableAchievements` rule itself, reload
  rules first and achievements second so the content reload samples the new
  rule value; for a global change, run `#reload rules global` followed by
  `#reload achievements global`.
- Reloading stages and validates a complete replacement before activating it.
  If loading fails, the existing achievement content and connected-client state
  remain active.
- After a successful reload, connected clients rebuild their indexed
  achievement state against the replacement definitions, reconcile durable
  progress, and receive fresh definition and state packets. Already-persisted
  completions are not announced again; newly satisfied definitions complete
  and announce normally. Reloading an empty or rule-disabled system also sends
  a real empty client model so stale window content is cleared. Reward delivery
  remains protected by the durable claim ledger.
- If one loaded-client state rebuild fails after a successful content swap,
  the replacement content remains active and that client's achievement state
  automatically retries loading once per second. Zoning, reconnecting, or a
  later reload also rebuilds it. The server logs the character and sends
  administrators a partial-reload warning. A zone restart is not required after
  changing definitions, criteria, rewards, restrictions, or enabled flags.
- The migrations create the engine schema but intentionally do not invent
  gameplay criteria or rewards. Importing ToB presentation data is optional;
  definitions outside the importer's exact Level, level-locked Progression,
  class Skill Cap, Own Item, Traveler, dependency, direct-tradeskill,
  AA-spent, Hunter, and explicitly opted-in Slayer shapes still require
  reviewed server-authored criteria and, when desired, reward rows.
- Content update `9377` installs achievement definitions, evaluation criteria,
  cast restrictions, and the achievement/task selectable-reward definitions.
- Character update `9378` installs achievement progress, reward-delivery state,
  task-reward occurrence state, and the durable pending-mutation queue used by
  cross-zone group, raid, dynamic-zone, and shared-task scripting.
