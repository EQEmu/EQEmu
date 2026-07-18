#pragma once

#include "../common/eq_packet_structs.h" // [DH_DEPOSIT_RETRIEVE]
#include "../common/types.h"

class Client;
class EQApplicationPacket;

// Dragon's Hoard feature handler
// Universal implementation - patch-agnostic logic
// Serialization is handled by the patch-specific serializer (tob.cpp etc.)
// [DH_SEND_ITEM_LIST] DB table: dragonhoard_items (account_id, slot_id, item_id, item_name, stack_count)

namespace DragonHoard {

	// Called on zone-in to populate the DH window with the character's stored items
	void SendItemList(Client* client);

	// [DH_UNLOCK] Send action=8 (enable flag) and action=2 (slot count) to unlock the DH window
	void SendUnlock(Client* client);

	// Called when client deposits an item into Dragon's Hoard
	void HandleDeposit(Client* client, const EQApplicationPacket* app);

	// Called when client retrieves an item from Dragon's Hoard
	void HandleRetrieve(Client* client, const EQApplicationPacket* app);

	// Send a single item update to the DH window (add or remove)
	void SendItemUpdate(Client* client, uint32 slot_id, uint32 item_id, bool remove);

	// Max slots available in Dragon's Hoard
	static constexpr int MAX_SLOTS = 200;

} // namespace DragonHoard
