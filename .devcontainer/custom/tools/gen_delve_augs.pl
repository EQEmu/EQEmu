#!/usr/bin/perl
# =============================================================================================
# AoTv4 -- generate the Delve augment set (three tiers, combined in the Refining Crucible)
#                                                                                     2026-07-31
#   perl .devcontainer/custom/tools/gen_delve_augs.pl > .devcontainer/custom/sql/aotv4_delve_augs.sql
#
# Writes SQL only; it touches no database itself. The delve chest hands out a tier 1/2/3 augment
# (aotv4_dungeon.lua) and FOUR OF A TIER COMBINE INTO ONE RANDOM AUGMENT OF THE NEXT TIER in the
# Refining Crucible, item 2000060 (zone/tradeskills.cpp, AoTv4RefineCombine).
#
# ⚠️⚠️ THESE ARE ORDINARY ITEMS -- `evoitem` IS 0 AND THAT IS DELIBERATE. An earlier version made
# them native EVOLVING items that grew on delve score, and it was abandoned for a concrete reason:
# an augment can never appear in the client's evolving-item window. That window is enumerated CLIENT
# side inside eqgame.exe (there is no enumeration packet anywhere -- the only server to client evolve
# message is OP_EvolveItem UPDATE_ITEMS, keyed by a unique id the client must already know), and its
# walk does not descend into augment sockets. So the charm would have shown in the native window
# while augments needed a second, custom one. Combining sidesteps all of it: the sigil stays the only
# evolving item on the server, and there is exactly ONE evolve window.
# ⚠️ Do not "restore" evoitem = 1 here. It would put these back in a window that cannot list them,
# and re-open the two-window problem this design exists to close.
#
# ⚠️⚠️ THE RNG IS A FIXED SEED LCG, ON PURPOSE, AND MUST STAY THAT WAY. Re-running this generator
# has to produce byte-identical SQL, because these ids are LIVE ON PLAYER CHARACTERS: an augment is
# a row in `items`, so a regen with a different seed would silently rewrite the stats of augments
# people are already wearing. Perl's own rand() is NOT used -- it is seeded per process and is not
# guaranteed stable across perl builds, which is exactly the property we cannot have here.
# ⚠️⚠️ `sort` EVERY `keys` WHOSE ORDER CAN REACH THE OUTPUT. Perl randomises hash key order per
# process; a bare `keys` in the weight distributor made this non-deterministic once already (three
# runs, three different files). Verify after any change by regenerating and diffing.
#
# ---------------------------------------------------------------------------------------------
# THE WEIGHT SYSTEM
#
# Every augment carries a stat WEIGHT: tier 1 = 2, tier 2 = 4, tier 3 = 8.
#   tier 1 is a SINGLE stat, and the set is exhaustive -- one variant per stat line, so the chest
#   can never roll a gap.
#   tiers 2 and 3 spread their weight over several lines and are randomly rolled, which is where the
#   "no two drops alike" feel comes from.
#
# ⚠️ HP / MANA / ENDURANCE PAY $BIG_PER_WEIGHT (10) POINTS PER WEIGHT, everything else 1. So the
# three tiers read as 20 / 40 / 80 on those lines and 2 / 4 / 8 on everything else. Without the
# conversion a "2 hp" tier 1 is a dead roll -- the sigil alone gives 100 hp.
# ⚠️ $CAP is per LINE and in WEIGHT. At these tier weights it can never actually bind (8 < 16); it is
# retained so that raising a tier weight later cannot silently produce a single monster line.
# =============================================================================================

use strict;
use warnings;

# ---------------------------------------------------------------- tunables
my $SEED           = 20260731;
my $T2_VARIANTS    = 100;     # rolled variants for tier 2
my $T3_VARIANTS    = 200;     # rolled variants for tier 3
my $CAP            = 16;      # max WEIGHT on any one line
my $BIG_PER_WEIGHT = 10;      # hp/mana/endurance points per weight
my $HEROIC_WEIGHT  = 3;       # weight per heroic point
my $HEROIC_TIER    = 3;       # heroic can only appear at this tier or above
my $HEROIC_CHANCE  = 15;      # percent of eligible rolls that carry heroic
my $HEROIC_MAX     = 2;       # 2 * 3 = 6 weight, most of a tier 3
my $ID_BASE        = 147600;  # sigil owns 147500-147509
# ⚠️ The DELETE below clears this WHOLE band, not just the rows generated now. An earlier version of
# this set ran to 148007 with a different layout; deleting only the current range would strand its
# tail as orphaned items that nothing references and nothing cleans up.
my $ID_BAND_END    = 148199;
my $TEMPLATE       = 71402;   # Stone of the Defensive Arts -- a plain stat aug, no click/charges
my $ICON           = 1439;
my $AUGTYPE        = 255;     # slot types 1-8; type 7 alone covers 33k items, 8 another 8k

# tier => [ weight, how many variants, how many lines to spread over ]
my %TIER = (
    1 => { weight => 2, count => 0,            lines => [1, 1] },   # count 0 = one per stat line
    2 => { weight => 4, count => $T2_VARIANTS, lines => [1, 2] },
    3 => { weight => 8, count => $T3_VARIANTS, lines => [2, 3] },
);

# ⚠️⚠️ EVERY ROLL IN A TIER MUST BE A DISTINCT STAT COMBINATION, and that is enforced by rejection
# below rather than assumed. The first version of this set deduped only NAMES -- when two rolls came
# out stat-identical it appended a roman numeral and kept both -- which left TEN of forty tier 2 rows
# redundant, SIX of them the same CR 4. A quarter of the pool was wasted and Frost was six times more
# likely to appear than any other single-stat tier 2 roll.
#
# ⚠️ The space has to be big enough for the count, or generation would spin forever:
#   tier 2, weight 4 over 1-2 lines:  16 single-line + 120 pairs x 3 splits = ~376
#   tier 3, weight 8 over 2-3 lines:  120 pairs x 7 + 560 triples x 21     = ~12,600
# So 100 of 376 and 200 of 12,600 are both comfortable. Raising a count toward its ceiling makes
# rejection slow and eventually impossible -- $MAX_REROLLS turns that into a loud failure rather than
# a hang.
my $MAX_REROLLS = 20000;

# ---------------------------------------------------------------- the stat lines
# key, items column, points per weight, heroic column (undef = no heroic form), name suffix
my @LINES = (
    [ 'str',  'astr',  1, 'heroic_str', 'Might'        ],
    [ 'sta',  'asta',  1, 'heroic_sta', 'Vigor'        ],
    [ 'agi',  'aagi',  1, 'heroic_agi', 'Grace'        ],
    [ 'dex',  'adex',  1, 'heroic_dex', 'Precision'    ],
    [ 'int',  'aint',  1, 'heroic_int', 'Intellect'    ],
    [ 'wis',  'awis',  1, 'heroic_wis', 'Insight'      ],
    [ 'cha',  'acha',  1, 'heroic_cha', 'Presence'     ],
    [ 'ac',   'ac',    1, undef,        'the Bulwark'  ],
    [ 'hp',   'hp',   10, undef,        'Life'         ],
    [ 'mana', 'mana', 10, undef,        'the Mind'     ],
    [ 'end',  'endur',10, undef,        'Stamina'      ],
    [ 'mr',   'mr',    1, 'heroic_mr',  'Warding'      ],
    [ 'fr',   'fr',    1, 'heroic_fr',  'Embers'       ],
    [ 'cr',   'cr',    1, 'heroic_cr',  'Frost'        ],
    [ 'dr',   'dr',    1, 'heroic_dr',  'the Blight'   ],
    [ 'pr',   'pr',    1, 'heroic_pr',  'Venom'        ],
);

my @MATERIAL = ( undef, "Delver's Shard", "Delver's Gem", "Delver's Prism" );

# every stat column this generator ever writes -- each row zeroes ALL of them and then sets its own
my @ALL_COLS = ( (map { $_->[1] } @LINES), (grep { defined } map { $_->[3] } @LINES) );

# ---------------------------------------------------------------- deterministic RNG (see header)
my $state = $SEED;
sub rnd  { $state = ($state * 1103515245 + 12345) % 2147483648; return $state / 2147483648; }
sub rint { my ($lo, $hi) = @_; return $lo if $hi <= $lo; return $lo + int(rnd() * ($hi - $lo + 1)); }

# ---------------------------------------------------------------- roll one augment
sub roll {
    my ($tier, $fixed_line) = @_;
    my $weight = $TIER{$tier}{weight};

    if (defined $fixed_line) {   # tier 1: exhaustive, one variant per stat line
        return { lines => { $LINES[$fixed_line][0] => $weight }, heroic => undef, heroic_pts => 0 };
    }

    my ($lo, $hi) = @{ $TIER{$tier}{lines} };
    my $n = rint($lo, $hi);
    $n = $weight if $n > $weight;          # every line needs at least 1 weight
    $n = scalar(@LINES) if $n > scalar(@LINES);

    my @pool = (0 .. $#LINES);
    my @pick;
    for (1 .. $n) { push @pick, splice(@pool, rint(0, $#pool), 1); }

    # everyone starts at 1, the rest goes out one unit at a time to a line still under the cap
    my %w = map { $LINES[$_][0] => 1 } @pick;
    my $left = $weight - $n;
    while ($left > 0) {
        my @open = sort grep { $w{$_} < $CAP } keys %w;   # sort: see the determinism note in the header
        last unless @open;
        $w{ $open[ rint(0, $#open) ] }++;
        $left--;
    }

    my ($hkey, $hpts) = (undef, 0);
    if ($tier >= $HEROIC_TIER && rint(1, 100) <= $HEROIC_CHANCE) {
        my %hero = map { $_->[0] => 1 } grep { defined $_->[3] } @LINES;
        my @elig = sort grep { $hero{$_} && $w{$_} >= $HEROIC_WEIGHT } keys %w;
        if (@elig) {
            $hkey = $elig[ rint(0, $#elig) ];
            $hpts = int($w{$hkey} / $HEROIC_WEIGHT);
            $hpts = $HEROIC_MAX if $hpts > $HEROIC_MAX;
            $w{$hkey} -= $hpts * $HEROIC_WEIGHT;   # leftover stays as the ordinary stat
            delete $w{$hkey} if $w{$hkey} <= 0;
        }
    }

    return { lines => \%w, heroic => $hkey, heroic_pts => $hpts };
}

# ---------------------------------------------------------------- naming
my %SUFFIX = map { $_->[0] => $_->[4] } @LINES;
my %seen_name;
my $NAME_MAX = 64;   # ⚠️ items.Name is varchar(64); longer is silently TRUNCATED, which can also
                     #    collapse two distinct names into one.

sub roman {
    my $n = shift;
    my @map = ( [50,'L'], [40,'XL'], [10,'X'], [9,'IX'], [5,'V'], [4,'IV'], [1,'I'] );
    my $s = '';
    for my $m (@map) { while ($n >= $m->[0]) { $s .= $m->[1]; $n -= $m->[0]; } }
    return $s;
}

# ⚠️ Two augments of the same tier with DIFFERENT stats must not share a name -- comparing two drops
# called the same thing is exactly when the name has to distinguish them.
sub name_for {
    my ($tier, $r) = @_;
    my @by = sort { $r->{lines}{$b} <=> $r->{lines}{$a} || $a cmp $b } keys %{ $r->{lines} };
    push @by, $r->{heroic} if $r->{heroic} && !exists $r->{lines}{ $r->{heroic} };

    my $base = "$MATERIAL[$tier] of ";
    my @cands = ( $base . $SUFFIX{ $by[0] } );
    push @cands, $base . $SUFFIX{ $by[0] } . ' and ' . $SUFFIX{ $by[1] } if @by > 1;
    push @cands, $base . $SUFFIX{ $by[0] } . ', ' . $SUFFIX{ $by[1] } . ' and ' . $SUFFIX{ $by[2] }
        if @by > 2;

    for my $c (@cands) {
        next if $seen_name{$c} || length($c) > $NAME_MAX;
        $seen_name{$c} = 1;
        return $c;
    }
    for my $n (2 .. 99) {
        my $c = $cands[0] . ' ' . roman($n);
        next if $seen_name{$c} || length($c) > $NAME_MAX;
        $seen_name{$c} = 1;
        return $c;
    }
    die "could not build a unique name under $NAME_MAX chars for tier $tier\n";
}

sub sqlstr { my $s = shift; $s =~ s/'/''/g; return "'$s'"; }

# ---------------------------------------------------------------- emit
my %PER  = map { $_->[0] => $_->[2] } @LINES;
my %COL  = map { $_->[0] => $_->[1] } @LINES;
my %HCOL = map { $_->[0] => $_->[3] } @LINES;

# The items-table columns a roll writes, as { column => value }.
sub cols_for {
    my $r = shift;
    my %cols;
    for my $k (keys %{ $r->{lines} }) { $cols{ $COL{$k} } = $r->{lines}{$k} * $PER{$k}; }
    $cols{ $HCOL{ $r->{heroic} } } = $r->{heroic_pts} if $r->{heroic};
    return \%cols;
}

# ⚠️ `sort` here is what makes the signature order-independent, so two rolls that set the same columns
# to the same values collide no matter which order the generator happened to pick the lines in.
sub signature {
    my $cols = shift;
    return join(',', map { "$_=$cols->{$_}" } sort keys %$cols);
}

my (@rows, %block);
my $id = $ID_BASE;

for my $tier (1, 2, 3) {
    my $first = $id;
    my $count = $TIER{$tier}{count} || scalar(@LINES);
    my %seen_sig;    # per tier: a tier 2 and a tier 3 roll may legitimately look alike

    for my $i (0 .. $count - 1) {
        my ($r, $cols, $sig);
        my $tries = 0;
        while (1) {
            $r    = ($TIER{$tier}{count} == 0) ? roll($tier, $i) : roll($tier);
            $cols = cols_for($r);
            $sig  = signature($cols);
            last if !$seen_sig{$sig};
            if (++$tries > $MAX_REROLLS) {
                die "tier $tier: could not find a distinct roll after $MAX_REROLLS tries at variant "
                  . ($i + 1) . " of $count -- the requested count is too close to the space of\n"
                  . "possible combinations for weight $TIER{$tier}{weight} over "
                  . "$TIER{$tier}{lines}[0]-$TIER{$tier}{lines}[1] lines. Lower the count or widen the\n"
                  . "line range.\n";
            }
        }
        $seen_sig{$sig} = 1;

        push @rows, [ $id++, $tier, name_for($tier, $r), $cols, $TIER{$tier}{weight} ];
    }
    $block{$tier} = [ $first, $id - 1 ];
}

my $total = scalar @rows;
my $last  = $ID_BASE + $total - 1;

print <<"HDR";
-- =============================================================================================
-- AoTv4 -- Delve augments: three tiers, combined in the Refining Crucible
--                                                              GENERATED, DO NOT HAND EDIT
--   source: .devcontainer/custom/tools/gen_delve_augs.pl   (fixed seed $SEED)
--
-- $total item rows, ids $ID_BASE..$last, in three CONTIGUOUS blocks:
--   tier 1  $block{1}[0]..$block{1}[1]   weight 2   single stat, one variant per stat line
--   tier 2  $block{2}[0]..$block{2}[1]   weight 4   randomly rolled over 1-2 lines
--   tier 3  $block{3}[0]..$block{3}[1]   weight 8   randomly rolled over 2-3 lines
--
-- ⚠️⚠️ THOSE BLOCKS ARE LOAD BEARING AND ARE MIRRORED IN C++. zone/aotv4_tiers.h holds the same
--    boundaries so AoTv4RefineCombine can tell an augment's tier from its id and pick a random
--    member of the next tier's block. CHANGE THE COUNTS HERE AND THAT HEADER MUST CHANGE TOO --
--    nothing will fail to compile, the crucible will just start producing the wrong items.
--
-- ⚠️ `evoitem` is 0: these are ORDINARY items, not native evolving items. See the script header for
--    why that is deliberate and must not be "restored".
-- ⚠️ `items` IS IN SHARED MEMORY: world down, ./shared_memory, restart. A zone restart is NOT enough.
-- ⚠️ Cloned via a temp table from $TEMPLATE so all ~250 item columns stay valid -- never hand list
--    item columns. Every row zeroes every stat column this generator knows about before setting its
--    own, so no row can inherit a stat from the template or from the row before it.
-- ⚠️ nodrop/norent are INVERTED in this schema: nodrop = 0 is No Drop, norent = 1 is permanent.
--    (Cloth Cap and Rusty Long Sword are both nodrop 1 / norent 1.)
-- =============================================================================================

DROP TEMPORARY TABLE IF EXISTS tmp_aug;
CREATE TEMPORARY TABLE tmp_aug AS SELECT * FROM items WHERE id = $TEMPLATE;

-- ⚠️ Clears the WHOLE reserved band, not just the rows written below: earlier versions of this set
-- had a different layout reaching 148007, and deleting only the current range would leave their tail
-- behind as orphaned items.
DELETE FROM items WHERE id BETWEEN $ID_BASE AND $ID_BAND_END;
-- the old evolving version of this set had rows here; they are no longer used by anything
DELETE FROM items_evolving_details WHERE item_evo_id IN (2001, 2002, 2003);

-- fields shared by every row in the set
UPDATE tmp_aug SET
    augtype = $AUGTYPE, augrestrict = 0, augdistiller = 0,
    icon = $ICON, itemtype = 54, slots = 2097150,
    classes = 65535, races = 65535, loregroup = 0, magic = 1,
    nodrop = 0, norent = 1, price = 0, sellrate = 0, ldonsellbackrate = 0,
    reclevel = 0, reqlevel = 0, stacksize = 1,
    lore = '', evoitem = 0, evoid = 0, evolvinglevel = 0, evomax = 0;
HDR

for my $r (@rows) {
    my ($rid, $tier, $nm, $cols, $w) = @$r;
    my @sets = ( "id = $rid", "Name = " . sqlstr($nm) );
    push @sets, map { "$_ = 0" } @ALL_COLS;                       # zero everything first...
    push @sets, map { "$_ = $cols->{$_}" } sort keys %$cols;      # ...then this roll's lines
    my $desc = join(', ', map { "$_ $cols->{$_}" } sort keys %$cols);
    print "-- T$tier  weight $w  ($desc)\n";
    print "UPDATE tmp_aug SET " . join(', ', @sets) . ";\n";
    print "INSERT INTO items SELECT * FROM tmp_aug;\n";
}

print "\nDROP TEMPORARY TABLE tmp_aug;\n\n";
print <<"FTR";
-- ---------------------------------------------------------------- verify
SELECT 'aug items' AS what, COUNT(*) n FROM items WHERE id BETWEEN $ID_BASE AND $last
UNION ALL SELECT 'tier 1', COUNT(*) FROM items WHERE id BETWEEN $block{1}[0] AND $block{1}[1]
UNION ALL SELECT 'tier 2', COUNT(*) FROM items WHERE id BETWEEN $block{2}[0] AND $block{2}[1]
UNION ALL SELECT 'tier 3', COUNT(*) FROM items WHERE id BETWEEN $block{3}[0] AND $block{3}[1]
UNION ALL SELECT 'still flagged evolving', COUNT(*) FROM items WHERE id BETWEEN $ID_BASE AND $last AND evoitem <> 0;
-- expected: $total / @{[ scalar @LINES ]} / $T2_VARIANTS / $T3_VARIANTS / 0
FTR
