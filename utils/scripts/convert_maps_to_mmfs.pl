#!/usr/bin/perl

###########################################################
#::: Automatic (Map-to-MMF) Conversion Script
#::: Author: Uleat
#::: Purpose: To convert existing zone maps to memory-mapped files
###########################################################

use Config;

print("\n");
print("Zone Map-to-MMF Batch convertor\n");
print("===============================\n");
print("\n");

if($Config{osname}=~/freebsd|linux/i){ $OS = "Linux"; }
if($Config{osname}=~/Win|MS/i){ $OS = "Windows"; }
print("Operating System is: $Config{osname}\n");
print("\n");

opendir(D, "maps") || die "Can't find directory maps: $!\n";
my @mapfiles = grep { /\.map$/ && !/_lit/ } readdir(D);
closedir(D);

foreach my $mapfile (@mapfiles) {
	my $result = "Unknown action..\n";
	print("processing map: '$mapfile'\n");
    if($OS eq "Windows"){ $result = `zone convert_map $mapfile`; } 
	if($OS eq "Linux"){ $result = `./zone convert_map $mapfile`; } 
	print("-- $result");
}

print("\n");
print("Batch processing complete\n")
