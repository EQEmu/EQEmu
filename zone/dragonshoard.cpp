#include "zone/dragonshoard.h"
#include "zone/client.h"
#include "common/global_define.h"
#include "common/item_instance.h"
#include "common/repositories/dragonhoard_items_repository.h"
#include "common/rulesys.h"
#include "common/serialize_buffer.h"
#include "common/strings.h"
#include <cstring>

// Dragon's Hoard feature handler (The Outer Brood client).
// Opcodes: OP_DragonHoard=0x5807 (window/list/unlock + deposit/retrieve action codes),
//          OP_FeatureUnlock=0x5B9B (client feature array). Item packet: ItemPacketDragonHoard=0x77.

bool DragonHoard::IsEnabled(Client* client)
{
	return client
		&& RuleB(Features, DragonHoardEnabled)
		&& client->ClientVersion() >= EQ::versions::ClientVersion::TOB;
}

void DragonHoard::SendItemList(Client* client)
{
	if (!IsEnabled(client)) {
		return;
	}

	const uint32 account_id = client->AccountID();
	if (!account_id) {
		return;
	}

	const auto rows = DragonhoardItemsRepository::GetWhere(
		database, fmt::format("account_id = {} ORDER BY slot_id", account_id));

	for (const auto& e : rows) {
		const uint32 slot_id = static_cast<uint32>(e.slot_id);
		const uint32 item_id = static_cast<uint32>(e.item_id);

		const EQ::ItemData* item_data = database.GetItem(item_id);
		if (!item_data) {
			LogError(
				"DragonHoard::SendItemList item_id {} not found for account_id {}",
				item_id,
				account_id
			);
			continue;
		}

		EQ::ItemInstance* inst = database.CreateItem(item_data, e.stack_count);
		if (!inst) {
			continue;
		}

		// Stamp a stable per-slot serial (slot_id + 1, so it's never 0). The client echoes
		// it back on retrieve, letting us identify the exact item by serial instead of a
		// fragile positional index.
		inst->SetSerialNumber(static_cast<int32>(slot_id + 1));

		client->SendItemPacket(slot_id, inst, ItemPacketType::ItemPacketDragonHoard);
		safe_delete(inst);
	}

	LogDebug("DragonHoard::SendItemList sent items to account_id {}", account_id);
}

// Sends the DH window's enable flag (action 8) and slot capacity (action 2). The feature GRANT
// is carried by the player-profile claims array (see the TOB OP_PlayerProfile encode); this only
// sets the window's own DragonHoardCapacity field, which the claim does not populate.
void DragonHoard::SendUnlock(Client* client)
{
	if (!IsEnabled(client)) {
		return;
	}

	// enable flag
	SerializeBuffer enable;
	enable.WriteUInt32(Enable);
	enable.WriteUInt8(0);
	auto* outapp = new EQApplicationPacket(OP_DragonHoard, enable.size());
	outapp->WriteData(enable.buffer(), enable.size());

	// max slot count
	SerializeBuffer slots;
	slots.WriteUInt32(SetSlotCount);
	slots.WriteUInt32(static_cast<uint32>(RuleI(Features, DragonHoardSlots)));
	auto* outapp2 = new EQApplicationPacket(OP_DragonHoard, slots.size());
	outapp2->WriteData(slots.buffer(), slots.size());

	client->FastQueuePacket(&outapp);
	client->FastQueuePacket(&outapp2);

	LogDebug("DragonHoard::SendUnlock sent to {}", client->GetName());
}

void DragonHoard::HandleDeposit(Client* client, const EQApplicationPacket* app)
{
	if (!app || !IsEnabled(client)) {
		return;
	}

	// deposit packet layout: [action=4 u32][item_id u64] — minimum 12 bytes
	if (app->size < 12) {
		LogError("DragonHoard::HandleDeposit packet too small: {}", app->size);
		return;
	}

	const EQ::ItemInstance* cursor_item = client->GetInv().GetItem(EQ::invslot::slotCursor);
	if (!cursor_item) {
		LogDebug("DragonHoard::HandleDeposit no cursor item for {}", client->GetName());
		return;
	}

	const uint32 account_id = client->AccountID();
	const uint32 item_id = cursor_item->GetItem()->ID;
	const uint32 stack = cursor_item->GetCharges() > 0 ? static_cast<uint32>(cursor_item->GetCharges()) : 1U;
	const char* name = cursor_item->GetItem()->Name;
	const EQ::ItemData* item_data = cursor_item->GetItem();

	// Next slot: append after the highest existing slot (MAX(slot_id) + 1).
	uint32 slot_id = 0;
	const auto highest = DragonhoardItemsRepository::GetWhere(
		database, fmt::format("account_id = {} ORDER BY slot_id DESC LIMIT 1", account_id));
	if (!highest.empty()) {
		slot_id = static_cast<uint32>(highest.front().slot_id) + 1;
	}

	if (slot_id >= static_cast<uint32>(RuleI(Features, DragonHoardSlots))) {
		client->Message(Chat::Red, "Your Dragon's Hoard is full.");
		return;
	}

	auto e = DragonhoardItemsRepository::NewEntity();
	e.account_id  = static_cast<int32>(account_id);
	e.slot_id     = static_cast<int32>(slot_id);
	e.item_id     = static_cast<int32>(item_id);
	e.item_name   = name ? name : "";
	e.stack_count = stack;

	e = DragonhoardItemsRepository::InsertOne(database, e);
	if (!e.id) {
		LogError("DragonHoard::HandleDeposit DB insert failed for account_id {}", account_id);
		return;
	}

	// Remove item from cursor
	client->DeleteItemInInventory(EQ::invslot::slotCursor, 0, true);

	// Send item to DH window. Stamp the same stable per-slot serial (slot_id + 1) that
	// SendItemList uses, so the item can be retrieved immediately without re-zoning —
	// otherwise a freshly deposited item carries an ephemeral serial that maps to no slot.
	EQ::ItemInstance* inst = database.CreateItem(item_data, static_cast<int16>(stack));
	if (inst) {
		inst->SetSerialNumber(static_cast<int32>(slot_id + 1));
		client->SendItemPacket(slot_id, inst, ItemPacketType::ItemPacketDragonHoard);
		safe_delete(inst);
	}

	// Send the action=10 (0xA) deposit acknowledgement. The client increments an internal
	// pending-operation counter when it sends a deposit and freezes inventory/mouse input
	// until the server acks it (client sub_14020B560 case 0xA -> decrement dword_140EE2B2C).
	// Unlike action=3 this only clears the pending op; it does not remove a window row.
	SerializeBuffer ack_buf;
	ack_buf.WriteUInt32(DepositAck);
	auto* ack = new EQApplicationPacket(OP_DragonHoard, ack_buf.size());
	ack->WriteData(ack_buf.buffer(), ack_buf.size());
	client->FastQueuePacket(&ack);

	LogDebug("DragonHoard::HandleDeposit account_id {} deposited item_id {} slot {}", account_id, item_id, slot_id);
}

void DragonHoard::HandleRetrieve(Client* client, const EQApplicationPacket* app)
{
	if (!app || !IsEnabled(client)) {
		return;
	}

	// retrieve packet layout (client sub_14010DAE0):
	//   +0x00 u32 action=3
	//   +0x04 u64 serial  <- the selected row's item serial (we stamped it = slot_id+1)
	//   +0x0C u32 index   <- positional index; unreliable, ignored
	// The old code used the +0x0C index with ORDER BY item_name, but SendItemList sends
	// ORDER BY slot_id, so the index never matched the clicked row (always the wrong item).
	if (app->size < 16) {
		return;
	}
	uint64 client_serial = 0;
	memcpy(&client_serial, app->pBuffer + 4, sizeof(uint64));
	if (client_serial == 0) {
		return;
	}
	const uint32 target_slot = static_cast<uint32>(client_serial) - 1; // undo the +1 from SendItemList

	const uint32 account_id = client->AccountID();

	const auto rows = DragonhoardItemsRepository::GetWhere(
		database, fmt::format("account_id = {} AND slot_id = {}", account_id, target_slot));

	if (rows.empty()) {
		LogError("DragonHoard::HandleRetrieve slot {} not found for account_id {}", target_slot, account_id);
		return;
	}

	const auto& row = rows.front();
	const uint32 slot_id = static_cast<uint32>(row.slot_id);
	const uint32 item_id = static_cast<uint32>(row.item_id);
	const uint32 stack = row.stack_count;

	const EQ::ItemData* item_data = database.GetItem(item_id);
	if (!item_data) {
		LogError("DragonHoard::HandleRetrieve item_id {} not found", item_id);
		return;
	}

	// Check cursor is empty
	if (client->GetInv().GetItem(EQ::invslot::slotCursor)) {
		client->Message(Chat::Red, "You must have an empty cursor to retrieve an item.");
		return;
	}

	// Remove from DB
	DragonhoardItemsRepository::DeleteWhere(
		database, fmt::format("account_id = {} AND slot_id = {}", account_id, slot_id));

	// Give item to cursor
	EQ::ItemInstance* inst = database.CreateItem(item_data, static_cast<int16>(stack));
	if (inst) {
		client->PushItemOnCursor(*inst, true);
		safe_delete(inst);
	}

	// Send the action=3 retrieve confirmation. Without it the client leaves a pending DH
	// operation open, which locks the item on the cursor and never updates the window. The
	// client handler (sub_14020B560 case 3 -> sub_14010D8C0) matches the item by serial,
	// removes the row (when qty >= its stack), and decrements the pending-op counter, which
	// releases the cursor. Layout mirrors the request: [action=3][serial u64][qty u32].
	SerializeBuffer confirm_buf;
	confirm_buf.WriteUInt32(Retrieve);
	confirm_buf.WriteUInt64(client_serial); // == slot_id+1 we stamped
	confirm_buf.WriteUInt32(stack);         // >= stack removes the row
	auto* confirm = new EQApplicationPacket(OP_DragonHoard, confirm_buf.size());
	confirm->WriteData(confirm_buf.buffer(), confirm_buf.size());
	client->FastQueuePacket(&confirm);

	LogDebug("DragonHoard::HandleRetrieve account_id {} slot {} retrieved item_id {}", account_id, target_slot, item_id);
}
