#include "dragonshoard.h"
#include "client.h"
#include "../common/global_define.h"
#include "../common/item_instance.h"
#include "../common/rulesys.h"
#include "../common/strings.h"
#include <cstring>

// Dragon's Hoard feature handler (The Outer Brood client).
// Opcodes: OP_DragonHoard1=0x5807 (window/list/unlock + deposit/retrieve action codes),
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

	auto results = database.QueryDatabase(
		fmt::format(
			"SELECT slot_id, item_id, stack_count FROM dragonhoard_items "
			"WHERE account_id = {} ORDER BY slot_id",
			account_id
		)
	);

	if (!results.Success()) {
		LogError(
			"DragonHoard::SendItemList failed for account_id {}: {}",
			account_id,
			results.ErrorMessage()
		);
		return;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		const uint32 slot_id = Strings::ToUnsignedInt(row[0]);
		const uint32 item_id = Strings::ToUnsignedInt(row[1]);
		const uint32 stack_count = Strings::ToUnsignedInt(row[2]);

		const EQ::ItemData* item_data = database.GetItem(item_id);
		if (!item_data) {
			LogError(
				"DragonHoard::SendItemList item_id {} not found for account_id {}",
				item_id,
				account_id
			);
			continue;
		}

		EQ::ItemInstance* inst = database.CreateItem(item_data, stack_count);
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

// OP_FeatureUnlock (0x5B9B) populates the client's feature array (player+0x2620). The TOB
// client (sub_1402DC7E0) reads {feature_id, count} pairs; the deposit path first checks
// sub_14065B0E0(player, 2016763), and without a count>0 entry the client silently refuses to
// deposit. Same mechanism as Laurion's OP_FeatureUnlock (0x4451 there).
// Wire format: [u8 header=0][u32 feature_count][ {u32 feature_id, u32 count>=1} x feature_count ]
void DragonHoard::SendFeatureUnlock(Client* client)
{
	if (!IsEnabled(client)) {
		return;
	}

	// TOB feature IDs (from client sub_1402DC7E0). Dragon's Hoard is the only feature the
	// server implements; Tradeskill Depot IDs (2018125 / 2018260) are noted for future use.
	static const uint32 feature_ids[] = {
		2016763, // 0x1EC5FB — Dragon's Hoard
	};
	const uint32 num_features = static_cast<uint32>(sizeof(feature_ids) / sizeof(feature_ids[0]));
	const uint32 active_count = 1; // any value > 0 marks the feature active

	const uint32 packet_size = 1 + 4 + (num_features * 8);
	auto* outapp = new EQApplicationPacket(OP_FeatureUnlock, packet_size);
	memset(outapp->pBuffer, 0, packet_size);

	uint8* buf = outapp->pBuffer;
	buf[0] = 0; // header byte
	*reinterpret_cast<uint32*>(buf + 1) = num_features;
	uint32 off = 5;
	for (uint32 i = 0; i < num_features; ++i) {
		*reinterpret_cast<uint32*>(buf + off)     = feature_ids[i];
		*reinterpret_cast<uint32*>(buf + off + 4) = active_count;
		off += 8;
	}

	client->FastQueuePacket(&outapp);

	LogDebug("DragonHoard::SendFeatureUnlock sent {} feature(s) to {}", num_features, client->GetName());
}

// Sent at zone-in after SendFeatureUnlock: action=8 sets the client's enabled flag,
// action=2 sets the max slot count.
void DragonHoard::SendUnlock(Client* client)
{
	if (!IsEnabled(client)) {
		return;
	}

	// action=8 — enable flag
	auto* outapp = new EQApplicationPacket(OP_DragonHoard1, 5);
	memset(outapp->pBuffer, 0, 5);
	*reinterpret_cast<uint32_t*>(outapp->pBuffer) = 8;
	outapp->pBuffer[4] = 0;

	// action=2 — max slot count
	auto* outapp2 = new EQApplicationPacket(OP_DragonHoard1, 8);
	memset(outapp2->pBuffer, 0, 8);
	*reinterpret_cast<uint32_t*>(outapp2->pBuffer) = 2;
	*reinterpret_cast<uint32_t*>(outapp2->pBuffer + 4) = static_cast<uint32_t>(MAX_SLOTS);

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

	// Find next available slot
	auto slot_result = database.QueryDatabase(
		fmt::format(
			"SELECT COALESCE(MAX(slot_id), -1) + 1 FROM dragonhoard_items WHERE account_id = {}",
			account_id
		)
	);

	uint32 slot_id = 0;
	if (slot_result.Success() && slot_result.RowCount() > 0) {
		auto row = slot_result.begin();
		slot_id = row[0] ? Strings::ToUnsignedInt(row[0]) : 0U;
	}

	if (slot_id >= static_cast<uint32>(MAX_SLOTS)) {
		client->Message(Chat::Red, "Your Dragon's Hoard is full.");
		return;
	}

	auto insert = database.QueryDatabase(
		fmt::format(
			"INSERT INTO dragonhoard_items (account_id, slot_id, item_id, item_name, stack_count) "
			"VALUES ({}, {}, {}, '{}', {})",
			account_id,
			slot_id,
			item_id,
			Strings::Escape(std::string(name ? name : "")),
			stack
		)
	);

	if (!insert.Success()) {
		LogError("DragonHoard::HandleDeposit DB insert failed for account_id {}: {}", account_id, insert.ErrorMessage());
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

	auto result = database.QueryDatabase(
		fmt::format(
			"SELECT slot_id, item_id, stack_count FROM dragonhoard_items "
			"WHERE account_id = {} AND slot_id = {}",
			account_id, target_slot
		)
	);

	if (!result.Success() || result.RowCount() == 0) {
		LogError("DragonHoard::HandleRetrieve slot {} not found for account_id {}", target_slot, account_id);
		return;
	}

	auto row = result.begin();
	const uint32 slot_id = Strings::ToUnsignedInt(row[0]);
	const uint32 item_id = Strings::ToUnsignedInt(row[1]);
	const uint32 stack = Strings::ToUnsignedInt(row[2]);

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
	auto del = database.QueryDatabase(
		fmt::format(
			"DELETE FROM dragonhoard_items WHERE account_id = {} AND slot_id = {}",
			account_id,
			slot_id
		)
	);

	if (!del.Success()) {
		LogError("DragonHoard::HandleRetrieve DB delete failed: {}", del.ErrorMessage());
		return;
	}

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
	auto* confirm = new EQApplicationPacket(OP_DragonHoard1, 16);
	memset(confirm->pBuffer, 0, 16);
	*reinterpret_cast<uint32*>(confirm->pBuffer)      = 3;
	*reinterpret_cast<uint64*>(confirm->pBuffer + 4)  = client_serial; // == slot_id+1 we stamped
	*reinterpret_cast<uint32*>(confirm->pBuffer + 12) = stack;         // >= stack removes the row
	client->FastQueuePacket(&confirm);

	LogDebug("DragonHoard::HandleRetrieve account_id {} slot {} retrieved item_id {}", account_id, target_slot, item_id);
}

void DragonHoard::SendItemUpdate(Client* client, uint32 slot_id, uint32 item_id, bool remove)
{
	if (!IsEnabled(client)) {
		return;
	}

	(void)slot_id;
	(void)item_id;
	(void)remove;

	// TODO: send single item add/remove delta update to DH window
}
