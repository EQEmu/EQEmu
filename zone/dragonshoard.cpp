#include "dragonshoard.h"
#include "client.h"
#include "../common/global_define.h"
#include "../common/item_instance.h"
#include "../common/rulesys.h" // [DH_RULE]
#include "../common/strings.h"
#include <cstring> // [DH_UNLOCK] memset for DH unlock packets
#if __has_include("../common/repositories/dragonhoard_items_repository.h")
#include "../common/repositories/dragonhoard_items_repository.h"
#else
// TODO: create repository.
#endif

// Dragon's Hoard feature handler
// Universal implementation - patch-agnostic logic
// Opcodes (TOB): OP_DragonHoard1=0x5807 (window/list), OP_DragonHoard2=0x603D (deposit/retrieve)
// Item packet type: ItemPacketDragonHoard=0x77
// Feature unlock slot: entry[29] DragonHoardSlots=200

void DragonHoard::SendItemList(Client* client)
{
	if (!client) {
		return;
	}

	if (!RuleB(Features, DragonHoardEnabled)) {
		return;
	}

	const uint32 account_id = client->AccountID();
	//client->Message(Chat::Yellow, "[DH] SendItemList called for account_id %u", account_id); // [DH_DEBUG_MSG_REMOVED]

	// [DH_SEND_ITEM_LIST]
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

	//client->Message(Chat::Yellow, "[DH] Query returned %i rows", results.RowCount()); // [DH_DEBUG_MSG_REMOVED]

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

		client->SendItemPacket(slot_id, inst, ItemPacketType::ItemPacketDragonHoard);
		safe_delete(inst);
	}

	LogDebug("DragonHoard::SendItemList sent items to account_id {}", account_id);
}

// [DH_UNLOCK] Send feature enable and slot count to client
void DragonHoard::SendUnlock(Client* client)
{
	if (!client) { // [DH_UNLOCK]
		return;
	}
	if (!RuleB(Features, DragonHoardEnabled)) { // [DH_UNLOCK]
		return;
	}

	// [DH_UNLOCK] action=8 — sets enabled flag at qword_140E65D90 + 0x2468
	auto* outapp = new EQApplicationPacket(OP_DragonHoard1, 5);
	memset(outapp->pBuffer, 0, 5); // [DH_UNLOCK]
	*reinterpret_cast<uint32_t*>(outapp->pBuffer) = 8; // [DH_UNLOCK]
	outapp->pBuffer[4] = 0; // [DH_UNLOCK]
	// [DH_UNLOCK_DEBUG]
	client->Message(Chat::Yellow, "[DH] SendUnlock action=8 byte[0-4]: %u %u %u %u %u",
		outapp->pBuffer[0], outapp->pBuffer[1], outapp->pBuffer[2], outapp->pBuffer[3], outapp->pBuffer[4]);

	// [DH_UNLOCK] action=2 — sets max slots at qword_140E65D90 + 0x246C
	auto* outapp2 = new EQApplicationPacket(OP_DragonHoard1, 8);
	memset(outapp2->pBuffer, 0, 8); // [DH_UNLOCK]
	*reinterpret_cast<uint32_t*>(outapp2->pBuffer) = 2; // [DH_UNLOCK]
	*reinterpret_cast<uint32_t*>(outapp2->pBuffer + 4) = static_cast<uint32_t>(MAX_SLOTS); // [DH_UNLOCK]

	// [DH_UNLOCK_DEBUG2]
	uint8_t dbg_byte4 = outapp->pBuffer[4];
	uint32_t dbg_maxslots = *((uint32_t*)outapp2->pBuffer + 1);

	client->FastQueuePacket(&outapp); // [DH_UNLOCK]
	client->FastQueuePacket(&outapp2); // [DH_UNLOCK]

	// [DH_OLD] client->Message(Chat::Yellow, "[DH] Sent action=8 packet size=5 byte4=%u", outapp->pBuffer[4]);
	// [DH_OLD] client->Message(Chat::Yellow, "[DH] Sent action=2 packet size=8 maxslots=%u", *((uint32_t*)outapp2->pBuffer + 1));

	// [DH_UNLOCK_DEBUG2]
	client->Message(Chat::Yellow, "[DH] Sent action=8 size=5 byte4=%u", dbg_byte4);
	client->Message(Chat::Yellow, "[DH] Sent action=2 size=8 maxslots=%u", dbg_maxslots);

	LogDebug("DragonHoard::SendUnlock sent to {}", client->GetName()); // [DH_UNLOCK]
}

void DragonHoard::HandleDeposit(Client* client, const EQApplicationPacket* app)
{
	// [DH_DEPOSIT_RETRIEVE]
	// OP_DragonHoard2 (0x603D) — client deposits cursor item into Dragon's Hoard
	if (!client || !app) {
		return;
	}

	if (!RuleB(Features, DragonHoardEnabled)) { // [DH_RULE]
		return;
	}

	// [DH_OLD] if (app->size < sizeof(DragonHoard_Struct)) {
	// [DH_OLD] 	LogError("DragonHoard::HandleDeposit bad packet size {} from {}", app->size, client->GetName());
	// [DH_OLD] 	return;
	// [DH_OLD] }

	// [DH_ACTION_ROUTE] deposit packet layout: action=4 (uint32), item_id (uint64)
	// Minimum size is 12 bytes
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

	// Send item to DH window
	EQ::ItemInstance* inst = database.CreateItem(item_data, static_cast<int16>(stack));
	if (inst) {
		client->SendItemPacket(slot_id, inst, ItemPacketType::ItemPacketDragonHoard);
		safe_delete(inst);
	}

	LogDebug("DragonHoard::HandleDeposit account_id {} deposited item_id {} slot {}", account_id, item_id, slot_id);
}

void DragonHoard::HandleRetrieve(Client* client, const EQApplicationPacket* app)
{
	// [DH_DEPOSIT_RETRIEVE]
	// OP_DragonHoard1 (0x5807) — client retrieves item from Dragon's Hoard to cursor
	if (!client || !app) {
		return;
	}

	if (!RuleB(Features, DragonHoardEnabled)) { // [DH_RULE]
		return;
	}

	// [DH_RETRIEVE_DEBUG] dump raw packet
	client->Message(Chat::Yellow, "[DH] Retrieve packet size: %u", app->size);
	if (app->size >= 4) {
		uint32 raw0 = *(uint32*)app->pBuffer;
		client->Message(Chat::Yellow, "[DH] Raw uint32[0]: %u (0x%08X)", raw0, raw0);
	}
	if (app->size >= 8) {
		uint32 raw1 = *(uint32*)(app->pBuffer + 4);
		client->Message(Chat::Yellow, "[DH] Raw uint32[1]: %u (0x%08X)", raw1, raw1);
	}
	if (app->size >= 12) {
		uint32 raw2 = *(uint32*)(app->pBuffer + 8);
		client->Message(Chat::Yellow, "[DH] Raw uint32[2]: %u (0x%08X)", raw2, raw2);
	}

	// [DH_OLD] if (app->size < sizeof(DragonHoard_Struct)) {
	// [DH_OLD] 	// [DH_NO_RESEND] window open request — items already sent on zone-in, do not resend
	// [DH_OLD] 	return;
	// [DH_OLD] }

	// [DH_OLD] auto* dh = (DragonHoard_Struct*)app->pBuffer;
	// [DH_OLD] // [DH_RETRIEVE_FIX] client sends alphabetical list index, not slot_id
	// [DH_OLD] uint32 list_index = dh->list_index;

	// [DH_ACTION_ROUTE] retrieve packet layout:
	// +0x00 uint32 action=3
	// +0x04 uint64 item_id
	// +0x0C uint32 list_index (1-based)
	if (app->size < 16) {
		return;
	}
	uint32_t list_index = *(uint32_t*)(app->pBuffer + 12);

	const uint32 account_id = client->AccountID();

	auto result = database.QueryDatabase(
		fmt::format(
			"SELECT slot_id, item_id, stack_count FROM dragonhoard_items "
			"WHERE account_id = {} ORDER BY item_name LIMIT 1 OFFSET {}",
			account_id, list_index
		)
	);

	if (!result.Success() || result.RowCount() == 0) {
		LogError("DragonHoard::HandleRetrieve list_index {} not found for account_id {}", list_index, account_id);
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

	LogDebug("DragonHoard::HandleRetrieve account_id {} list_index {} retrieved item_id {} from slot {}", account_id, list_index, item_id, slot_id);
}

void DragonHoard::SendItemUpdate(Client* client, uint32 slot_id, uint32 item_id, bool remove)
{
	if (!client) {
		return;
	}

	(void)slot_id;
	(void)item_id;
	(void)remove;

	// TODO: send single item add/remove delta update to DH window
}
