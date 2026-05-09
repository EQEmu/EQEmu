use DBI;

my $db = peq;
my $user = eqemu;
my $pass = eqemu;
my $host = localhost;

my $source = "DBI:mysql:database=$db;host=$host";
my $dbh = DBI->connect($source, $user, $pass) || die "Could not create db handle\n";

my $select_query = "SELECT id,name,base,mod_c1,mod_c2,mod_c3,mod_c4,mod_c5,mod_c6,mod_c7,mod_c8,mod_c9,mod_c10,mod_c11,mod_c12,";
$select_query .= "mod_c13,mod_c14,mod_c15,mod_c16,mod_r1,mod_r2,mod_r3,mod_r4,mod_r5,mod_r6,mod_r7,mod_r8,mod_r9,mod_r10,mod_r11,";
$select_query .= "mod_r12,mod_r14,mod_r42,mod_r75,mod_r108,mod_r128,mod_r130,mod_r161,mod_r330,mod_r367,mod_r522,mod_d140,mod_d201,";
$select_query .= "mod_d202,mod_d203,mod_d204,mod_d205,mod_d206,mod_d207,mod_d208,mod_d209,mod_d210,mod_d211,mod_d212,mod_d213,";
$select_query .= "mod_d214,mod_d215,mod_d216 FROM faction_list";

my $count = 0;
my $sth = $dbh->prepare($select_query);
$sth->execute();
while (my $ref = $sth->fetchrow_hashref()) {
    for(my $i = 1; $i <= 16; $i++) {
        my $field_name = "mod_c" . $i;
        if($ref->{$field_name} != 0) {
            my $mod_name = "c" . $i;
            my $rsth = $dbh->prepare("REPLACE INTO `faction_list_mod` (`faction_id`,`mod`,`mod_name`) VALUES(?, ?, ?)");
            $rsth->execute($ref->{id}, $ref->{$field_name}, $mod_name);
	     $count++;
        }
    }

    for(my $i = 1; $i <= 12; $i++) {
        my $field_name = "mod_r" . $i;
        if($ref->{$field_name} != 0) {
            my $mod_name = "r" . $i;
            my $rsth = $dbh->prepare("REPLACE INTO `faction_list_mod` (`faction_id`,`mod`,`mod_name`) VALUES(?, ?, ?)");
            $rsth->execute($ref->{id}, $ref->{$field_name}, $mod_name);
	     $count++;
        }
    }

    if($ref->{"mod_r14"} != 0) {
        my $rsth = $dbh->prepare("REPLACE INTO `faction_list_mod` (`faction_id`,`mod`,`mod_name`) VALUES(?, ?, ?)");
        $rsth->execute($ref->{id}, $ref->{"mod_r14"}, "r14");
	 $count++;
    }

    if($ref->{"mod_r42"} != 0) {
        my $rsth = $dbh->prepare("REPLACE INTO `faction_list_mod` (`faction_id`,`mod`,`mod_name`) VALUES(?, ?, ?)");
        $rsth->execute($ref->{id}, $ref->{"mod_r42"}, "r42");
	 $count++;
    }

    if($ref->{"mod_r75"} != 0) {
        my $rsth = $dbh->prepare("REPLACE INTO `faction_list_mod` (`faction_id`,`mod`,`mod_name`) VALUES(?, ?, ?)");
        $rsth->execute($ref->{id}, $ref->{"mod_r75"}, "r75");
	 $count++;
    }

    if($ref->{"mod_r108"} != 0) {
        my $rsth = $dbh->prepare("REPLACE INTO `faction_list_mod` (`faction_id`,`mod`,`mod_name`) VALUES(?, ?, ?)");
        $rsth->execute($ref->{id}, $ref->{"mod_r108"}, "r108");
	 $count++;
    }

    if($ref->{"mod_r128"} != 0) {
        my $rsth = $dbh->prepare("REPLACE INTO `faction_list_mod` (`faction_id`,`mod`,`mod_name`) VALUES(?, ?, ?)");
        $rsth->execute($ref->{id}, $ref->{"mod_r128"}, "r128");
	 $count++;
    }

    if($ref->{"mod_r130"} != 0) {
        my $rsth = $dbh->prepare("REPLACE INTO `faction_list_mod` (`faction_id`,`mod`,`mod_name`) VALUES(?, ?, ?)");
        $rsth->execute($ref->{id}, $ref->{"mod_r130"}, "r130");
	 $count++;
    }

    if($ref->{"mod_r161"} != 0) {
        my $rsth = $dbh->prepare("REPLACE INTO `faction_list_mod` (`faction_id`,`mod`,`mod_name`) VALUES(?, ?, ?)");
        $rsth->execute($ref->{id}, $ref->{"mod_r161"}, "r161");
	 $count++;
    }

    if($ref->{"mod_r330"} != 0) {
        my $rsth = $dbh->prepare("REPLACE INTO `faction_list_mod` (`faction_id`,`mod`,`mod_name`) VALUES(?, ?, ?)");
        $rsth->execute($ref->{id}, $ref->{"mod_r330"}, "r330");
	 $count++;
    }

    if($ref->{"mod_r367"} != 0) {
        my $rsth = $dbh->prepare("REPLACE INTO `faction_list_mod` (`faction_id`,`mod`,`mod_name`) VALUES(?, ?, ?)");
        $rsth->execute($ref->{id}, $ref->{"mod_r367"}, "r367");
	 $count++;
    }

    if($ref->{"mod_r522"} != 0) {
        my $rsth = $dbh->prepare("REPLACE INTO `faction_list_mod` (`faction_id`,`mod`,`mod_name`) VALUES(?, ?, ?)");
        $rsth->execute($ref->{id}, $ref->{"mod_r522"}, "r522");
	 $count++;
    }

    for(my $i = 201; $i <= 216; $i++) {
        my $field_name = "mod_d" . $i;
        if($ref->{$field_name} != 0) {
            my $mod_name = "d" . $i;
            my $rsth = $dbh->prepare("REPLACE INTO `faction_list_mod` (`faction_id`,`mod`,`mod_name`) VALUES(?, ?, ?)");
            $rsth->execute($ref->{id}, $ref->{$field_name}, $mod_name);
	     $count++;
        }
    }
}

print "$count entries created.\n";