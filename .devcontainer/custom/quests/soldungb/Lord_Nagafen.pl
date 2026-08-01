# AoTv4: the classic level>=53 banish was REMOVED. The level cap rises past 52 as eras unlock
# (Kunark+ = 60/65/70), so a max-level Bard MUST be able to fight Nagafen -- the old gate banished
# them to lavastorm on enter/aggro/target, making the encounter impossible. Leash logic kept.
my $spawn_x = 0;
my $spawn_y = 0;
my $spawn_z = 0;
my $spawn_h = 0;

sub EVENT_SPAWN {
	$spawn_x = $x;
	$spawn_y = $y;
	$spawn_z = $z;
	$spawn_h = $h;
	my $range = 200;
	quest::set_proximity_range($range, $range);
	quest::setnexthpevent(96);
}

sub EVENT_HP {
	#if my HP are dropping make certain the timer is running
	#this gets around 100% pet tanking, because the owner is
	#on the aggro list but with 0's and EVENT_AGGRO isn't firing.
	quest::stoptimer(1);
	EVENT_AGGRO();
	#backup safety check
	quest::setnexthpevent(int($npc->GetHPRatio()) - 9);
}

# AoTv4: EVENT_ENTER banish removed (see header).

sub EVENT_AGGRO {
	# a 1 second leash timer.
	quest::settimer(1,1);
}

sub EVENT_TIMER {
	if ($timer == 1) {
		if ($x < -1000 || $x > -650 || $y < -1500 || $y > -1290) {
			WIPE_AGGRO();
		}

		# AoTv4: level>52 banish removed. Keep the leash: if nothing is on hate, reset to spawn.
		my @hate_list = $npc->GetHateListClients();
		if (@hate_list == 0) {
			WIPE_AGGRO();
		}
	}
}

sub WIPE_AGGRO {
	$npc->BuffFadeAll();
	$npc->WipeHateList();
	$npc->SetHP($npc->GetMaxHP());
	$npc->GMMove($spawn_x, $spawn_y, $spawn_z, $spawn_h);
	quest::stoptimer(1);
	quest::setnexthpevent(96);
}
