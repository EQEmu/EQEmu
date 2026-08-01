#!/usr/bin/perl
# AoTv4 -- dump EVERY INDIVIDUAL TURN-IN as CSV, for rating quest difficulty.
# =============================================================================================
#   reward, turnin_zone, item_1, item_2, item_3, item_4
#
# One row per HAND-IN, not per NPC. That is the whole point: quest_difficulty.pl and
# gen_quest_catalogue.pl both treat an NPC file as one quest, which merges independent quests that
# happen to share a giver and makes "how hard is this hand-in" unanswerable. Here each
# `check_turn_in` / `check_handin` call is its own row with exactly the items that go in the trade
# window for it.
#
# ⚠️ FOUR ITEM COLUMNS IS NOT ARBITRARY -- the EQ trade window has four slots, so four is the true
# maximum for a single hand-in. The script counts and reports any row that would exceed it rather
# than silently truncating, since that would mean the parser had merged two hand-ins.
#
# ⚠️⚠️ QUANTITY IS EXPRESSED BY REPEATING THE ITEM. The Perl form carries counts
# (`plugin::check_handin(\%itemcount, 1234 => 2)`) and needing two of something is genuinely more
# work than needing one, which is exactly what a difficulty rating wants to see. So `1234 => 2`
# emits item_1=1234, item_2=1234. The Lua form has no counts at all -- `check_turn_in{item1=X,
# item2=Y}` means one each of two DIFFERENT items -- so those are always one apiece.
#
# ⚠️ REWARD is the item id(s) the block hands back (`SummonItem` / `summonitem`), joined with ";".
# It is BLANK when the hand-in pays only experience, coin or faction -- which is common, and is not
# an error. Use --verbose to see the split.
#
# ⚠️⚠️ THE BLOCK BOUNDARY IS A HEURISTIC, and it is the one thing here that can be wrong. A reward
# is attributed to a hand-in if it appears between that `check_turn_in` and the NEXT one (or the
# next function/sub, or EOF). That is exact for the standard if/elseif chain every PEQ script uses,
# but a script that rewards outside the chain, or nests turn-ins inside a loop, can mis-attribute.
# It never invents items -- the REQUIREMENT columns come straight from the call itself and are
# always right; only `reward` is inferred.
#
# Usage:
#   perl .devcontainer/custom/tools/gen_quest_turnins.pl > quest_turnins.csv
#   perl .devcontainer/custom/tools/gen_quest_turnins.pl --names     # append readable name columns
#   perl .devcontainer/custom/tools/gen_quest_turnins.pl --npc       # add the giver as a column
#   perl .devcontainer/custom/tools/gen_quest_turnins.pl --verbose   # stats to stderr

use strict;
use warnings;

my $QUESTS = ".devcontainer/repo/quests";
my ($WANT_NAMES, $WANT_NPC, $VERBOSE) = (0, 0, 0);
for my $a (@ARGV) {
    $WANT_NAMES = 1 if $a eq '--names';
    $WANT_NPC   = 1 if $a eq '--npc';
    $VERBOSE    = 1 if $a eq '--verbose';
}
die "run from /src (no $QUESTS)\n" unless -d $QUESTS;

sub mysql {
    my ($sql) = @_;
    open(my $h, "-|", "mysql", "-h127.0.0.1", "-upeq", "-ppeqpass", "peq", "-N", "--batch", "-e", $sql)
        or die "mysql: $!";
    my @rows;
    while (<$h>) { chomp; push @rows, [split /\t/, $_, -1] }
    close $h;
    return @rows;
}

# ---------------------------------------------------------------- collect the files
my @files;
opendir(my $qd, $QUESTS) or die "opendir $QUESTS: $!";
for my $zone (sort readdir $qd) {
    next if $zone =~ /^\./;
    next unless -d "$QUESTS/$zone";
    next if $zone =~ /^(lua_modules|plugins|global|mods|templates|items|spells|bots|merchants)$/;
    opendir(my $d, "$QUESTS/$zone") or next;
    for my $f (sort readdir $d) {
        next unless $f =~ /\.(lua|pl)$/;
        push @files, [$zone, $f, "$QUESTS/$zone/$f"];
    }
    closedir $d;
}
closedir $qd;

# ---------------------------------------------------------------- parse
my (@rows, %want_names, $n_overflow, $n_no_reward, $n_qty);
$n_overflow = $n_no_reward = $n_qty = 0;

for my $e (@files) {
    my ($zone, $file, $path) = @$e;
    open(my $fh, '<', $path) or next;
    local $/;
    my $src = <$fh>;
    close $fh;

    (my $npc = $file) =~ s/\.(lua|pl)$//;

    # Find every hand-in call and remember WHERE it started, so the reward scan has a boundary.
    my @hits;
    while ($src =~ /check_turn_in\s*\([^,]*,\s*\{([^}]*)\}/g) {
        my ($blk, $end) = ($1, pos($src));
        my @ids = ($blk =~ /item\d+\s*=\s*(\d+)/g);
        push @hits, { pos => $end, items => [@ids] } if @ids;
    }
    while ($src =~ /check_handin\s*\(\s*\\?%\w+\s*,([^)]*)\)/g) {
        my ($blk, $end) = ($1, pos($src));
        my @ids;
        # ⚠️ id => count. Repeat the id `count` times so quantity survives into the CSV.
        while ($blk =~ /(\d+)\s*=>\s*(\d+)/g) {
            my ($id, $qty) = ($1, $2);
            $n_qty++ if $qty > 1;
            push @ids, ($id) x $qty;
        }
        push @hits, { pos => $end, items => [@ids] } if @ids;
    }
    next unless @hits;

    @hits = sort { $a->{pos} <=> $b->{pos} } @hits;

    # Where each block ends: the next hand-in, or the next function/sub, or EOF.
    for my $i (0 .. $#hits) {
        my $start = $hits[$i]{pos};
        my $stop  = ($i < $#hits) ? $hits[$i + 1]{pos} : length($src);
        my $chunk = substr($src, $start, $stop - $start);
        if ($chunk =~ /\n\s*(?:function\s|sub\s)/) {
            $chunk = substr($chunk, 0, $-[0]);
        }

        my @reward;
        while ($chunk =~ /(?:SummonItem|summonitem)\s*\(\s*(\d{2,6})/g) { push @reward, $1 }

        my @items = @{ $hits[$i]{items} };
        if (@items > 4) { $n_overflow++ }
        $n_no_reward++ unless @reward;

        $want_names{$_} = 1 for (@items, @reward);
        push @rows, {
            reward => join(';', @reward),
            zone   => $zone,
            npc    => $npc,
            items  => [ @items[0 .. 3] ],   # pads with undef to exactly four
        };
    }
}

# ---------------------------------------------------------------- names (optional)
my %name;
if ($WANT_NAMES && %want_names) {
    my $in = join(',', sort { $a <=> $b } keys %want_names);
    for my $r (mysql("SELECT id, name FROM items WHERE id IN ($in)")) { $name{ $r->[0] } = $r->[1] }
}

# ⚠️ Item names contain commas and apostrophes ("Bone Chips", "Rodrick's Head"), so every field is
# quoted and internal quotes are doubled -- the RFC 4180 form every spreadsheet reads.
sub csv { my $s = shift; $s = '' unless defined $s; $s =~ s/"/""/g; return '"' . $s . '"' }

my @hdr = ('reward', 'turnin_zone');
push @hdr, 'npc' if $WANT_NPC;
push @hdr, 'item_1', 'item_2', 'item_3', 'item_4';
push @hdr, 'reward_name', 'item_1_name', 'item_2_name', 'item_3_name', 'item_4_name' if $WANT_NAMES;
print join(',', map { csv($_) } @hdr), "\n";

for my $r (@rows) {
    my @out = ($r->{reward}, $r->{zone});
    push @out, $r->{npc} if $WANT_NPC;
    push @out, map { defined $_ ? $_ : '' } @{ $r->{items} };
    if ($WANT_NAMES) {
        push @out, join(';', map { $name{$_} // $_ } split /;/, ($r->{reward} // ''));
        push @out, map { defined $_ ? ($name{$_} // $_) : '' } @{ $r->{items} };
    }
    print join(',', map { csv($_) } @out), "\n";
}

if ($VERBOSE) {
    printf STDERR "turn-ins: %d   across %d scripts\n", scalar @rows, scalar @files;
    printf STDERR "  with an item reward : %d\n", scalar(@rows) - $n_no_reward;
    printf STDERR "  reward is exp/coin  : %d\n", $n_no_reward;
    printf STDERR "  quantity > 1 seen   : %d\n", $n_qty;
    printf STDERR "  MORE THAN 4 ITEMS   : %d  (should be 0 -- the trade window has 4 slots)\n", $n_overflow;
}
