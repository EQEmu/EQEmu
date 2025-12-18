#include "zone/client.h"

void command_peqzone(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #peqzone [Zone ID] or #peqzone [Zone Short Name]");
		return;
	}

	auto reuse_timer = RuleI(Zone, PEQZoneReuseTime);
	if (reuse_timer) {
		uint32 time_left = c->GetPTimers().GetRemainingTime(pTimerPeqzoneReuse);
		if (!c->GetPTimers().Expired(&database, pTimerPeqzoneReuse, false)) {
			c->Message(
				Chat::White,
				fmt::format(
					"You must wait {} before using this command again.",
					Strings::SecondsToTime(time_left)
				).c_str()
			);
			return;
		}
	}

	auto hp_ratio = RuleI(Zone, PEQZoneHPRatio);
	if (c->GetHPRatio() < hp_ratio) {
		c->Message(
			Chat::White,
			fmt::format(
				"You cannot use this command with less than {}%% health.",
				hp_ratio
			).c_str()
		);
		return;
	}

	if (
		c->IsInvisible(c) ||
		c->IsRooted() ||
		c->IsStunned() ||
		c->IsMezzed() ||
		c->AutoAttackEnabled() ||
		c->GetInvul()
	) {
		c->Message(Chat::White, "You cannot use this command in your current state. Settle down and wait.");
		return;
	}

	auto zone_id = (
		sep->IsNumber(1) ?
		static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[1])) :
		static_cast<uint16>(ZoneID(sep->arg[1]))
	);
	auto zone_short_name = ZoneName(zone_id);
	auto zone_long_name = ZoneLongName(zone_id);
	if (
		!zone_id ||
		!zone_short_name
	) {
		c->Message(
			Chat::White,
			fmt::format(
				"No zones were found matching '{}'.",
				sep->arg[1]
			).c_str()
		);
		return;
	}

	uint8 peqzone_flag = content_db.GetPEQZone(zone_id, 0);
	if (peqzone_flag == 0) {
		c->Message(
			Chat::White,
			fmt::format(
				"You cannot use this command to enter {} ({}).",
				zone_long_name,
				zone_short_name
			).c_str()
		);
		return;
	} else if (peqzone_flag == 2 && !c->HasPEQZoneFlag(zone_id)) {
		c->Message(
			Chat::White,
			fmt::format(
				"You do not have the required PEQZone flag to use this command to enter {} ({}).",
				zone_long_name,
				zone_short_name
			).c_str()
		);
		return;
	}

	if (zone_id == zone->GetZoneID()) {
		c->Message(
			Chat::White,
			fmt::format(
				"You are already in {}.",
				zone->GetZoneDescription()
			).c_str()
		);
		return;
	}

	if (RuleB(Zone, UsePEQZoneDebuffs)) {
		c->SpellOnTarget(RuleI(Zone, PEQZoneDebuff1), c);
		c->SpellOnTarget(RuleI(Zone, PEQZoneDebuff2), c);
	}

	if (reuse_timer) {
		c->GetPTimers().Start(pTimerPeqzoneReuse, reuse_timer);
	}

	c->MoveZone(zone_short_name);
}
