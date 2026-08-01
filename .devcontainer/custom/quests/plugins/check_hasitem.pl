#plugin::check_hasitem($client, $item_id);

# AoTv4: a Hallowed or Mythic copy IS the item.
# ⚠️⚠️ THIS PLUGIN SCANS INVENTORY SLOTS ITSELF rather than calling CountItem, so none of the C++
# choke points that make quest item checks tier-blind is on this path -- NPC::CheckHandin covers
# turn-ins, and Lua_Client::CountItem / Perl_Client_CountItem / quest::countitem cover possession,
# but a raw `GetItemIDAt($slot) == $item_id` reaches none of them. Without this, 133 Perl quest
# scripts tell a player holding only the Mythic version that they do not have the item at all.
# This is the exact Perl twin of aotv4_same_item in lua_modules/client_ext.lua, which exists for the
# same reason on Client:HasItem -- fix the two together.
# ⚠️ Keep the band in step with zone/aotv4_tiers.h: Hallowed = base +300,000, Mythic = +600,000, so
# the reserved band is [300000, 900000) and a tier id reduces to its base with id % 300000.
sub aotv4_item_base {
	my $id = shift;
	$id = 0 unless defined $id;
	return ($id >= 300000 && $id < 900000) ? ($id % 300000) : $id;
}

sub aotv4_same_item {
	my ($a, $b) = @_;
	return 1 if defined $a && defined $b && $a == $b;
	return aotv4_item_base($a) == aotv4_item_base($b);
}

sub check_hasitem {
	my $client = shift;
	my $item_id = shift;
	#my $body_count = $client->GetCorpseCount();
	my @augment_slots = (
		quest::getinventoryslotid("augsocket.begin")..quest::getinventoryslotid("augsocket.end")
	);
	#my @corpse_slots = (
	#	quest::getinventoryslotid("possessions.begin")..quest::getinventoryslotid("possessions.end"),
	#	quest::getinventoryslotid("generalbags.begin")..quest::getinventoryslotid("generalbags.end"),
	#);
	my @inventory_slots = (
		quest::getinventoryslotid("possessions.begin")..quest::getinventoryslotid("possessions.end"),
		quest::getinventoryslotid("generalbags.begin")..quest::getinventoryslotid("generalbags.end"),
		quest::getinventoryslotid("bank.begin")..quest::getinventoryslotid("bank.end"),
		quest::getinventoryslotid("bankbags.begin")..quest::getinventoryslotid("bankbags.end"),
		quest::getinventoryslotid("sharedbank.begin")..quest::getinventoryslotid("sharedbank.end"),
		quest::getinventoryslotid("sharedbankbags.begin")..quest::getinventoryslotid("sharedbankbags.end"),
	);
	foreach $slot_id (@inventory_slots) {
		if ($client->GetItemAt($slot_id)) {
			if (plugin::aotv4_same_item($client->GetItemIDAt($slot_id), $item_id)) {
				return 1;
			}

			foreach $augment_slot (@augment_slots) {
				if ($client->GetAugmentAt($slot_id, $augment_slot) && plugin::aotv4_same_item($client->GetAugmentIDAt($slot_id, $augment_slot), $item_id)) {
					return 1;
				}
			}
		}
	}

	if ($client->HasItemOnCorpse($item_id)) {
		return 1;
	}

  	return 0;
}

return 1;
