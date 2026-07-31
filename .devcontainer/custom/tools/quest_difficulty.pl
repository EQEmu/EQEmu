#!/usr/bin/perl
# quest_difficulty.pl -- score every quest in the server as ONE floating point number.
# =============================================================================================
# THE NUMBER
#
#   score = LEVEL BAND  +  AT-LEVEL DIFFICULTY
#           <ones place>  <the decimal>
#
#   ones place = floor(1 + level / 10), clamped 1..8   -- a level 30 quest bands at 4, a level 60
#                                                         quest at 7. Level is the single biggest
#                                                         input, so it owns the integer part and
#                                                         sorting by score sorts by level first.
#   decimal    = 0.00 .. 0.99                          -- how hard the quest is ASSUMING you are
#                                                         the right level for it. A level 60 fetch
#                                                         quest scores 7.05; a level 10 quest that
#                                                         drags you through six zones scores 2.80.
#
# ⚠️ The decimal is deliberately LEVEL-RELATIVE, which is what "assuming appropriate level" has to
# mean. Turn-in count and travel are absolute (ten turn-ins is ten turn-ins at any level), but the
# GEAR term is scored as how far the required content sits ABOVE the quest's own level -- content
# at your level adds nothing, content fifteen levels above you maxes the term. Scoring gear
# absolutely would have made every level 60 quest look hard purely for being level 60, which the
# integer part already says.
#
# THE THREE DECIMAL TERMS            weight
#   turn-ins   distinct items to hand in, and how many separate hand-in steps      0.35
#   travel     distinct zones the quest pulls you through                          0.35
#   gear       required content tier (0.70) + turn-in item quality (0.30)          0.30
#
# Each term saturates rather than growing without bound, so one absurd outlier cannot push a quest
# past its band into the next one -- the decimal is hard-capped at 0.99 for exactly that reason.
#
# TWO CORPORA, ONE REPORT
#   SCRIPT  quests/<zone>/<NPC>.lua|.pl   -- the classic hand-written turn-in quests
#   TASK    tasks + task_activities        -- the structured task system
# They are scored by the same model; only the extraction differs. Source is tagged per row.
#
#   perl .devcontainer/custom/tools/quest_difficulty.pl              # every quest, hardest first
#   perl .devcontainer/custom/tools/quest_difficulty.pl --zone=crushbone
#   perl .devcontainer/custom/tools/quest_difficulty.pl --source=task --limit=40
#   perl .devcontainer/custom/tools/quest_difficulty.pl --min=5.0    # only level 40+ bands
#   perl .devcontainer/custom/tools/quest_difficulty.pl --explain    # show the term breakdown
#   perl .devcontainer/custom/tools/quest_difficulty.pl --tsv        # machine readable
#
# Read-only: touches nothing but the database and the quest scripts.
# =============================================================================================

use strict;
use warnings;

my $QUESTS = "/src/.devcontainer/repo/quests";

# ---------------------------------------------------------------- options
my %opt = (limit => 0, min => 0, zone => '', source => '', explain => 0, tsv => 0);
for my $a (@ARGV) {
    if    ($a =~ /^--limit=(\d+)$/)      { $opt{limit}  = $1 }
    elsif ($a =~ /^--min=([\d.]+)$/)     { $opt{min}    = $1 }
    elsif ($a =~ /^--zone=(\w+)$/)       { $opt{zone}   = lc $1 }
    elsif ($a =~ /^--source=(\w+)$/)     { $opt{source} = lc $1 }
    elsif ($a eq '--explain')            { $opt{explain} = 1 }
    elsif ($a eq '--tsv')                { $opt{tsv}     = 1 }
    else { die "unknown option: $a\n" }
}

# ---------------------------------------------------------------- helpers
sub mysql {
    my $sql = shift;
    open(my $h, "-|", "mysql", "-h127.0.0.1", "-upeq", "-ppeqpass", "peq", "-N", "--batch", "-e", $sql)
        or die "mysql: $!";
    my @rows;
    while (my $l = <$h>) { chomp $l; push @rows, [split(/\t/, $l, -1)] if length $l; }
    close $h;
    return @rows;
}

# Saturating 0..1 curve. Grows fast at the low end then flattens, so the difference between one and
# three turn-ins matters far more than the difference between eleven and thirteen -- which is how
# the work actually feels. `full` is the value that scores ~1.0.
sub sat {
    my ($x, $full) = @_;
    return 0 if !$x || $x <= 0;
    my $v = log(1 + $x) / log(1 + $full);
    return $v > 1 ? 1 : $v;
}

sub clamp01 { my $v = shift; return $v < 0 ? 0 : ($v > 1 ? 1 : $v) }

# How far required content sits above the quest's own level. At or below level = 0, fifteen levels
# above = 1. This is the whole "assuming appropriate level" idea in one line.
sub over_level {
    my ($content_level, $quest_level) = @_;
    return 0 if !$content_level || !$quest_level;
    return clamp01(($content_level - $quest_level) / 15);
}

# ---------------------------------------------------------------- the model
# One place, so SCRIPT and TASK rows cannot drift apart.
sub score {
    my (%p) = @_;   # level, items, steps, zones, content_level, item_level

    my $band = int(1 + ($p{level} || 1) / 10);
    $band = 1 if $band < 1;
    $band = 8 if $band > 8;

    # turn-ins: distinct items carries most of it, extra hand-in STEPS add on top because each one
    # is another trip back to an NPC.
    my $t = 0.75 * sat($p{items}, 8) + 0.25 * sat(($p{steps} || 1) - 1, 4);

    # travel: one zone is the baseline (the quest's own), so it contributes nothing.
    my $z = sat(($p{zones} || 1) - 1, 5);

    my $g = 0.70 * over_level($p{content_level}, $p{level})
          + 0.30 * over_level($p{item_level},    $p{level});

    my $dec = 0.35 * $t + 0.35 * $z + 0.30 * $g;
    $dec = 0.99 if $dec > 0.99;

    return ($band + $dec, $band, $t, $z, $g);
}

# ---------------------------------------------------------------- pass 1: scan the scripts
# Collects, per file: the turn-in item ids (grouped per hand-in call) and every item id mentioned
# anywhere. The second set builds the item -> zones map below.
my (%quest, %item_zones, %all_items);

my @files;
if (opendir(my $d, $QUESTS)) {
    for my $zone (sort readdir $d) {
        next if $zone =~ /^\./;
        next if $zone =~ /^(lua_modules|plugins|globals?|mods|templates|items|spells)$/;
        my $zd = "$QUESTS/$zone";
        next unless -d $zd;
        next if $opt{zone} && lc($zone) ne $opt{zone};
        opendir(my $z, $zd) or next;
        for my $f (readdir $z) {
            next unless $f =~ /\.(lua|pl)$/;
            push @files, [$zone, $f, "$zd/$f"];
        }
        closedir $z;
    }
    closedir $d;
}

for my $e (@files) {
    my ($zone, $file, $path) = @$e;
    open(my $fh, '<', $path) or next;
    local $/;
    my $src = <$fh>;
    close $fh;

    my (@steps, %items);

    # Lua:  check_turn_in(e.trade, {item1 = 1234, item2 = 5678})
    while ($src =~ /check_turn_in\s*\([^,]*,\s*\{([^}]*)\}/g) {
        my $blk = $1;
        my @ids = ($blk =~ /item\d+\s*=\s*(\d+)/g);
        next unless @ids;
        push @steps, scalar @ids;
        $items{$_} = 1 for @ids;
    }
    # Perl: plugin::check_handin(\%itemcount, 1234 => 1, 5678 => 2)
    while ($src =~ /check_handin\s*\(\s*\\?%\w+\s*,([^)]*)\)/g) {
        my $blk = $1;
        my @ids = ($blk =~ /(\d+)\s*=>/g);
        next unless @ids;
        push @steps, scalar @ids;
        $items{$_} = 1 for @ids;
    }

    # Where a quest chain HANDS you an item: an explicit summonitem call. This is the other way a
    # required item enters the world besides being looted, and it is what makes a chain span zones.
    # ⚠️ Matched narrowly on the actual call. An earlier version also matched `item_id`/`itemid`
    # anywhere in the file, which swept up merchant tables and config and credited quests with
    # "travel" to 200 zones.
    for my $id ($src =~ /(?:summonitem|SummonItem)\s*\(\s*(\d{3,6})/g) {
        $item_zones{$id}{$zone} = 1;
        $all_items{$id} = 1;
    }

    next unless @steps;
    # ⚠️⚠️ Register the item as WANTED, never as AVAILABLE. %item_zones is the map of zones that
    # PROVIDE an item (summonitem, above); a zone that demands an item in a hand-in supplies
    # nothing. An earlier version added the quest's own zone here too, which made the "can I get
    # this without leaving?" test true for every quest's own items and drove script travel to
    # exactly zero across all 2,743 rows.
    for my $id (keys %items) { $all_items{$id} = 1 }

    (my $npc = $file) =~ s/\.(lua|pl)$//;
    $quest{"$zone/$npc"} = {
        zone  => $zone,
        npc   => $npc,
        items => [sort { $a <=> $b } keys %items],
        steps => scalar @steps,
    };
}

die "no quest scripts found under $QUESTS\n" unless %quest;

# ---------------------------------------------------------------- pass 2: the database
# Quest-giver levels, keyed zone/name exactly as the filenames are written.
my %npc_level;
for my $r (mysql(q{
        SELECT s2.zone, nt.name, MAX(nt.level)
        FROM npc_types nt
        JOIN spawnentry se ON se.npcID = nt.id
        JOIN spawn2 s2 ON s2.spawngroupID = se.spawngroupID
        GROUP BY s2.zone, nt.name})) {
    $npc_level{ lc($r->[0]) . '/' . $r->[1] } = $r->[2];
}

# Where a required item can actually be OBTAINED, and how tough the EASIEST source is.
#
# ⚠️⚠️ ONE SOURCE PER ITEM, NOT ALL OF THEM. A common drop can come from forty zones, but you only
# have to visit ONE of them -- so travel is the number of distinct zones you must actually go to,
# never the union of everywhere an item could theoretically come from. An earlier version unioned
# them and reported quests spanning 200 zones.
#
# ⚠️ MIN level, not MAX, for the same reason: a player farms the easiest source. Taking MAX let one
# high-level dropper somewhere in the world pin the gear term at its ceiling for nearly every quest.
#
# ⚠️ Most quest paper is not looted at all -- another script hands it to you -- so this comes back
# empty for a large share of ids. That is what the summonitem map above covers.
my (%loot_src, %item_gate);
if (%all_items) {
    my $ids = join(',', sort { $a <=> $b } keys %all_items);
    for my $r (mysql(qq{
            SELECT lde.item_id, s2.zone, MIN(nt.level)
            FROM lootdrop_entries lde
            JOIN loottable_entries lte ON lte.lootdrop_id = lde.lootdrop_id
            JOIN npc_types nt ON nt.loottable_id = lte.loottable_id
            JOIN spawnentry se ON se.npcID = nt.id
            JOIN spawn2 s2 ON s2.spawngroupID = se.spawngroupID
            WHERE lde.item_id IN ($ids) AND nt.level > 0
            GROUP BY lde.item_id, s2.zone})) {
        my ($id, $zone, $lvl) = @$r;
        # Track the easiest zone, but ⚠️ NEVER by replacing the hashref -- an earlier version did
        # `$loot_src{$id} = {...}` here, which threw away the {zones} set accumulated so far and made
        # the "can I get this without leaving?" test miss zones it had already seen.
        $loot_src{$id} ||= {};
        if (!defined $loot_src{$id}{level} || $lvl < $loot_src{$id}{level}) {
            $loot_src{$id}{zone}  = $zone;
            $loot_src{$id}{level} = $lvl;
        }
        $loot_src{$id}{zones}{$zone} = $lvl;
    }

    # Ground spawns: pick the thing up off the floor. No fight, so it costs travel but contributes
    # nothing to the gear term.
    for my $r (mysql(qq{
            SELECT gs.item, z.short_name
            FROM ground_spawns gs
            JOIN zone z ON z.zoneidnumber = gs.zoneid
            WHERE gs.item IN ($ids)
            GROUP BY gs.item, z.short_name})) {
        my ($id, $zone) = @$r;
        $loot_src{$id}{ground}{$zone} = 1;
    }

    # Merchants: buyable, so again travel but no combat. level_required is a real gate though.
    for my $r (mysql(qq{
            SELECT ml.item, s2.zone, MAX(ml.level_required)
            FROM merchantlist ml
            JOIN npc_types nt ON nt.merchant_id = ml.merchantid
            JOIN spawnentry se ON se.npcID = nt.id
            JOIN spawn2 s2 ON s2.spawngroupID = se.spawngroupID
            WHERE ml.item IN ($ids)
            GROUP BY ml.item, s2.zone})) {
        my ($id, $zone, $req) = @$r;
        $loot_src{$id}{shop}{$zone} = $req || 0;
    }

    # Turn-in item quality. reclevel is what the item WANTS you to be; reqlevel is the hard gate.
    for my $r (mysql(qq{
            SELECT id, GREATEST(reclevel, reqlevel) FROM items WHERE id IN ($ids)})) {
        $item_gate{$r->[0]} = $r->[1];
    }
}

# ---------------------------------------------------------------- pass 3: score the scripts
my @rows;
unless ($opt{source} && $opt{source} ne 'script') {
    for my $key (sort keys %quest) {
        my $q = $quest{$key};
        next unless @{$q->{items}};

        my $level = $npc_level{ lc($q->{zone}) . '/' . $q->{npc} } || 0;

        # Travel: each required item costs ONE trip, to whichever single zone is easiest to get it
        # from. If the quest's own zone can supply it, that trip is free.
        my (%zones, $content, $itemlvl);
        $zones{ $q->{zone} } = 1;
        for my $id (@{$q->{items}}) {
            my $s     = $loot_src{$id} || {};
            my $given = $item_zones{$id} || {};

            my $ground = $s->{ground} || {};
            my $shop   = $s->{shop}   || {};

            # Free if anything can supply it in the zone you are already standing in.
            if ($ground->{ $q->{zone} } || exists $shop->{ $q->{zone} }
                || ($s->{zones} && $s->{zones}{ $q->{zone} }) || $given->{ $q->{zone} }) {
                # no travel
            }
            # Otherwise one trip, to the cheapest kind of source available. Order matters: picking
            # something up off the floor or buying it beats having to kill for it.
            elsif (keys %$ground)      { $zones{ (sort keys %$ground)[0] } = 1 }
            elsif (keys %$shop)        { $zones{ (sort keys %$shop)[0]   } = 1 }
            elsif ($s->{zone})         { $zones{ $s->{zone} } = 1 }
            elsif (keys %$given)       { $zones{ (sort keys %$given)[0]  } = 1 }

            # Content tier = the toughest of the EASIEST sources: the hardest thing you cannot avoid
            # fighting. Only LOOT contributes -- a ground spawn or a merchant costs no combat. Items
            # with no known source contribute nothing rather than a guess.
            $content = $s->{level} if ($s->{level} || 0) > ($content || 0)
                                   && !keys %$ground && !keys %$shop;
            $itemlvl = $item_gate{$id} if ($item_gate{$id} || 0) > ($itemlvl || 0);
        }

        # No level on the quest giver (unspawned, or a name the file spells differently) -- fall back
        # to the toughest thing the quest makes you beat, then to the item's own level gate.
        $level ||= $content || $itemlvl || 1;

        my ($s, $band, $t, $z, $g) = score(
            level => $level, items => scalar @{$q->{items}}, steps => $q->{steps},
            zones => scalar keys %zones, content_level => $content, item_level => $itemlvl);

        push @rows, {
            score => $s, source => 'SCRIPT', name => "$q->{zone}/$q->{npc}",
            level => $level, items => scalar @{$q->{items}}, steps => $q->{steps},
            zones => scalar keys %zones, t => $t, z => $z, g => $g,
        };
    }
}

# ---------------------------------------------------------------- pass 4: score the DB tasks
unless ($opt{source} && $opt{source} ne 'task') {
    my %task;
    for my $r (mysql(q{SELECT id, title, min_level, max_level FROM tasks WHERE enabled = 1})) {
        $task{$r->[0]} = { title => $r->[1], min => $r->[2], max => $r->[3],
                           items => {}, zones => {}, steps => 0, kill_lvl => 0 };
    }
    # activitytype: 1 Deliver, 2 Kill, 3 Loot, 5 Explore, 11 Touch (common/tasks.h)
    for my $r (mysql(q{
            SELECT taskid, activitytype, goalcount, zones, item_id_list
            FROM task_activities WHERE optional = 0})) {
        my ($tid, $type, $goal, $zones, $items) = @$r;
        my $t = $task{$tid} or next;
        $t->{steps}++ if $type == 1;                       # a Deliver is a hand-in step
        $t->{items}{$_} = 1 for grep { $_ } split /[|,;]/, ($items // '');
        $t->{zones}{$_} = 1 for grep { $_ } split /[|,;]/, ($zones // '');
        $t->{goal} = ($t->{goal} || 0) + ($goal || 0) if $type == 1 || $type == 3;
    }

    for my $id (sort { $a <=> $b } keys %task) {
        my $t = $task{$id};
        next unless $t->{steps} || keys %{$t->{items}};
        next if $opt{zone};   # tasks are not filed under a zone directory

        my $level = $t->{min} || $t->{max} || 1;
        my $items = scalar keys %{$t->{items}};
        $items ||= $t->{goal} || 1;

        my ($s, $band, $tt, $z, $g) = score(
            level => $level, items => $items, steps => $t->{steps},
            zones => scalar keys %{$t->{zones}},
            content_level => $t->{max}, item_level => 0);

        push @rows, {
            score => $s, source => 'TASK', name => "[$id] $t->{title}",
            level => $level, items => $items, steps => $t->{steps},
            zones => scalar keys %{$t->{zones}}, t => $tt, z => $z, g => $g,
        };
    }
}

# ---------------------------------------------------------------- output
@rows = grep { $_->{score} >= $opt{min} } @rows;
@rows = sort { $b->{score} <=> $a->{score} || $a->{name} cmp $b->{name} } @rows;
my $total = scalar @rows;
@rows = @rows[0 .. $opt{limit} - 1] if $opt{limit} && $opt{limit} < @rows;

if ($opt{tsv}) {
    print join("\t", qw(score source name level items steps zones turnin_term travel_term gear_term)), "\n";
    printf("%.2f\t%s\t%s\t%d\t%d\t%d\t%d\t%.3f\t%.3f\t%.3f\n",
        $_->{score}, $_->{source}, $_->{name}, $_->{level}, $_->{items},
        $_->{steps}, $_->{zones}, $_->{t}, $_->{z}, $_->{g}) for @rows;
    exit 0;
}

printf("%-6s %-6s %-44s %5s %5s %5s %5s%s\n",
    'SCORE', 'SRC', 'QUEST', 'LVL', 'ITEM', 'STEP', 'ZONE',
    $opt{explain} ? '   TURNIN TRAVEL   GEAR' : '');
print '-' x ($opt{explain} ? 102 : 80), "\n";
for my $r (@rows) {
    printf("%-6.2f %-6s %-44.44s %5d %5d %5d %5d%s\n",
        $r->{score}, $r->{source}, $r->{name}, $r->{level},
        $r->{items}, $r->{steps}, $r->{zones},
        $opt{explain} ? sprintf('   %6.3f %6.3f %6.3f', $r->{t}, $r->{z}, $r->{g}) : '');
}
printf("\n%d quests scored%s.\n", $total, $opt{limit} && $opt{limit} < $total ? ", showing top $opt{limit}" : '');
