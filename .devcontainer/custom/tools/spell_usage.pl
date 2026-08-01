#!/usr/bin/perl
# AoTv4 -- classify every spell as REFERENCED or DEAD, to find what is safe to prune.
# =============================================================================================
# READ ONLY. It deletes nothing and changes nothing. The output is a report plus
# custom/spell_usage.csv (spell_id, name, verdict, referenced_by).
#
# WHY. spells_new holds 41,049 rows against a HARD client ceiling of 45000 ids (measured, see
# SPELL_RANKS.md), leaving only 3,951 free. Most of the table is live-EQ rank/revamp bloat that this
# server can never reach. Reclaiming it is the only way to grow the id space -- but a spell that
# LOOKS unused and is not will silently stop casting, with no error anywhere, and `spells_new` is
# shared memory so the breakage appears at the next rebuild rather than at the edit.
#
# ⚠️⚠️ THE ANSWER IS ONLY AS GOOD AS THE REFERENCE LIST. Every miss below becomes a spell wrongly
# reported as dead. Anything added to the schema that can hold a spell id MUST be added here.
#
# ⚠️⚠️ BASE AND LIMIT VALUES CARRY SPELL IDS, and only for particular SPAs. Section 14 records this
# being missed once already: the 50xxx -> 43xxx renumber broke Moonfire and Firefist because their
# trigger refs live in effect_base_value / effect_limit_value and were not renumbered with them.
# Both fields are checked for every SPA in @SPELL_SPAS.
#
# ⚠️ A spell referenced ONLY by another dead spell is still counted as referenced here. Resolving
# that needs a transitive closure; this pass is deliberately conservative and over-counts, because
# over-counting costs id space and under-counting costs content.

use strict;
use warnings;

my $QUESTS = ".devcontainer/repo/quests";
my $OUT    = "custom/spell_usage.csv";
die "run from /src\n" unless -d $QUESTS;

sub mysql {
    my ($sql) = @_;
    open(my $h, "-|", "mysql", "-h127.0.0.1", "-upeq", "-ppeqpass", "peq", "-N", "--batch", "-e", $sql)
        or die "mysql: $!";
    my @r; while (<$h>) { chomp; push @r, [split /\t/, $_, -1] } close $h; return @r;
}

# SPAs whose base OR limit value is a spell id (common/spdat.h line numbers in brackets).
#   85 WeaponProc[1148]  289 CastOnFadeEffect[1352]  323 DefensiveProc[1386]  340 SpellTrigger[1403]
#   374 ApplyEffect[1437]  383 SympatheticProc[1446]  406 CastonNumHitFade[1469]
#   419 AddMeleeProc[1482]  427 SkillProcAttempt[1490]  429 SkillProcSuccess[1492]
my @SPELL_SPAS = (85, 289, 323, 340, 374, 383, 406, 419, 427, 429);

my %ref;   # spell_id => { source => 1 }
sub mark { my ($id, $src) = @_; return unless $id && $id =~ /^\d+$/ && $id > 0; $ref{$id}{$src} = 1 }

# ---------------------------------------------------------------- database references
my %TABLES = (
    'items.click'      => 'SELECT DISTINCT clickeffect  FROM items WHERE clickeffect  > 0',
    'items.proc'       => 'SELECT DISTINCT proceffect   FROM items WHERE proceffect   > 0',
    'items.worn'       => 'SELECT DISTINCT worneffect   FROM items WHERE worneffect   > 0',
    'items.focus'      => 'SELECT DISTINCT focuseffect  FROM items WHERE focuseffect  > 0',
    'items.scroll'     => 'SELECT DISTINCT scrolleffect FROM items WHERE scrolleffect > 0',
    'items.bard'       => 'SELECT DISTINCT bardeffect   FROM items WHERE bardeffect   > 0',
    'npc_spells'       => 'SELECT DISTINCT spellid FROM npc_spells_entries WHERE spellid > 0',
    'npc_proc'         => 'SELECT DISTINCT attack_proc FROM npc_spells WHERE attack_proc > 0
                           UNION SELECT DISTINCT defensive_proc FROM npc_spells WHERE defensive_proc > 0
                           UNION SELECT DISTINCT range_proc FROM npc_spells WHERE range_proc > 0',
    'aa'               => 'SELECT DISTINCT spell FROM aa_ranks WHERE spell > 0',
    'auras'            => 'SELECT DISTINCT spell_id FROM auras WHERE spell_id > 0',
    'blocked'          => 'SELECT DISTINCT spellid FROM blocked_spells WHERE spellid > 0',
    'damageshield'     => 'SELECT DISTINCT spellid FROM damageshieldtypes WHERE spellid > 0',
    'ldon_trap'        => 'SELECT DISTINCT spell_id FROM ldon_trap_templates WHERE spell_id > 0',
    'spell_buckets'    => 'SELECT DISTINCT spell_id FROM spell_buckets WHERE spell_id > 0',
    'spell_globals'    => 'SELECT DISTINCT spellid FROM spell_globals WHERE spellid > 0',
    'merc_buffs'       => 'SELECT DISTINCT SpellId FROM merc_buffs WHERE SpellId > 0',
    # ⚠️ LIVE PLAYER DATA. A spell somebody has scribed, memmed or buffed is in use by definition,
    # whatever the static tables say. Deleting one strands a real character.
    'char_scribed'     => 'SELECT DISTINCT spell_id FROM character_spells WHERE spell_id > 0',
    'char_memmed'      => 'SELECT DISTINCT spell_id FROM character_memmed_spells WHERE spell_id > 0',
    'char_buffs'       => 'SELECT DISTINCT spell_id FROM character_buffs WHERE spell_id > 0',
    'char_auras'       => 'SELECT DISTINCT spell_id FROM character_auras WHERE spell_id > 0',
    'char_pet'         => 'SELECT DISTINCT spell_id FROM character_pet_buffs WHERE spell_id > 0
                           UNION SELECT DISTINCT spell_id FROM character_pet_info WHERE spell_id > 0',
    'recourse'         => 'SELECT DISTINCT RecourseLink FROM spells_new WHERE RecourseLink > 0',
);
# ⚠️⚠️ `aotv4_spell_origin` IS DELIBERATELY NOT IN THAT LIST. It has 40,885 rows -- one per spell --
# because it is a CATALOGUE tagging each spell native/custom, not a record of anything USING a spell.
# Including it (the first version of this tool did, having found it by scanning the schema for
# `spell_id` columns) marked 41,012 of 41,049 spells as referenced and made the whole report
# meaningless. A column named `spell_id` is not evidence of use; check what the table is FOR.
for my $src (sort keys %TABLES) {
    for my $r (mysql($TABLES{$src})) { mark($r->[0], $src) }
}

# base / limit values, only for the SPAs that actually carry a spell id
{
    my $spas = join(',', @SPELL_SPAS);
    my @parts;
    for my $s (1 .. 12) {
        push @parts, "SELECT DISTINCT effect_base_value$s  FROM spells_new WHERE effectid$s IN ($spas) AND effect_base_value$s  > 0";
        push @parts, "SELECT DISTINCT effect_limit_value$s FROM spells_new WHERE effectid$s IN ($spas) AND effect_limit_value$s > 0";
    }
    for my $r (mysql(join(' UNION ', @parts))) { mark($r->[0], 'spa_trigger') }
}

# ---------------------------------------------------------------- file references
# ⚠️ Quest scripts and our generated Lua pools cast spells by literal id. A spell offered by the
# level-up picker is obviously in use even though nothing in the schema points at it.
my @scan;
sub walk {
    my ($dir) = @_;
    opendir(my $d, $dir) or return;
    for my $f (sort readdir $d) {
        next if $f =~ /^\./;
        my $p = "$dir/$f";
        if (-d $p) { walk($p) } elsif ($f =~ /\.(lua|pl)$/) { push @scan, $p }
    }
    closedir $d;
}
walk($QUESTS);
for my $p (@scan) {
    open(my $fh, '<', $p) or next; local $/; my $src = <$fh>; close $fh;
    my $tag = ($p =~ /lua_modules/) ? 'lua_module' : 'quest_script';
    # Any 2-6 digit literal in a spell-ish call. Deliberately loose: a false REFERENCE is cheap,
    # a false DEAD is not.
    while ($src =~ /(?:SpellFinished|ApplySpellBuff|CastSpell|cast_spell|castspell|SpellEffect|spell_id|spellid|BuffFadeBySpellID|FindBuff|self_cast|AddSpellBonus)\D{0,20}(\d{2,6})/gi) {
        mark($1, $tag);
    }
}

# ---------------------------------------------------------------- classify
my (%name, %lvl, %id_ok);
for my $r (mysql('SELECT id, name, classes8 FROM spells_new')) {
    $id_ok{$r->[0]} = 1; $name{$r->[0]} = $r->[1]; $lvl{$r->[0]} = $r->[2];
}

my (%bucket, @dead);
for my $id (sort { $a <=> $b } keys %id_ok) {
    my @srcs = sort keys %{ $ref{$id} || {} };
    my $b;
    if (@srcs)                                   { $b = 'referenced' }
    elsif ($id < 10000 && $lvl{$id} >= 1 && $lvl{$id} <= 100) { $b = 'in the offerable pool' }
    else                                         { $b = 'DEAD'; push @dead, $id }
    $bucket{$b}++;
    push @dead, () if 0;
}

mkdir 'custom' unless -d 'custom';
open(my $csv, '>', $OUT) or die "write $OUT: $!";
sub q_ { my $s = shift // ''; $s =~ s/"/""/g; return '"' . $s . '"' }
print $csv "spell_id,name,learn_level,verdict,referenced_by\n";
for my $id (sort { $a <=> $b } keys %id_ok) {
    my @srcs = sort keys %{ $ref{$id} || {} };
    my $v = @srcs ? 'referenced'
          : ($id < 10000 && $lvl{$id} >= 1 && $lvl{$id} <= 100) ? 'pool' : 'DEAD';
    print $csv join(',', $id, q_($name{$id}), $lvl{$id}, $v, q_(join(';', @srcs))), "\n";
}
close $csv;

printf "spells_new rows: %d\n", scalar keys %id_ok;
printf "  %-24s %6d\n", $_, $bucket{$_} for sort keys %bucket;
printf "\nDEAD by id band:\n";
my %band;
$band{ $_ < 10000 ? 'below 10000' : ($_ < 40000 ? '10000-39999' : '40000+') }++ for @dead;
printf "  %-14s %6d\n", $_, $band{$_} for sort keys %band;
printf "\nwrote %s\n", $OUT;

# ---------------------------------------------------------------- "what if we stripped weapon procs?"
# How many spells are held alive ONLY by items.proc -- i.e. would become dead if every weapon proc
# were removed. Anything also referenced elsewhere survives regardless, so this is the true marginal
# gain, not the raw proc count.
my (%proc_only, $proc_total);
for my $id (keys %ref) {
    my @s = keys %{ $ref{$id} };
    next unless grep { $_ eq 'items.proc' } @s;
    $proc_total++;
    $proc_only{$id} = 1 if @s == 1;
}
printf "\nweapon procs:\n";
printf "  spells referenced by items.proc at all : %d\n", $proc_total // 0;
printf "  ...of those, referenced ONLY by a proc : %d   <- freed if all weapon procs were stripped\n",
    scalar keys %proc_only;
printf "  ...and how many of those sit below the 45000 client ceiling: %d\n",
    scalar grep { $_ < 45000 } keys %proc_only;
