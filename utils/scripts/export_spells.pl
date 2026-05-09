#! /usr/bin/perl

#use strict;
#use warnings;
use DBI;
use Getopt::Std;

#get config info from eqemu_config.xml to connect to the db & point to spells_us.txt
getopts('c:t:i:s:oh');
my $conf = "eqemu_config.xml"; #default
my $table = "spells_new"; #default
my $id = "id"; #default
my $spellf = "spells_us.txt"; #default

if ($opt_h) { #help (-h)
	printf "\nUsage: export_spells.pl [-c path] [-t table] [-i column] [-s path] [-o]\n";
	printf "  -c path	path/to/eqemu_config.xml. defaults to $conf\n";
	printf "  -t table	table to load the spells from. defaults to $table\n";
	printf "  -i column	name of the column in the database table to order by. defaults to $id\n";
	printf "  -s path	path/to/export/spells_us.txt. defaults to $spellf\n";
	printf "  -o		overwrite $spellf if it exists\n";
	printf "\n";
	exit;
}
if ($opt_c) {$conf = $opt_c;} #use config file from -c, if defined
if ($opt_t) {$table = $opt_t;} #use db table -t, if defined
if ($opt_i) {$id = $opt_i;} #use column name -i, if defined
if ($opt_s) {$spellf = $opt_s;} #use spells file from -s, if defined

my $db = "eq";
my $user = "eq";
my $pass = "eq";
my $host = "localhost";
open(F, "<$conf") or die "Unable to open config: $conf\n";
my $indb = 0;
while(<F>) {
	s/\r//g;
	if(/<database>/i) {
		$indb = 1;
	}
	next unless($indb == 1);
	if(/<\/database>/i) {
		$indb = 0;
		last;
	}
	if(/<host>(.*)<\/host>/i) {
		$host = $1;
	} elsif(/<username>(.*)<\/username>/i) {
		$user = $1;
	} elsif(/<password>(.*)<\/password>/i) {
		$pass = $1;
	} elsif(/<db>(.*)<\/db>/i) {
		$db = $1;
	}
}
if(!$db || !$user || !$pass || !$host) {
	die "Invalid database info, missing one of: host, user, password, database\n";
}

#connect to the db before we waste time loading the spells file
my $source = "DBI:mysql:database=$db;host=$host";
my $dbh = DBI->connect($source, $user, $pass) || die "Could not create db handle\n";

if ((-e $spellf) && !$opt_o) {	#spell file already exists & we don't want to overwrite
	die "'$spellf' already exists; Use -o if you want to overwrite\n";
}

#open spell file
open(SPELLS, ">$spellf") or die "Unable to open spell file for export: $spellf\n";

#parse through spells & write to spells_us.txt
my $sth = $dbh->prepare("SELECT * FROM $table ORDER BY $id ASC");
$sth->execute(); #run the query on the db
my($numspells, $loadedspells, $highid); #define some variables, cause we're cool like that
while (my $val = $sth->fetch()) {
	++$numspells;
	printf("Exporting \"%s\" (%d)                            \r", @$val[1], @$val[0]); # name (id)
	my $line = join("^", @$val) . "\n"; #convert to delimited string, with a newline at the end
	print SPELLS $line; #write to the file
	$highid = @$val[0]; #set to current id
}

close (SPELLS); #since we're done with it...

#summary of results
print "Spells Exported: $numspells                            \n";
print "Highest ID: $highid\n\n";