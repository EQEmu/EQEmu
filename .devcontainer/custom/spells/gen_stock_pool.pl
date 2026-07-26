#!/usr/bin/perl
# gen_stock_pool.pl -- regenerate the level-up reward pool from the STOCK EQ spell set.
# =============================================================================================
# 2026-07-26: the 113-spell custom set (43000-43149) turned out to be mostly redundant with
# native spells, so the picker goes back to the original setup described in CLAUDE.md section 5:
# draw from spells_new where id < 10000, indexed by classes8 (the spell's lowest learn level).
#
# The genuinely UNIQUE customs are kept and appended into the same pool: the ones whose behaviour
# lives in Lua (quests/global/spells/43xxx.lua, aotv4_summon_table, aotv4_reactions) rather than in
# effect slots, because no stock spell does what they do.  They are identified by having every
# effect slot empty (254) -- that is exactly the Lua-driven set.
#
# NOT touched: the class auras (43500+), which are achievement rewards, and the pet summon spells
# (43200+), which the kept summon abilities call into.
#
# Writes four generated files that spell_choice.lua consumes:
#   spell_pool.lua      pool[learn_level] = { {id=, name=}, ... }
#   spell_icons.lua     id -> spellbook icon index (the picker window shows it)
#   spell_desc.lua      id -> description text (sent as SPELLDESCDATA)
#   spell_blacklist.lua { [id]=true } never offered -- travel, rez, enchant materials, LDoN junk
#
# The blacklist MUST be regenerated with the pool. It was emptied when the custom set took over
# ("empty by design: the pool is a hand-authored custom set"), so re-pointing at the stock set
# without rebuilding it silently reopens every port, rez and Enchant-material spell to the picker.
#
#   perl .devcontainer/custom/spells/gen_stock_pool.pl
#
# Lua modules are require()d once per zone process, so a zone restart is needed to pick these up
# (#reloadquest does NOT reload required modules -- CLAUDE.md section 10).

use strict;
use warnings;

my $DIR = "/src/.devcontainer/repo/quests/lua_modules";

# ---------------------------------------------------------------- helpers
sub q_lua {                     # escape a value for a double-quoted Lua string
    my $s = shift // '';
    $s =~ s/\\/\\\\/g;
    $s =~ s/"/\\"/g;
    $s =~ s/[\r\n]+/ /g;
    return $s;
}

# A literal % is genuinely dangerous here: Client::Message is printf-style, so a stray % in a
# description is consumed as a format token -- it renders as garbage in the spellbook (CLAUDE.md
# section 14) and can eat the following bytes on the way out. Stock descriptions are full of them.
sub sanitize_desc {
    my $s = shift // '';
    $s =~ s/[\r\n]+/ /g;
    $s =~ s/\s+/ /g;
    $s =~ s/\s*%/ percent/g;    # "10%" -> "10 percent"
    $s =~ s/[\^|]/ /g;          # ^ and | are the wire-format separators
    $s =~ s/^\s+|\s+$//g;
    return $s;
}

sub mysql {
    my $sql = shift;
    open(my $h, "-|", "mysql", "-h127.0.0.1", "-upeq", "-ppeqpass", "peq", "-N", "--batch", "-e", $sql)
        or die "mysql: $!";
    my @rows;
    while (my $l = <$h>) { chomp $l; push @rows, [split(/\t/, $l, -1)] if length $l; }
    close $h;
    return @rows;
}

# ---------------------------------------------------------------- source data
# Stock pool: the same filter the original setup used. classes8 is the lowest level any class can
# learn the spell at, which doubles as the pool index.
my @stock = mysql(q{
    SELECT s.id, s.name, s.classes8, s.new_icon, IFNULL(d.value,'')
    FROM spells_new s LEFT JOIN db_str d ON d.id = s.descnum AND d.type = 6
    WHERE s.id < 10000
      AND s.name NOT LIKE '% Rk. %'
      AND s.classes8 BETWEEN 1 AND 100
    ORDER BY s.classes8, s.id});

# Kept customs, two bands:
#   43000-43149 with every effect slot empty -- the Lua-driven abilities. Their effect-slot
#                siblings in the same range are the redundant ones and stay unoffered.
#   43300-43349 -- NEW custom spells written to fill gaps in native lines (e.g. the low-level
#                Skin of the Reptile tiers, custom/sql/aotv4_reptile_line.sql). These DO use effect
#                slots, so they need their own band rather than the empty-slot test. Put helper /
#                trigger spells at 43350+ so they are never offered.
my @custom = mysql(q{
    SELECT s.id, s.name, s.classes8, s.new_icon, IFNULL(d.value,'')
    FROM spells_new s LEFT JOIN db_str d ON d.id = s.descnum AND d.type = 6
    WHERE s.classes8 BETWEEN 1 AND 100
      AND ( (s.id BETWEEN 43000 AND 43149
             AND s.effectid1 = 254 AND s.effectid2 = 254 AND s.effectid3 = 254)
         OR  s.id BETWEEN 43300 AND 43349 )
    ORDER BY s.classes8, s.id});

# ---------------------------------------------------------------- blacklist
# Categories of native spell that must never be offered as a level-up reward. Every SPA number
# below was read out of THIS database rather than from memory (e.g. SELECT effectid1 FROM
# spells_new WHERE name='Gate') so the constants match what the server actually runs.
my @RULES = (
    # ⚠️ Do NOT key travel off `teleport_zone <> ''`. That column is not "destination zone", it is
    # "names an NPC or a zone": every pet/familiar/warder/Eye-of-Zomm spell puts its SUMMON TYPE
    # there (SPA 33/71/152 pets, 106 warders, 108 familiars, 67 eye). Using it pruned 95 stock pet
    # spells -- every Magician elemental, Necromancer pet, Shaman/Beastlord warder and Enchanter
    # animation -- out of the reward pool. The travel SPAs below are the reliable signal, and no
    # travel spell lacks one.
    ["travel",  q{targettype = 3
                  OR 25  IN (effectid1,effectid2,effectid3,effectid4,effectid5,effectid6)   /* Bind Affinity */
                  OR 26  IN (effectid1,effectid2,effectid3,effectid4,effectid5,effectid6)   /* Gate          */
                  OR 83  IN (effectid1,effectid2,effectid3,effectid4,effectid5,effectid6)   /* Teleport      */
                  OR 88  IN (effectid1,effectid2,effectid3,effectid4,effectid5,effectid6)   /* Evacuate      */
                  OR 104 IN (effectid1,effectid2,effectid3,effectid4,effectid5,effectid6)}, # Translocate
                "ports, gates, binds, evacs -- they also defeat region locking"],
    ["enchant", q{name LIKE 'Enchant %' OR name LIKE 'Mass Enchant %'},
                "tradeskill material conversion, useless as a reward"],
    ["rez",     q{81 IN (effectid1,effectid2,effectid3)},
                "resurrection"],
    ["curecurse", q{(effectid1 = 116 AND effect_base_value1 < 0)
                    OR (effectid2 = 116 AND effect_base_value2 < 0)
                    OR (effectid3 = 116 AND effect_base_value3 < 0)},
                "curse-counter cures"],
    ["ldon",    q{targettype = 34},
                "LDoN dungeon-object appraise/disarm/unlock"],
    ["corpse",  q{91 IN (effectid1,effectid2,effectid3)},
                "summon corpse"],
);

my (%black, %black_why, %cat_n);
for my $r (@RULES) {
    my ($cat, $where, $note) = @$r;
    for my $row (mysql("SELECT id FROM spells_new WHERE id < 10000 AND classes8 BETWEEN 1 AND 100 AND ($where)")) {
        my $id = $row->[0];
        $cat_n{$cat}++ unless $black{$id};      # count each spell once, under its first matching rule
        next if $black{$id};
        $black{$id}     = 1;
        $black_why{$id} = $cat;
    }
}

my (%pool, %icons, %descs, $n_stock, $n_custom);
for my $r (@stock, @custom) {
    my ($id, $name, $lvl, $icon, $desc) = @$r;
    push @{ $pool{$lvl} }, { id => $id, name => $name };
    $icons{$id} = $icon if $icon && $icon > 0;
    my $d = sanitize_desc($desc);
    $descs{$id} = $d if length $d;
    $id >= 43000 ? $n_custom++ : $n_stock++;
}

# ---------------------------------------------------------------- spell_pool.lua
open(my $p, '>', "$DIR/spell_pool.lua") or die $!;
print $p <<'HDR';
-- AUTO-GENERATED by .devcontainer/custom/spells/gen_stock_pool.pl -- do not hand-edit.
-- pool[learn_level] = { {id=, name=}, ... }
--
-- Drawn from the STOCK EQ spell set (id < 10000, indexed by classes8 = lowest learn level), which
-- is the original setup: the 113-spell custom set that briefly replaced it was mostly redundant
-- with native spells. The UNIQUE customs are kept and appear here alongside the stock ones -- the
-- Lua-driven abilities (quests/global/spells/43xxx.lua) that no native spell replicates.
--
-- Runtime filters still apply on top of this (spell_choice.gather_candidates): spell_blacklist.lua,
-- already-known, and the "Enchant %s" name reject.
return {
HDR
for my $lvl (sort { $a <=> $b } keys %pool) {
    my @e = map { sprintf('{id=%d,name="%s"}', $_->{id}, q_lua($_->{name})) } @{ $pool{$lvl} };
    print $p "\t[$lvl]={ " . join(", ", @e) . " },\n";
}
print $p "}\n";
close $p;

# ---------------------------------------------------------------- spell_icons.lua
open(my $i, '>', "$DIR/spell_icons.lua") or die $!;
print $i "-- AUTO-GENERATED by .devcontainer/custom/spells/gen_stock_pool.pl -- do not hand-edit.\n";
print $i "-- id -> spellbook icon index, for the level-up reward window.\n";
print $i "return {\n";
print $i "\t[$_]=$icons{$_},\n" for sort { $a <=> $b } keys %icons;
print $i "}\n";
close $i;

# ---------------------------------------------------------------- spell_desc.lua
open(my $d, '>', "$DIR/spell_desc.lua") or die $!;
print $d "-- AUTO-GENERATED by .devcontainer/custom/spells/gen_stock_pool.pl -- do not hand-edit.\n";
print $d "-- id -> description (db_str type 6), sent to the picker as SPELLDESCDATA.\n";
print $d "-- Literal percent signs are spelled out: Client::Message is printf-style, so a stray %\n";
print $d "-- is eaten as a format token and renders as garbage.\n";
print $d "return {\n";
printf $d "\t[%d]=\"%s\",\n", $_, q_lua($descs{$_}) for sort { $a <=> $b } keys %descs;
print $d "}\n";
close $d;

# ---------------------------------------------------------------- spell_blacklist.lua
open(my $bl, '>', "$DIR/spell_blacklist.lua") or die $!;
print $bl "-- AUTO-GENERATED by .devcontainer/custom/spells/gen_stock_pool.pl -- do not hand-edit.\n";
print $bl "-- { [id]=true } -- spells that are NEVER offered as a level-up reward. Applied at runtime\n";
print $bl "-- in spell_choice.gather_candidates, so the pool itself still lists them.\n--\n";
printf $bl "-- %-10s %5d  %s\n", $_->[0], ($cat_n{$_->[0]} // 0), $_->[2] for @RULES;
print $bl "return {\n";
printf $bl "\t[%d]=true,  -- %s\n", $_, $black_why{$_} for sort { $a <=> $b } keys %black;
print $bl "}\n";
close $bl;

printf "regenerated from the stock spell set:\n";
printf "  spell_pool.lua   %d levels, %d stock + %d unique custom = %d spells\n",
       scalar(keys %pool), $n_stock, $n_custom, $n_stock + $n_custom;
printf "  spell_icons.lua  %d icons\n", scalar(keys %icons);
printf "  spell_desc.lua   %d descriptions\n", scalar(keys %descs);
printf "  spell_blacklist.lua %d pruned:\n", scalar(keys %black);
printf "      %-10s %5d  %s\n", $_->[0], ($cat_n{$_->[0]} // 0), $_->[2] for @RULES;
printf "  => %d spells actually offerable\n",
       $n_stock + $n_custom - scalar(grep { $black{$_} } keys %black);
