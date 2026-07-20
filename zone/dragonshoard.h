#pragma once

#include "common/eq_packet_structs.h"
#include "common/types.h"

class Client;
class EQApplicationPacket;

// Dragon's Hoard feature (The Outer Brood client).
// DB table: dragonhoard_items (account_id, slot_id, item_id, item_name, stack_count).

namespace DragonHoard {

	// OP_DragonHoard action codes — the leading u32 of the packet body.
	enum Action : uint32 {
		WindowOpen   = 0,  // c->s: window opened (no server response needed)
		SetSlotCount = 2,  // s->c: max slot count
		Retrieve     = 3,  // c->s request / s->c confirm
		Deposit      = 4,  // c->s: deposit the cursor item
		Enable       = 8,  // s->c: set the client enabled flag
		DepositAck   = 10, // s->c: clear the pending-op counter after a deposit
	};

	// True only when the feature rule is on AND the client is TOB or later.
	bool IsEnabled(Client* client);

	// Zone-in: populate the DH window with the character's stored items.
	void SendItemList(Client* client);

	// Send the DH window's enable flag + slot capacity (the feature grant rides the player-profile
	// claims array, not this).
	void SendUnlock(Client* client);

	// Client deposits the cursor item into the hoard.
	void HandleDeposit(Client* client, const EQApplicationPacket* app);

	// Client retrieves an item from the hoard to the cursor.
	void HandleRetrieve(Client* client, const EQApplicationPacket* app);

} // namespace DragonHoard
