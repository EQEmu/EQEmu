#!/usr/bin/perl

###########################################################
#::: Automatic (Map-to-MMF) Conversion Script
#::: Author: Uleat
#::: Purpose: To convert existing zone maps to memory-mapped files
###########################################################

use strict;
use warnings;
use Config;

print("\n");
print("Zone Map-to-MMF Batch convertor\n");
print("===============================\n");
print("\n");

my $zone;
if ($Config{osname} =~ /freebsd|linux/i) { $zone = "./zone"; }
elsif ($Config{osname} =~ /Win|MS/i) { $zone = "zone.exe"; }
else { die "Unsupported operating system: $Config{osname}\n"; }
print("Operating System is: $Config{osname}\n");
print("\n");

opendir(D, "maps") || die "Can't find directory maps: $!\n";
my @mapfiles = grep { /\.map$/ && !/_lit/ } readdir(D);
closedir(D);

my $failed = 0;
foreach my $mapfile (@mapfiles) {
	print("processing map: '$mapfile'\n");
	# The explicit program and argument list avoid shell interpretation of map names.
	my $status = system { $zone } $zone, "convert_map", $mapfile;
	if ($status == -1) {
		warn "Can't execute $zone for '$mapfile': $!\n";
		$failed = 1;
	}
	elsif ($status & 127) {
		warn "Conversion of '$mapfile' terminated by signal " . ($status & 127) . "\n";
		$failed = 1;
	}
	elsif ($status != 0) {
		warn "Conversion of '$mapfile' failed with exit status " . ($status >> 8) . "\n";
		$failed = 1;
	}
}

print("\n");
print($failed ? "Batch processing failed\n" : "Batch processing complete\n");
exit($failed ? 1 : 0);
