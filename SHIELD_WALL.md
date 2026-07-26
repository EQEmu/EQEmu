# Shield Wall — sharing damage with `/shield`

On AoTv4, a group can **split a monster's melee damage between several players**. One person holds
aggro as normal, and anyone standing with them can take a share of every hit.

This is a rework of EverQuest's `/shield` command. On live it is a Warrior-only, 12-second panic
button. Here it belongs to everyone, it lasts as long as you stand together, and more than one
person can help at once.

---

## Turning it on

Target the person you want to protect and type:

```
/shield
```

You will both see a message, and from that moment you take a share of every melee hit that lands on
them.

The person you are protecting gains a **Shielded** buff for as long as it lasts, so they can see at
a glance that it is still up. It disappears the instant the shield breaks — which is exactly when
they need to notice.

**You get one Shielded buff per person shielding you** — three helpers means three buffs. Inspect
any of them to see who it came from, the same way you would check the caster of any other spell.
They update immediately as people start and stop, so what you see is always who is actually
protecting you right now.

**Requirements:** none. Any class, any level, no shield or weapon needed. You just have to be near
them — within about **25 units**, which is roughly melee range.

## Turning it off

Type `/shield` again:

```
/shield
```

You stop shielding, and you can immediately shield someone else — there is no cooldown on stopping.

It also **ends by itself** if:

- you move more than ~25 units apart (no message — just check your positions)
- either of you dies
- you zone

There is a short **6 second** cooldown before you can start shielding again after stopping.

---

## The catch: splitting costs more total health

This is the important part, and it is deliberate.

Damage is **not** split evenly and for free. Every extra person sharing a hit **increases the total
damage dealt**. Two people do not take 50% each — they take **60% each**.

| People sharing | Each person takes | Total damage taken |
|---|---|---|
| 1 (nobody shielding) | 100% | 100% |
| 2 | **60%** | 120% |
| 3 | **~47%** | 140% |
| 4 | **~40%** | 160% |

So a 1,000 damage hit on a lone tank becomes two hits of 600 when a second player shields them.

### Why would you ever do that?

Because **big single hits are what kill people.** A tank taking 1,000 in one swing can be dead before
a heal lands. Two people taking 600 each is 200 more damage overall, but neither of them is anywhere
near dying from one hit, and a healer has time to react.

You are trading **more total damage** for **smaller, more predictable damage**. That is a good trade
when hits are big and scary, and a bad one when they are small and your healer is comfortable.

### When to use it

- **Yes:** hard-hitting monsters, anything that can spike a tank down fast, fights where your healer
  is struggling to keep up with burst
- **Yes:** the tank is undergeared or underlevelled for what you are fighting
- **No:** trash mobs, or anything already dying comfortably — you are just burning extra health and
  extra healing for nothing
- **Careful:** stacking three or four people is progressively worse value. Four sharers take 160% of
  the original damage between them. Only worth it against something genuinely dangerous.

---

## Practical notes

- **Anyone can shield anyone.** It does not have to be a tank, and you do not have to be grouped.
- **You cannot shield someone who is shielding you.** No damage loops.
- **Up to 4 people** can share one hit, including whoever has aggro.
- **You keep taking your share even without aggro.** The monster is not hitting you — you are
  choosing to absorb part of what it does to your friend. It will not turn on you for shielding.
- **Only melee hits are shared.** Spells the monster casts, damage over time and area effects still
  land entirely on their own targets.
- **Watch your own health.** The person shielding is not the one being healed by default. Say so in
  group chat before you start, or you may quietly die while helping.

---

## Server settings

For server operators, all of it is tunable in `rule_values` without a rebuild (zone restart to
apply):

| Rule | Default | Meaning |
|---|---|---|
| `AoT:ShieldAnyClass` | `true` | Any class may use `/shield`. `false` restores Warrior-only. |
| `AoT:ShieldMinLevel` | `1` | Level needed to use it (live is 30). |
| `AoT:ShieldPermanent` | `true` | Lasts until you separate. `false` restores the 12 second duration. |
| `AoT:ShieldRecastSeconds` | `6` | Cooldown after stopping (live is 180). |
| `AoT:ShieldDistance` | `25` | How far apart you may drift before it drops (live is 15). |
| `AoT:ShieldWallPenaltyPercent` | `20` | Extra damage per additional sharer. `0` makes splitting free. |
| `AoT:ShieldWallMaxSharers` | `4` | Maximum people sharing one hit, including the aggro holder. |

The share formula is `damage * (100 + penalty * (N-1)) / (N * 100)`, where `N` is the number of
people sharing.
