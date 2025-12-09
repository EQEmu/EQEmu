#include "../../client.h"
#include "../../zone.h"

extern Zone* zone;

void ShowZoneVariables(Client *c, const Seperator *sep)
{
	if (!zone->GetVariables().empty()) {
		c->Message(Chat::White, "Zone Variables:");
		for (auto &key: zone->GetVariables()) {
			c->Message(Chat::White, fmt::format("{}: {}", key, zone->GetVariable(key)).c_str());
		}
	} else {
		c->Message(Chat::White, "No zone variables set.");
	}
}
