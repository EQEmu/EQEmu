opendir my $dir, "logs" or die "Cannot open directory: $!";
my @files = readdir $dir;
closedir $dir;
$inc = 0;
foreach my $val (@files){
	if($val=~/crash_zone/i){
		$stl = 0;
		$crash[$inc] = "";
		my $file = "logs/" . $val;
		open my $info, $file or die "Could not open $file: $!"; 
		while( my $line = <$info>)  {
			if($line=~/CRTStartup/i){ $stl = 0; }
			@data = split(']', $line);
			if($stl == 1){ $crash[$inc] .= $data[1]; }
			if($line=~/dbghelp.dll/i){ $stl = 1; }
		} 
		close $info;
		$inc++;
	}
}

#::: Count Crash Occurrence first
$i = 0;
while($crash[$i]){
	$crash_count[length($crash[$i])]++;
	$unique_crash[length($crash[$i])] = $crash[$i];
	$i++;
}

$i = 0;
while($crash[$i]){
	if($unique_crash_tracker[length($crash[$i])] != 1){
		print "Crash Occurrence " . $crash_count[length($crash[$i])] . " Time(s) Length (" . length($crash[$i]) .  ") \n\n"; 
		print $crash[$i] . "\n";
		print "=========================================\n";
	}
	$unique_crash_tracker[length($crash[$i])] = 1;
	$i++;
}