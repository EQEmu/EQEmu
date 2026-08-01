#!/usr/bin/perl
# AoTv4 -- build the QUEST CATALOGUE that backs the Allaclone "Quests" mode.
# =============================================================================================
# Scans every quest script and emits custom/sql/aotv4_quest_catalogue.sql: one row per NPC file
# that performs a hand-in, carrying the turn-in item ids and a difficulty score.
#
# ⚠️⚠️ THE PARSER IS COPIED FROM quest_difficulty.pl ON PURPOSE, AND MUST STAY IN STEP WITH IT.
# Both read `check_turn_in` (Lua) and `plugin::check_handin` (Perl); if one learns a new hand-in
# form and the other does not, the catalogue and the difficulty report silently describe different
# sets of quests. The regexes below are the same two, verbatim.
#
# ⚠️ Unit of "a quest" is one NPC FILE, exactly as in quest_difficulty.pl section 23. An NPC that
# implements several independent quests reads as one many-step quest. That is a known
# approximation, not a bug to fix here -- fixing it means understanding script control flow.
#
# ⚠️ ONLY TURN-INS ARE VISIBLE. A quest gated on saying a keyword, or on killing something first,
# has no machine-readable progress and will show requirements only. The window says so; do not
# let the catalogue imply it knows more than it does.
#
# ⚠️ `summonitem` is deliberately NOT treated as a requirement. It is how a chain HANDS you an
# item, so counting it would list rewards as things to go and fetch.
#
# Usage:  perl .devcontainer/custom/tools/gen_quest_catalogue.pl [--limit N] [--zone NAME]
# Then:   mysql ... < .devcontainer/custom/sql/aotv4_quest_catalogue.sql

use strict;
use warnings;

my $QUESTS = ".devcontainer/repo/quests";
my $OUT    = ".devcontainer/custom/sql/aotv4_quest_catalogue.sql";
# ⚠️⚠️ THE SERVER READS THE LUA MODULE, NOT THE SQL TABLE. Lua has no arbitrary-query binding, so a
# table would be unreachable from the quest scripts that need it. The .sql is emitted purely so the
# catalogue can be inspected and joined with mysql from the shell; nothing at runtime touches it.
# Same convention as spell_pool.lua / aa_pool.lua / pok_portals.lua.
my $LUA_OUT = ".devcontainer/repo/quests/lua_modules/quest_catalogue.lua";

# ---------------------------------------------------------------- db helper (from quest_difficulty.pl)
sub mysql {
    my ($sql) = @_;
    open(my $h, "-|", "mysql", "-h127.0.0.1", "-upeq", "-ppeqpass", "peq", "-N", "--batch", "-e", $sql)
        or die "mysql: $!";
    my @rows;
    while (<$h>) { chomp; push @rows, [split /\t/, $_, -1] }
    close $h;
    return @rows;
}

my ($LIMIT, $ONLY_ZONE) = (0, '');
for (my $i = 0; $i < @ARGV; $i++) {
    $LIMIT     = $ARGV[++$i] if $ARGV[$i] eq '--limit';
    $ONLY_ZONE = $ARGV[++$i] if $ARGV[$i] eq '--zone';
}

die "run from /src (no $QUESTS)\n" unless -d $QUESTS;

# ---------------------------------------------------------------- pass 1: scan the scripts
my @files;
opendir(my $qd, $QUESTS) or die "opendir $QUESTS: $!";
for my $zone (sort readdir $qd) {
    next if $zone =~ /^\./;
    next unless -d "$QUESTS/$zone";
    # not zone directories
    next if $zone =~ /^(lua_modules|plugins|global|mods|templates|items|spells|bots|merchants)$/;
    next if $ONLY_ZONE && $zone ne $ONLY_ZONE;
    opendir(my $d, "$QUESTS/$zone") or next;
    for my $f (sort readdir $d) {
        next unless $f =~ /\.(lua|pl)$/;
        push @files, [$zone, $f, "$QUESTS/$zone/$f"];
    }
    closedir $d;
}
closedir $qd;

my %quest;
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

    next unless @steps;

    (my $npc = $file) =~ s/\.(lua|pl)$//;
    $quest{"$zone/$npc"} = {
        zone  => $zone,
        npc   => $npc,
        items => [sort { $a <=> $b } keys %items],
        steps => scalar @steps,
    };
}

die "no quest scripts found under $QUESTS\n" unless %quest;

# ---------------------------------------------------------------- emit
# Score: same shape as quest_difficulty.pl -- integer part is the level band, decimal is how hard
# it is at that level. Level is resolved at APPLY time from npc_types, because the script filename
# is the NPC's name and joining it here would mean a second DB round trip in a generator that is
# otherwise pure text. The SQL below does that join once, in the database, where it is cheap.
sub sq { my $s = shift // ''; $s =~ s/\\/\\\\/g; $s =~ s/'/\\'/g; return $s }

my @keys = sort keys %quest;
@keys = @keys[0 .. $LIMIT - 1] if $LIMIT && $LIMIT < @keys;

open(my $out, '>', $OUT) or die "write $OUT: $!";
print $out <<"HEAD";
-- AoTv4 quest catalogue -- GENERATED by custom/tools/gen_quest_catalogue.pl, do not hand edit.
-- Rows: one per quest script that performs a hand in.
--
-- ⚠️ Regenerate after any change to the quest scripts; nothing detects drift automatically.
-- ⚠️ `npc_level` and `zone_id` are filled by the UPDATE at the foot of this file, by joining
--    npc_types/zone on the script's own name. A quest whose NPC cannot be resolved keeps level 0
--    and is still listed -- the level is for sorting and display, never a gate.
-- ⚠️ NOT in shared memory. A zone restart is enough; no ./shared_memory rebuild.

DROP TABLE IF EXISTS aotv4_quest_catalogue;
CREATE TABLE aotv4_quest_catalogue (
  id          INT UNSIGNED NOT NULL AUTO_INCREMENT,
  zone        VARCHAR(32)  NOT NULL,
  zone_id     INT UNSIGNED NOT NULL DEFAULT 0,
  npc         VARCHAR(64)  NOT NULL,
  npc_display VARCHAR(64)  NOT NULL,
  npc_level   INT UNSIGNED NOT NULL DEFAULT 0,
  steps       INT UNSIGNED NOT NULL DEFAULT 1,
  -- ⚠️ TEXT, not VARCHAR(255). A few multi step NPCs demand more than 40 distinct items and the
  -- comma separated id list overflows 255 -- which fails as a hard ERROR 1406 mid import, leaving
  -- the catalogue half loaded rather than simply truncating.
  items       TEXT         NOT NULL,
  PRIMARY KEY (id),
  KEY idx_zone (zone),
  KEY idx_zone_id (zone_id),
  KEY idx_npc_display (npc_display)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO aotv4_quest_catalogue (zone, npc, npc_display, steps, items) VALUES
HEAD

my @rows;
for my $k (@keys) {
    my $q = $quest{$k};
    (my $disp = $q->{npc}) =~ s/_/ /g;     # scripts store the name underscored
    $disp =~ s/^#//;                        # leading # marks a named mob, not part of the name
    push @rows, sprintf("  ('%s','%s','%s',%d,'%s')",
        sq($q->{zone}), sq($q->{npc}), sq($disp), $q->{steps}, join(',', @{ $q->{items} }));
}
print $out join(",\n", @rows), ";\n";

print $out <<'TAIL';

-- Resolve the NPC's level and the numeric zone id. Done here rather than in the generator so the
-- script stays a pure text pass over the quest tree.
-- ⚠️ Match on npc_types.name with underscores intact -- that is how the column is stored AND how
-- the script filename is written, so no transformation is needed or wanted.
UPDATE aotv4_quest_catalogue c
JOIN   zone z ON z.short_name = c.zone
SET    c.zone_id = z.zoneidnumber;

UPDATE aotv4_quest_catalogue c
JOIN  (SELECT n.name, MIN(n.level) AS lvl
       FROM   npc_types n
       GROUP  BY n.name) x ON x.name = c.npc
SET    c.npc_level = x.lvl;

SELECT CONCAT('catalogue rows: ', COUNT(*),
              '   with zone_id: ',  SUM(zone_id > 0),
              '   with level: ',    SUM(npc_level > 0)) AS summary
FROM   aotv4_quest_catalogue;
TAIL
close $out;

# ---------------------------------------------------------------- pass 2: enrich from the database
# Levels and numeric zone ids, resolved here so the Lua module is self contained and the server
# never needs a query it has no binding for.
my (%zone_id, %npc_level, %item_name);
for my $r (mysql(q{SELECT short_name, zoneidnumber FROM zone})) { $zone_id{ $r->[0] } = $r->[1] }
# ⚠️ MIN(level): a name can exist at several levels across zones and the LOWEST is the one a player
# meets first, so it sorts the catalogue the way a player would expect. Same "easiest source wins"
# reasoning as quest_difficulty.pl's MIN dropper level.
for my $r (mysql(q{SELECT name, MIN(level) FROM npc_types GROUP BY name})) { $npc_level{ $r->[0] } = $r->[1] }

# ---------------------------------------------------------------- the TASK corpus
# ⚠️⚠️ THE ONLY REAL SOURCE OF QUEST TITLES ON THIS SERVER. Script quests have none -- their only
# leading comment is a generated "-- items: 123, 456" list -- so a script quest can only be
# identified by its NPC and zone. The 526 enabled rows in `tasks` DO have authored titles, levels
# and real quantities, so they are folded in and shown by name.
#
# ⚠️ These are INFORMATIONAL in our window. A task's progress belongs to the engine's task system and
# its native journal; we can show what it wants and what you are carrying, but assigning and
# completing one happens through the task system, not through us.
#
# ⚠️ Item requirements come from activitytype 1 (Deliver) and 3 (Loot) only. Kill and Explore steps
# carry no item and would read as an empty requirement.
#
# ⚠️⚠️ COLLECTED HERE, BEFORE %want IS BUILT. Doing it down at the emit loop -- the obvious place --
# adds these ids to %want AFTER the item-name query has already run, so every task item renders as
# a bare "item 13791" with no name and nothing reports a problem.
my %task;
for my $r (mysql(q{
        SELECT t.id, t.title, t.min_level, a.item_id_list, a.goalcount, a.zones
        FROM   tasks t
        JOIN   task_activities a ON a.taskid = t.id
        WHERE  t.enabled = 1 AND a.activitytype IN (1,3) AND a.item_id_list <> ''
        ORDER  BY t.id})) {
    my ($tid, $title, $minlvl, $ilist, $gcount, $zones) = @$r;
    next unless defined $title && $title ne '';
    my $t = $task{$tid} ||= { title => $title, lvl => ($minlvl || 0), zid => 0, items => {}, steps => 0 };
    # ⚠️ `zones` can be a LIST; the first entry is the one shown. Display only, never a gate.
    if (!$t->{zid} && defined $zones && $zones =~ /^(\d+)/) { $t->{zid} = $1 }
    $t->{steps}++;
    for my $id (split /[,;|]/, $ilist) {
        next unless $id =~ /^\d+$/;
        # Highest goalcount wins: a Loot 4 and a Deliver 4 describe the SAME four items, not eight.
        my $need = ($gcount && $gcount > 0) ? $gcount : 1;
        $t->{items}{$id} = $need if !$t->{items}{$id} || $t->{items}{$id} < $need;
    }
}

# ⚠️⚠️ ITEM NAMES ARE NOT BAKED IN. They were, and it cost 192 KB of the module's 580 KB for
# 5,774 entries -- which pushed the file past tooling size limits -- to duplicate something the
# server already has: `eq.get_item_name(id)` (lua_general.cpp) reads it straight out of shared
# memory. The window resolves a handful of ids per detail view, so the lookup is free and the names
# can never go stale against the database the way a generated copy does.
# ⚠️ Do not "optimise" this back into the file. The only thing the table bought was avoiding a
# binding that already exists.

sub lq { my $s = shift // ''; $s =~ s/\\/\\\\/g; $s =~ s/"/\\"/g; return $s }

# ---------------------------------------------------------------- STABLE quest numbers
# ⚠️⚠️ THE QUEST NUMBER IS SHOWN TO PLAYERS AND IS USED TO LOOK A QUEST UP, so it MUST NOT be an
# array position. Sorting is deterministic, but adding or removing a single script shifts every
# entry after it -- so a number somebody wrote down last week would silently point at a different
# quest. Instead the previous catalogue is read back and every quest KEEPS the number it already
# had; only genuinely new ones are assigned, from the high-water mark.
#
# ⚠️ The identity key is "s:<zone>/<npc>" or "t:<taskid>", NOT the display name. Renaming an NPC file
# is a new quest as far as this is concerned, which is correct -- the old script is gone.
# ⚠️ Retired quests keep their number reserved (the high-water mark never goes down), so a number is
# never recycled onto a different quest.
my (%old_id, $max_id);
$max_id = 0;
if (open(my $prev, '<', $LUA_OUT)) {
    while (my $line = <$prev>) {
        next unless $line =~ /^\s*\{\s*id=(\d+),/;
        my $id = $1;
        $max_id = $id if $id > $max_id;
        if    ($line =~ /src="s".*?zone="([^"]*)".*?npc="([^"]*)"/) { $old_id{"s:$1/$2"} = $id }
        elsif ($line =~ /src="t".*?tid=(\d+)/)                     { $old_id{"t:$1"}    = $id }
    }
    close $prev;
}
my $reused = 0;
sub quest_id {
    my ($key) = @_;
    if (defined $old_id{$key}) { $reused++; return $old_id{$key} }
    return ++$max_id;
}

open(my $lua, '>', $LUA_OUT) or die "write $LUA_OUT: $!";
print $lua <<'LHEAD';
-- AoTv4 quest catalogue -- GENERATED by custom/tools/gen_quest_catalogue.pl. DO NOT HAND EDIT.
--
-- One entry per quest script that performs a hand in. Backs the Allaclone "Quests" and "Tracked"
-- modes (aotv4_questjournal.lua).
--
-- ⚠️ Regenerate after ANY change to the quest scripts, and after any change to the parser in
-- quest_difficulty.pl -- the two share their hand-in regexes and silently describe different quest
-- sets if one is updated alone.
-- ⚠️ Item NAMES are deliberately NOT here. The module resolves them with eq.get_item_name(id) at
-- display time, out of shared memory: a baked copy cost 192 KB and could go stale against the DB.
-- ⚠️ `lvl` is the LOWEST level an npc of that name appears at, for sorting. It is never a gate.
--
--   M.quests[i] = { id=, src=, zone=, zid=, npc=, disp=, lvl=, steps=, items={id,..}, qty={n,..} }
--   qty is present only on TASK entries; absent means one of each.

local M = {}

LHEAD

print $lua "M.quests = {\n";

my $n = 0;
for my $k (@keys) {
    my $q = $quest{$k};
    (my $disp = $q->{npc}) =~ s/_/ /g;
    $disp =~ s/^#//;
    $n++;
    # ⚠️ SCRIPT quests carry no quantities -- `check_turn_in{item1=X, item2=Y}` means one each of two
    # DIFFERENT items -- so `qty` is OMITTED entirely and the reader defaults to 1. Emitting
    # "qty={1,1,1}" on every row cost 31 KB to say nothing.
    printf $lua "  { id=%d, src=\"s\", zone=\"%s\", zid=%d, npc=\"%s\", disp=\"%s\", lvl=%d, steps=%d, items={%s} },\n",
        quest_id("s:$q->{zone}/$q->{npc}"),
        lq($q->{zone}), ($zone_id{ $q->{zone} } // 0), lq($q->{npc}), lq($disp),
        ($npc_level{ $q->{npc} } // 0), $q->{steps}, join(',', @{ $q->{items} });
}

my %zone_name = reverse %zone_id;
my $tn = 0;
for my $tid (sort { $a <=> $b } keys %task) {
    my $t   = $task{$tid};
    my @ids = sort { $a <=> $b } keys %{ $t->{items} };
    next unless @ids;
    # (ids already registered above, before the item-name query)
    $tn++;
    # `tid` is carried so the id-preservation pass can re-identify this task next run. It is the
    # engine's own task id, never shown to a player.
    printf $lua "  { id=%d, src=\"t\", tid=%d, zone=\"%s\", zid=%d, npc=\"\", disp=\"%s\", lvl=%d, steps=%d, items={%s}, qty={%s} },\n",
        quest_id("t:$tid"), $tid,
        lq($zone_name{ $t->{zid} } // ''), $t->{zid}, lq($t->{title}), $t->{lvl}, $t->{steps},
        join(',', @ids), join(',', map { $t->{items}{$_} } @ids);
}
print $lua "}\n\nreturn M\n";
close $lua;

printf "wrote %s\n  quests: %d (of %d scanned)\n  zones : %d\n",
    $OUT, scalar @rows, scalar(keys %quest),
    scalar(keys %{ { map { $quest{$_}{zone} => 1 } @keys } });
printf "wrote %s\n  script quests: %d   task quests: %d   total: %d\n  item names: resolved at runtime (%d)   quest numbers reused: %d   highest number: %d\n", $LUA_OUT, $n, $tn, $n + $tn, 0, $reused, $max_id;
