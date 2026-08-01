#!/usr/bin/perl
# List the filenames inside an EverQuest .s3d (PFS) archive.
# =============================================================================================
#   perl s3d_list.pl <file.s3d> [pattern]
#
# Written to answer one question that nothing else could: which player races actually have ROBE
# textures. `strings` cannot -- a PFS filename table is zlib-deflated, so the archive reads as noise.
#
# FORMAT (little endian throughout):
#   0x00  uint32  offset of the directory
#   0x04  char[4] "PFS " magic
#   0x08  uint32  version
#   at directory offset:
#     uint32 count, then `count` entries of 12 bytes: crc, data offset, inflated size
#   Each data blob is a chain of blocks: uint32 deflated length, uint32 inflated length, then data.
#
# ⚠️⚠️ THE FILENAME TABLE IS THE ENTRY WITH CRC 0x61580AC9, and it is NOT in filename order. Its
# inflated content is: uint32 count, then per file a uint32 length and a NUL-terminated name. The
# other entries carry no name at all -- names exist only in that one blob, which is why a partial
# read of the archive tells you nothing about what is in it.

use strict;
use warnings;
use Compress::Zlib;

my ($path, $pattern) = @ARGV;
die "usage: s3d_list.pl <file.s3d> [pattern]\n" unless $path && -f $path;

open(my $fh, '<:raw', $path) or die "$path: $!";
my $raw = do { local $/; <$fh> };
close $fh;

my ($dir_offset, $magic, $version) = unpack('V a4 V', substr($raw, 0, 12));
die "not a PFS archive (magic '$magic')\n" unless $magic eq 'PFS ';

my $count = unpack('V', substr($raw, $dir_offset, 4));
my @entries;
for my $i (0 .. $count - 1) {
    my ($crc, $off, $size) = unpack('V V V', substr($raw, $dir_offset + 4 + $i * 12, 12));
    push @entries, { crc => $crc, off => $off, size => $size };
}

# Inflate one blob: a chain of (deflated_len, inflated_len, data) blocks.
sub inflate_blob {
    my ($off, $want) = @_;
    my $out = '';
    my $p   = $off;
    while (length($out) < $want) {
        my ($dlen, $ilen) = unpack('V V', substr($raw, $p, 8));
        last unless $dlen;
        my $chunk = uncompress(substr($raw, $p + 8, $dlen));
        last unless defined $chunk;
        $out .= $chunk;
        $p   += 8 + $dlen;
    }
    return $out;
}

# ⚠️ 0x61580AC9 is the filename-table CRC. Without this entry there are no names anywhere.
my ($dir) = grep { $_->{crc} == 0x61580AC9 } @entries;
die "no filename table in $path\n" unless $dir;

my $names_blob = inflate_blob($dir->{off}, $dir->{size});
my $n = unpack('V', substr($names_blob, 0, 4));
my $p = 4;
my @names;
for (1 .. $n) {
    my $len = unpack('V', substr($names_blob, $p, 4));
    $p += 4;
    my $name = substr($names_blob, $p, $len - 1);   # length includes the NUL
    $p += $len;
    push @names, $name;
}

@names = grep { /\Q$pattern\E/i } @names if defined $pattern && $pattern ne '';
print "$_\n" for sort @names;
printf STDERR "%s: %d files%s\n", $path, scalar @names,
    (defined $pattern && $pattern ne '') ? " matching '$pattern'" : '';
