# RoF2 Select Reward support

Select Reward is a shared RoF2 client facility, not an achievement-owned
system. The zone keeps its packet/session and delivery machinery in
`ClientRewardSelection`; achievements and tasks are separate providers that
authorize requests, persist their own state, and supply reward definitions.

RoF2 exposes two independent manager lanes. They describe what the window may
do, not which gameplay system supplied the reward:

| Lane | Emulated opcode | RoF2 opcode | Client behavior |
| --- | --- | --- | --- |
| Claimable | `OP_AchievementReward` | `0x6411` | Shows **Choose** and permits action `3` |
| Preview | `OP_RewardSelection` | `0x6471` | Shows rewards, details, Inspect, and Preview, but hides **Choose** |

Sessions and request limits are independent per lane. The claimable lane holds
all outstanding provider sessions and presents them as native client tabs;
each tab is claimed and persisted independently. Achievements, tasks, and
future providers may use either lane according to authorization. In
particular, achievement action `5` is requested on `0x6411` but its read-only
View Reward response is deliberately installed on `0x6471` unless that exact
achievement has an earned, outstanding selection. A matching pending selection
is reopened on `0x6411`. Provider reloads clear only matching source sessions,
so reloading achievement data cannot discard a task's pending claim.

## Compatibility boundary

Selectable task rewards are strictly opt-in and RoF2-only:

- `tasks.reward_method = 3` (`METHODSELECT`) opts a task into the new path.
- The task must also have one valid, enabled reward set. Invalid or incomplete
  content is rejected and is not advertised to the client.
- Shared tasks remain on their existing reward methods. World may assign them
  to offline or older-client members whose Select Reward capability is
  unknown, so `METHODSELECT` shared-task sets fail closed until member
  capability negotiation exists.
- Existing task reward methods `0`, `1`, and `2`, including
  `reward_id_list`, retain their existing behavior.
- Titanium, SoF, SoD, UF, and RoF retain their existing task selector and task
  description bytes, including a zero reward-selection indicator.
  `METHODSELECT` tasks are not offered to or accepted from those clients, and
  they are never sent the RoF2 Select Reward opcode.
- Achievement rewards continue to use their existing achievement persistence
  tables. Their previews use the preview lane and outstanding selections use
  the claimable lane.

`METHODSELECT` replaces the automatic `reward_id_list` item grant with the
selectable set. Existing completion emote, faction, cash, experience, and
reward-point fields still run through the established task reward path. A
selectable set may also contain coin, experience, AA, currency, or title
entries; leave the corresponding legacy field empty when the same grant is in
the set so content does not award it twice.

## Task content

The content database owns four tables:

- `task_reward_sets`: one enabled set per task.
- `task_reward_options`: ordered choices within the set. An option marked
  `common_to_all = 1` is granted with every selectable option.
- `task_rewards`: typed grant entries owned by the task.
- `task_reward_option_entries`: maps each reward to exactly one option.

Reward, set, and option IDs sent over the wire must fit in an unsigned 32-bit
integer. Every enabled reward must be mapped exactly once, every enabled option
must contain at least one enabled reward, and a set must contain at least one
non-common option.

Supported reward types are:

| `reward_type` | Grant | `reward_data_id` | `amount` |
| ---: | --- | --- | --- |
| 0 | Item | Item ID | Stack/charge count |
| 1 | Experience | `0` = normal AA allocation; `1` = fixed normal XP only | Experience amount |
| 2 | Alternate advancement | 0 | AA points |
| 3 | Coin | 0 | Copper |
| 4 | Alternate currency | Currency ID | Currency amount |
| 5 | Title | Title-set ID | 1 |

Experience mode `0` uses the established `AddEXP` path, including the
character's AA allocation and configured experience modifiers. Mode `1`
preserves AA experience and adds the raw amount directly to normal experience;
it is the mode for rewards described as **No AA Exp**.

RoF2 supplies the visible list label **Player flags** for title/text rewards.
The row's `description` is shown in the lower detail pane, so title content
should use text such as `Unlocks the prefix and suffix titles Gatebreaker and
the Gatebreaker`.

Example with one common coin entry and two item choices:

```sql
UPDATE tasks SET reward_method = 3 WHERE id = 1001;

INSERT INTO task_reward_sets (reward_set_id, task_id, title)
VALUES (1001, 1001, 'Choose a Reward');

INSERT INTO task_reward_options
	(reward_set_id, option_id, sequence, label, common_to_all)
VALUES
	(1001, 1, 0, 'Completion coin', 1),
	(1001, 2, 1, 'First item', 0),
	(1001, 3, 2, 'Second item', 0);

INSERT INTO task_rewards
	(reward_id, task_id, sequence, reward_type, reward_data_id, amount)
VALUES
	(10001, 1001, 0, 3, 0, 1000),
	(10002, 1001, 1, 0, 5001, 1),
	(10003, 1001, 2, 0, 5002, 1);

INSERT INTO task_reward_option_entries
	(reward_set_id, option_id, reward_id)
VALUES
	(1001, 1, 10001),
	(1001, 2, 10002),
	(1001, 3, 10003);
```

## Completion and claim lifecycle

Accepting a selectable task reserves a fresh, server-owned occurrence token in
`character_task_reward_instances`. This does not modify
`character_tasks.accepted_time`; the gameplay timestamp keeps its established
timer and task-history meaning. A task that was already active when the
migration was installed gets its occurrence token lazily at completion.

Completion persists that token as `source_instance_id` in
`character_task_reward_selections` before the completed task can be removed
from `character_tasks`. The token distinguishes separate completions of a
repeatable task even when they were accepted during the same second.
`accepted_time` is copied into both rows only as diagnostic metadata and is not
used as the reward identity.

Completion then opens a copied snapshot on the claimable lane. Client requests
are handled as follows:

- Action 1 inspects an item in the matching tab snapshot.
- Action 3 claims one option. Provider validation checks the character,
  pending row, occurrence token, set, and option before delivery.
- Action 4 requests a read-only task preview on `0x6471`.
- Action 6 rebuilds all eligible pending and retryable tabs on `0x6411`.

The selected option becomes immutable on the first claim attempt. A claim
grants every common option plus the selected non-common option.
`character_task_rewards` is the per-entry delivery ledger, so a confirmed
entry is not granted again during a retry.

RoF2 action `7` replaces the claimable manager with a count followed by one
reward record per tab. Restoration combines task and achievement sessions in
one replacement packet. A successful claim identifies its tab by pending
reward ID and reward-set ID; the other tabs remain claimable.

Although database option IDs are scoped by reward set, the RoF2 manager's
detail cache treats them as global within the displayed collection. The zone
therefore assigns collection-unique wire option IDs and translates inspection
and claim requests back to the provider's canonical option ID. Reusing option
IDs between task and achievement content is safe.

Selection status values are:

| Status | Meaning | Automatically restored |
| ---: | --- | --- |
| 0 | Pending or claim in progress | Only while `selected_option_id = 0` |
| 1 | Fully delivered | No |
| 2 | Explicit, retryable delivery failure | Yes |
| 3 | Ambiguous delivery result | No |

An ambiguous result is deliberately not retried automatically because the
underlying grant may already have committed. It requires operator review.
On login or an action-6 restore request, an interrupted status-zero claim with
a selected option is reconciled from its entry ledger. Any in-flight entry
quarantines the selection as ambiguous; otherwise it becomes retryable and
already-delivered entries are skipped.

## Operational notes

Install content migration `9377` and character migration `9378` before starting
a zone that includes this support. The character migration normalizes existing
task-reward rows to occurrence tokens and removes the obsolete same-second
identity. Task reload validates and replaces the in-memory reward definitions;
active client sessions hold copied definitions, so a reload cannot leave
dangling pointers. Pending rows remain the durable authority across zoning,
disconnects, and server restarts.

When changing selectable reward content, avoid reusing a reward, option, or set
ID for a different semantic grant. Stable IDs are part of claim validation and
idempotency.
