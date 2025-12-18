#include "common/events/player_event_logs.h"
#include "zone/client.h"
#include "zone/queryserv.h"
#include "zone/string_ids.h"
#include "zone/worldserver.h"

extern QueryServ  *QServ;
extern WorldServer worldserver;

void SendParcelsSubCommands(Client *c)
{
	c->Message(Chat::White, "#parcels listdb [Character Name]");
	c->Message(Chat::White, "#parcels listmemory [Character Name] (Must be in the same zone)");
	c->Message(
		Chat::White,
		"#parcels add [Character Name] [item id] [quantity] [note].  To send money use item id of 99990. Quantity is valid for stackable items, charges on an item, or amount of copper."
	);
	c->Message(Chat::White, "#parcels details [Character Name]");
}

void command_parcels(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (!arguments) {
		SendParcelsSubCommands(c);
		return;
	}

	bool is_listdb     = !strcasecmp(sep->arg[1], "listdb");
	bool is_listmemory = !strcasecmp(sep->arg[1], "listmemory");
	bool is_details    = !strcasecmp(sep->arg[1], "details");
	bool is_add        = !strcasecmp(sep->arg[1], "add");

	if (!is_listdb && !is_listmemory && !is_details && !is_add) {
		SendParcelsSubCommands(c);
		return;
	}

	if (is_listdb) {
		auto player_name = std::string(sep->arg[2]);
		if (arguments < 2) {
			c->Message(Chat::White, "Usage: #parcels listdb [Character Name]");
		}

		if (player_name.empty()) {
			c->Message(
				Chat::White,
				fmt::format("You must provide a player name.").c_str());
			return;
		}

		auto player_id = CharacterParcelsRepository::GetParcelCountAndCharacterName(database, player_name);
		if (!player_id.at(0).char_id) {
			c->MessageString(Chat::Yellow, CANT_FIND_PLAYER, player_name.c_str());
			return;
		}

		auto results = CharacterParcelsRepository::GetWhere(
			database,
			fmt::format("char_id = '{}' ORDER BY slot_id ASC", player_id.at(0).char_id)
		);

		if (results.empty()) {
			c->Message(Chat::White, fmt::format("No parcels could be found for {}", player_name).c_str());
			return;
		}

		c->Message(Chat::Yellow, fmt::format("Found {} parcels for {}.", results.size(), player_name).c_str());

		for (auto const &p: results) {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"Slot [{:02}] has item id [{:10}] with quantity [{}].",
					p.slot_id,
					p.item_id,
					p.quantity
				).c_str()
			);
		}
	}
	if (is_listmemory) {
		auto player_name = std::string(sep->arg[2]);
		auto player      = entity_list.GetClientByName(player_name.c_str());

		if (arguments < 2) {
			c->Message(Chat::White, "Usage: #parcels listmemory [Character Name] (Must be in the same zone)");
		}

		if (!player) {
			c->Message(
				Chat::White,
				fmt::format(
					"Player {} could not be found in this zone.  Ensure you are in the same zone as the player.",
					player_name
				).c_str()
			);
			return;
		}

		auto parcels = player->GetParcels();
		if (parcels.empty()) {
			c->Message(Chat::White, fmt::format("No parcels could be found for {}", player_name).c_str());
			return;
		}

		c->Message(Chat::Yellow, fmt::format("Found {} parcels for {}.", parcels.size(), player_name).c_str());
		for (auto const &p: parcels) {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"Slot [{:02}] has item id [{:10}] with quantity [{}].",
					p.second.slot_id,
					p.second.item_id,
					p.second.quantity
				).c_str()
			);
		}
	}
	if (is_add) {
		if (arguments < 4) {
			SendParcelsSubCommands(c);
			return;
		}

		if (!Strings::IsNumber(sep->arg[3]) || !Strings::IsNumber(sep->arg[4])) {
			SendParcelsSubCommands(c);
			return;
		}

		auto to_name  = std::string(sep->arg[2]);
		auto item_id  = Strings::ToUnsignedInt(sep->arg[3]);
		auto quantity = Strings::ToUnsignedInt(sep->arg[4]);
		auto note     = std::string(sep->argplus[5]);

		auto send_to_client = CharacterParcelsRepository::GetParcelCountAndCharacterName(
			database,
			to_name
		);
		if (send_to_client.at(0).character_name.empty()) {
			c->MessageString(Chat::Yellow, CANT_FIND_PLAYER, to_name.c_str());
			return;
		}

		auto next_slot = c->FindNextFreeParcelSlot(send_to_client.at(0).char_id);
		if (next_slot == INVALID_INDEX) {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"Unfortunately, {} cannot accept any more parcels at this time. Please try again later.",
					send_to_client.at(0).character_name
				).c_str()
			);
			return;
		}

		if (item_id == PARCEL_MONEY_ITEM_ID) {
			if (quantity > INT32_MAX) {
				c->Message(
					Chat::Yellow,
					"Your quantity of {} copper pieces was too large.  Set to max quantity of {}.",
					quantity,
					INT32_MAX
				);
				quantity = INT32_MAX;
			}

			auto item = database.GetItem(PARCEL_MONEY_ITEM_ID);
			if (!item) {
				c->Message(Chat::Yellow, "Could not find item with id {}", item_id);
				return;
			}

			std::unique_ptr<EQ::ItemInstance> inst(database.CreateItem(item, 1));
			if (!inst) {
				c->Message(Chat::Yellow, "Could not find item with id {}", item_id);
				return;
			}

			auto parcel_out = CharacterParcelsRepository::NewEntity();
			parcel_out.from_name = c->GetName();
			parcel_out.note      = note;
			parcel_out.sent_date = time(nullptr);
			parcel_out.quantity  = quantity == 0 ? 1 : quantity;
			parcel_out.item_id   = PARCEL_MONEY_ITEM_ID;
			parcel_out.char_id   = send_to_client.at(0).char_id;
			parcel_out.slot_id   = next_slot;
			parcel_out.id        = 0;

			auto result = CharacterParcelsRepository::InsertOne(database, parcel_out);
			if (!result.id) {
				LogError(
					"Failed to add parcel to database.  From {} to {} item {} quantity {}",
					parcel_out.from_name,
					send_to_client.at(0).character_name,
					parcel_out.item_id,
					parcel_out.quantity
				);
				c->Message(
					Chat::Yellow,
					"Unable to save parcel to the database. Please contact an administrator."
				);
				return;
			}

			c->MessageString(
				Chat::Yellow,
				PARCEL_DELIVERY,
				c->GetCleanName(),
				"Money",
				send_to_client.at(0).character_name.c_str()
			);

			if (inst && PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::PARCEL_SEND)) {
				PlayerEvent::ParcelSend e{};
				e.from_player_name = parcel_out.from_name;
				e.to_player_name   = send_to_client.at(0).character_name;
				e.item_id          = parcel_out.item_id;
				e.augment_1_id     = inst->GetAugmentItemID(0);
				e.augment_2_id     = inst->GetAugmentItemID(1);
				e.augment_3_id     = inst->GetAugmentItemID(2);
				e.augment_4_id     = inst->GetAugmentItemID(3);
				e.augment_5_id     = inst->GetAugmentItemID(4);
				e.augment_6_id     = inst->GetAugmentItemID(5);
				e.quantity         = parcel_out.quantity;
				e.sent_date        = parcel_out.sent_date;

				RecordPlayerEventLogWithClient (c, PlayerEvent::PARCEL_SEND, e);
			}

			Parcel_Struct ps{};
			ps.item_slot = parcel_out.slot_id;
			strn0cpy(ps.send_to, send_to_client.at(0).character_name.c_str(), sizeof(ps.send_to));

			c->SendParcelDeliveryToWorld(ps);
		}
		else {
			auto item = database.GetItem(item_id);
			if (!item) {
				c->Message(Chat::Yellow, "Could not find an item with id {}", item_id);
				return;
			}

			std::unique_ptr<EQ::ItemInstance> inst(
				database.CreateItem(
					item,
					quantity > INT16_MAX ? INT16_MAX : (int16) quantity
				)
			);
			if (!inst) {
				c->Message(Chat::Yellow, "Could not find an item with id {}", item_id);
				return;
			}

			if (inst->IsStackable()) {
				quantity = quantity > inst->GetItem()->StackSize
					? inst->GetItem()->StackSize : (int16) quantity;
			}
			else if (inst->GetItem()->MaxCharges > 0) {
				quantity = quantity >= inst->GetItem()->MaxCharges
					? inst->GetItem()->MaxCharges : (int16) quantity;
			}

			auto parcel_out = CharacterParcelsRepository::NewEntity();
			parcel_out.from_name = c->GetName();
			parcel_out.note      = note.empty() ? "" : note;
			parcel_out.sent_date = time(nullptr);
			parcel_out.quantity  = quantity;
			parcel_out.item_id   = item_id;
			parcel_out.char_id   = send_to_client.at(0).char_id;
			parcel_out.slot_id   = next_slot;
			parcel_out.id        = 0;

			auto result = CharacterParcelsRepository::InsertOne(database, parcel_out);
			if (!result.id) {
				LogError(
					"Failed to add parcel to database.  From {} to {} item {} quantity {}",
					parcel_out.from_name,
					send_to_client.at(0).character_name,
					parcel_out.item_id,
					parcel_out.quantity
				);
				c->Message(
					Chat::Yellow,
					"Unable to save parcel to the database. Please contact an administrator."
				);
				return;
			}

			c->MessageString(
				Chat::Yellow,
				PARCEL_DELIVERY,
				c->GetCleanName(),
				inst->GetItem()->Name,
				send_to_client.at(0).character_name.c_str()
			);

			if (inst && PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::PARCEL_SEND)) {
				PlayerEvent::ParcelSend e{};
				e.from_player_name = parcel_out.from_name;
				e.to_player_name   = send_to_client.at(0).character_name;
				e.item_id          = parcel_out.item_id;
				e.augment_1_id     = inst->GetAugmentItemID(0);
				e.augment_2_id     = inst->GetAugmentItemID(1);
				e.augment_3_id     = inst->GetAugmentItemID(2);
				e.augment_4_id     = inst->GetAugmentItemID(3);
				e.augment_5_id     = inst->GetAugmentItemID(4);
				e.augment_6_id     = inst->GetAugmentItemID(5);
				e.quantity         = parcel_out.quantity;
				e.sent_date        = parcel_out.sent_date;

				RecordPlayerEventLogWithClient (c, PlayerEvent::PARCEL_SEND, e);
			}

			Parcel_Struct ps{};
			ps.item_slot = parcel_out.slot_id;
			strn0cpy(ps.send_to, send_to_client.at(0).character_name.c_str(), sizeof(ps.send_to));

			c->SendParcelDeliveryToWorld(ps);
		}
	}
}
