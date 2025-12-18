#include "zone/client.h"
#include "zone/dynamic_zone.h"

void command_dz(Client *c, const Seperator *sep)
{
	if (!c || !zone) {
		return;
	}

	if (strcasecmp(sep->arg[1], "cache") == 0) {
		if (strcasecmp(sep->arg[2], "reload") == 0) {
			DynamicZone::CacheAllFromDatabase();
			c->Message(Chat::White, fmt::format("Reloaded [{}] dynamic zone(s) from database", zone->dynamic_zone_cache.size()).c_str());
		}
	}
	else if (strcasecmp(sep->arg[1], "destroy") == 0 && sep->IsNumber(2)) {
		auto dz_id = std::strtoul(sep->arg[2], nullptr, 10);
		if (auto dz = DynamicZone::FindDynamicZoneByID(dz_id)) {
			c->Message(Chat::White, fmt::format("Destroying dz [{}] ({})", dz_id, dz->GetName()).c_str());
			dz->RemoveAllMembers();
		}
		else {
			c->Message(Chat::Red, fmt::format("Failed to destroy dz [{}]", sep->arg[3]).c_str());
		}
	}
	else if (strcasecmp(sep->arg[1], "list") == 0) {
		std::vector<DynamicZone*> dynamic_zones;
		for (const auto& it : zone->dynamic_zone_cache) {
			dynamic_zones.push_back(it.second.get());
		}

		std::ranges::sort(dynamic_zones, {}, &DynamicZone::GetID);
		c->Message(Chat::White, fmt::format("Total Dynamic Zones (cache): [{}]", dynamic_zones.size()).c_str());
		for (const DynamicZone* dz : dynamic_zones) {
			uint32_t seconds = dz->GetSecondsRemaining();
			c->Message(Chat::White, fmt::format(
				"id: [{}] - [{}] - {}: [{}:{}:{}] members: [{}] expires: [{:02}:{:02}:{:02}] leader: [{}]",
				dz->GetID(),
				DynamicZone::GetDynamicZoneTypeName(dz->GetType()),
				Saylink::Silent(fmt::format("#zoneinstance {}", dz->GetInstanceID()), "zone"),
				dz->GetZoneID(),
				dz->GetInstanceID(),
				dz->GetZoneVersion(),
				dz->GetMemberCount(),
				seconds / 3600,    // hours
				seconds / 60 % 60, // minutes
				seconds % 60,      // seconds
				Saylink::Silent(fmt::format("#goto {}", dz->GetLeaderName()), dz->GetLeaderName())
			).c_str());
		}
	}
	else if (strcasecmp(sep->arg[1], "listdb") == 0) {
		auto dz_list = DynamicZonesRepository::AllDzInstancePlayerCounts(database);
		c->Message(Chat::White, fmt::format("Total Dynamic Zones (database): [{}]", dz_list.size()).c_str());

		auto now = std::chrono::system_clock::now();

		for (const auto& dz : dz_list) {
			auto expire_time = std::chrono::system_clock::from_time_t(static_cast<time_t>(dz.start_time) + dz.duration);
			bool is_expired  = now > expire_time;

			if (!is_expired || strcasecmp(sep->arg[2], "all") == 0) {
				auto seconds = std::max(0, static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(expire_time - now).count()));
				c->Message(Chat::White, fmt::format(
					"id: [{}] - [{}] - {}: [{}:{}:{}] members: [{}] expires: [{:02}:{:02}:{:02}]",
					dz.id,
					DynamicZone::GetDynamicZoneTypeName(static_cast<DynamicZoneType>(dz.type)),
					is_expired ? "zone" : Saylink::Silent(fmt::format("#zoneinstance {}", dz.instance), "zone"),
					dz.zone,
					dz.instance,
					dz.version,
					dz.member_count,
					seconds / 3600,    // hours
					seconds / 60 % 60, // minutes
					seconds % 60       // seconds
				).c_str());
			}
		}
	}
	else if (strcasecmp(sep->arg[1], "lockouts") == 0) {
		if (strcasecmp(sep->arg[2], "remove") == 0 && sep->arg[3][0] != '\0') {
			if (sep->arg[5][0] == '\0') {
				c->Message(Chat::White, fmt::format("Removing [{}] lockouts on [{}].", sep->arg[4][0] ? sep->arg[4] : "all", sep->arg[3]).c_str());
			}
			else {
				c->Message(Chat::White, fmt::format("Removing [{}]:[{}] lockout on [{}].", sep->arg[4], sep->arg[5], sep->arg[3]).c_str());
			}
			DynamicZone::RemoveCharacterLockouts(sep->arg[3], sep->arg[4], sep->arg[5]);
		}
	}
	else if (strcasecmp(sep->arg[1], "makeleader") == 0 && sep->IsNumber(2) && sep->arg[3][0] != '\0') {
		uint32_t dz_id = std::strtoul(sep->arg[2], nullptr, 10);
		if (auto dz = DynamicZone::FindDynamicZoneByID(dz_id)) {
			std::string name = FormatName(sep->arg[3]);
			c->Message(Chat::White, fmt::format("Setting expedition [{}] leader to [{}]", dz_id, name).c_str());
			dz->SendWorldMakeLeaderRequest(c->CharacterID(), name);
		}
		else {
			c->Message(Chat::Red, fmt::format("Failed to find expedition [{}]", dz_id).c_str());
		}
	}
	else if (strcasecmp(sep->arg[1], "unlock") == 0 && sep->IsNumber(2)) {
		uint32_t dz_id = std::strtoul(sep->arg[2], nullptr, 10);
		if (auto dz = DynamicZone::FindDynamicZoneByID(dz_id)) {
			c->Message(Chat::White, fmt::format("Unlocking expedition dz [{}]", dz_id).c_str());
			dz->SetLocked(false, true);
		}
		else {
			c->Message(Chat::Red, fmt::format("Failed to find dz [{}]", sep->arg[2]).c_str());
		}
	}
	else {
		c->Message(Chat::White, "#dz usage:");
		c->Message(Chat::White, "#dz cache reload - reload current zone cache from db (also reloads expedition cache)");
		c->Message(Chat::White, "#dz destroy <dz_id> - destroy dz globally (must be in cache)");
		c->Message(Chat::White, "#dz list - list dynamic zones in current zone cache");
		c->Message(Chat::White, "#dz listdb [all] - list dynamic zones in database -- 'all' includes expired");
		c->Message(Chat::White, "#dz lockouts remove <char_name> - delete all of character's expedition lockouts");
		c->Message(Chat::White, "#dz lockouts remove <char_name> \"<expedition_name>\" - delete lockouts by expedition");
		c->Message(Chat::White, "#dz lockouts remove <char_name> \"<expedition_name>\" \"<event_name>\" - delete expedition event lockout");
		c->Message(Chat::White, "#dz makeleader <dz_id> <character_name> - set new expedition leader");
		c->Message(Chat::White, "#dz unlock <dz_id> - unlock expedition");
	}
}
