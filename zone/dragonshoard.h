#pragma once

#include "../common/eq_packet_structs.h"
#include "../common/types.h"

class Client;
class EQApplicationPacket;

// Dragon's Hoard feature (The Outer Brood client).
// DB table: dragonhoard_items (account_id, slot_id, item_id, item_name, stack_count).

namespace DragonHoard {

	// True only when the feature rule is on AND the client is TOB or later.
	bool IsEnabled(Client* client);

	// Zone-in: populate the DH window with the character's stored items.
	void SendItemList(Client* client);

	// OP_FeatureUnlock (0x5B9B): populate the client feature array (player+0x2620).
	// This is the gate the deposit path checks; must be sent before SendUnlock.
	void SendFeatureUnlock(Client* client);

	// Send action=8 (enable flag) and action=2 (slot count) to unlock the DH window.
	void SendUnlock(Client* client);

	// Client deposits the cursor item into the hoard (OP_DragonHoard1 action=4).
	void HandleDeposit(Client* client, const EQApplicationPacket* app);

	// Client retrieves an item from the hoard to the cursor (OP_DragonHoard1 action=3).
	void HandleRetrieve(Client* client, const EQApplicationPacket* app);

	// Send a single item add/remove delta to the DH window.
	void SendItemUpdate(Client* client, uint32 slot_id, uint32 item_id, bool remove);

	// Max slots available in Dragon's Hoard.
	static constexpr int MAX_SLOTS = 200;

} // namespace DragonHoard
