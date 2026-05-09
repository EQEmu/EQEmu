sub EVENT_SAY {
	if($text=~/despawn/i)
	{
		quest::disable_spawn2(10842);
		quest::say("Yes sir!");
	}
	elsif($text=~/enable/i)
	{
		quest::enable_spawn2(10842);
		quest::say("Yes sir!");
	}
	elsif($text=~/spawn/i)
	{
		quest::spawn_from_spawn2(10842);
		quest::say("Yes sir!");
	}
}