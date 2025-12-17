#pragma once

#include "common/repositories/player_event_logs_repository.h"
#include "common/rulesys.h"
#include "common/types.h"

#include "cereal/cereal.hpp"

#include <string>

#define CEREAL_NVP_IF_NONZERO(ar, name) \
if ((name) != 0) ar(cereal::make_nvp(#name, name))

#define CEREAL_NVP_IF_NOT_EMPTY(ar, name) \
if (!(name).empty()) ar(cereal::make_nvp(#name, name))

#define CEREAL_NVP_IF_TRUE(ar, name) \
if ((name)) ar(cereal::make_nvp(#name, name))

namespace PlayerEvent {
	enum EventType {
		GM_COMMAND = 1,
		ZONING,
		AA_GAIN,
		AA_PURCHASE,
		FORAGE_SUCCESS,
		FORAGE_FAILURE,
		FISH_SUCCESS,
		FISH_FAILURE,
		ITEM_DESTROY,
		WENT_ONLINE,
		WENT_OFFLINE,
		LEVEL_GAIN,
		LEVEL_LOSS,
		LOOT_ITEM,
		MERCHANT_PURCHASE,
		MERCHANT_SELL,
		GROUP_JOIN,  // unimplemented
		GROUP_LEAVE, // unimplemented
		RAID_JOIN,   // unimplemented
		RAID_LEAVE,  // unimplemented
		GROUNDSPAWN_PICKUP,
		NPC_HANDIN,
		SKILL_UP,
		TASK_ACCEPT,
		TASK_UPDATE,
		TASK_COMPLETE,
		TRADE,
		GIVE_ITEM, // unimplemented
		SAY,
		REZ_ACCEPTED,
		DEATH,
		COMBINE_FAILURE,
		COMBINE_SUCCESS,
		DROPPED_ITEM,
		SPLIT_MONEY,
		DZ_JOIN,  // unimplemented
		DZ_LEAVE, // unimplemented
		TRADER_PURCHASE,
		TRADER_SELL,
		BANDOLIER_CREATE, // unimplemented
		BANDOLIER_SWAP,   // unimplemented
		DISCOVER_ITEM,
		POSSIBLE_HACK,
		KILLED_NPC,
		KILLED_NAMED_NPC,
		KILLED_RAID_NPC,
		ITEM_CREATION,
		GUILD_TRIBUTE_DONATE_ITEM,
		GUILD_TRIBUTE_DONATE_PLAT,
		PARCEL_SEND,
		PARCEL_RETRIEVE,
		PARCEL_DELETE,
		BARTER_TRANSACTION,
		SPEECH,
		EVOLVE_ITEM,
		GUILD_BANK_DEPOSIT,
		GUILD_BANK_WITHDRAWAL,
		GUILD_BANK_MOVE_TO_BANK_AREA,
		MAX // dont remove
	};

	// Don't ever remove items, even if they are deprecated
	// If event is deprecated just tag (Deprecated) in the name
	// If event is unimplemented just tag (Unimplemented) in the name
	// Events don't get saved to the database if unimplemented or deprecated
	// Events tagged as deprecated will get automatically removed
	static const char* EventName[EventType::MAX] = {
		"None",
		"GM Command",
		"Zoning",
		"AA Gain",
		"AA Purchase",
		"Forage Success",
		"Forage Failure",
		"Fish Success",
		"Fish Failure",
		"Item Destroy",
		"Went Online",
		"Went Offline",
		"Level Gain",
		"Level Loss",
		"Loot Item",
		"Merchant Purchase",
		"Merchant Sell",
		"Group Join (Unimplemented)",
		"Group Leave (Unimplemented)",
		"Raid Join (Unimplemented)",
		"Raid Leave (Unimplemented)",
		"Groundspawn Pickup",
		"NPC Handin",
		"Skill Up",
		"Task Accept",
		"Task Update",
		"Task Complete",
		"Trade",
		"Given Item (Unimplemented)",
		"Say",
		"Rez Accepted",
		"Death",
		"Combine Failure",
		"Combine Success",
		"Dropped Item",
		"Split Money",
		"DZ Join (Unimplemented)",
		"DZ Leave (Unimplemented)",
		"Trader Purchase",
		"Trader Sell",
		"Bandolier Create (Unimplemented)",
		"Bandolier Swap (Unimplemented)",
		"Discover Item",
		"Possible Hack",
		"Killed NPC",
		"Killed Named NPC",
		"Killed Raid NPC",
		"Item Creation",
		"Guild Tribute Donate Item",
		"Guild Tribute Donate Platinum",
		"Parcel Item Sent",
		"Parcel Item Retrieved",
		"Parcel Prune Routine",
		"Barter Transaction",
		"Player Speech",
		"Evolve Item Update",
		"Guild Bank Item Deposit",
		"Guild Bank Item Withdrawal",
		"Guild Bank Move From Deposit Area to Bank Area"
	};

	// Generic struct used by all events
	struct PlayerEvent {
		int64       account_id;
		std::string account_name;
		int64       character_id;
		std::string character_name;
		int64       guild_id;
		std::string guild_name;
		int         zone_id;
		std::string zone_short_name;
		std::string zone_long_name;
		int         instance_id;
		float       x;
		float       y;
		float       z;
		float       heading;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(account_id),
				CEREAL_NVP(account_name),
				CEREAL_NVP(character_id),
				CEREAL_NVP(character_name),
				CEREAL_NVP(guild_id),
				CEREAL_NVP(guild_name),
				CEREAL_NVP(zone_id),
				CEREAL_NVP(zone_short_name),
				CEREAL_NVP(zone_long_name),
				CEREAL_NVP(instance_id),
				CEREAL_NVP(x),
				CEREAL_NVP(y),
				CEREAL_NVP(z),
				CEREAL_NVP(heading)
			);
		}
	};

	// contains metadata in use for things like log/discord formatters
	// along with the actual event to be persisted
	struct PlayerEventContainer {
		PlayerEvent                                player_event;
		PlayerEventLogsRepository::PlayerEventLogs player_event_log;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(player_event),
				CEREAL_NVP(player_event_log)
			);
		}
	};

	// used in events with no extra data
	struct EmptyEvent {
		std::string noop; // noop, gets discard upstream

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(noop)
			);
		}
	};

	// used in Trade event
	struct ItemCreationEvent {
		int64       item_id;
		std::string item_name;
		uint16      to_slot;
		int16       charges;
		uint32      augment_1_id;
		uint32      augment_2_id;
		uint32      augment_3_id;
		uint32      augment_4_id;
		uint32      augment_5_id;
		uint32      augment_6_id;
		bool        attuned;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			if constexpr (Archive::is_saving::value) {
				save(ar);
			}
			else {
				load(ar);
			}
		}

		template <class Archive>
		void save(Archive& ar) const
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(to_slot),
				CEREAL_NVP(charges)
			);

			CEREAL_NVP_IF_NONZERO(ar, augment_1_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_2_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_3_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_4_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_5_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_6_id);
			CEREAL_NVP_IF_TRUE(ar, attuned);
		}

		template <class Archive>
		void load(Archive& ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(to_slot),
				CEREAL_NVP(charges),
				CEREAL_NVP(augment_1_id),
				CEREAL_NVP(augment_2_id),
				CEREAL_NVP(augment_3_id),
				CEREAL_NVP(augment_4_id),
				CEREAL_NVP(augment_5_id),
				CEREAL_NVP(augment_6_id),
				CEREAL_NVP(attuned)
			);
		}
	};

	// used in Trade event
	// struct TradeItem {
	// 	int64       item_id;
	// 	std::string item_name;
	// 	int32       slot;
	//
	// 	// cereal
	// 	template<class Archive>
	// 	void serialize(Archive &ar)
	// 	{
	// 		ar(
	// 			CEREAL_NVP(item_id),
	// 			CEREAL_NVP(item_name),
	// 			CEREAL_NVP(slot)
	// 		);
	// 	}
	// };

	// used in Trade event
	class TradeItemEntry {
	public:
		uint16      slot;
		uint32      item_id;
		uint32      augment_1_id;
		std::string augment_1_name;
		uint32      augment_2_id;
		std::string augment_2_name;
		uint32      augment_3_id;
		std::string augment_3_name;
		uint32      augment_4_id;
		std::string augment_4_name;
		uint32      augment_5_id;
		std::string augment_5_name;
		uint32      augment_6_id;
		std::string augment_6_name;
		std::string item_name;
		uint16      charges;
		bool        in_bag;


		// cereal
		template <class Archive>
		void save(Archive& ar) const
		{
			ar(
				CEREAL_NVP(slot),
				CEREAL_NVP(item_id)
			);

			CEREAL_NVP_IF_NONZERO(ar, augment_1_id);
			CEREAL_NVP_IF_NOT_EMPTY(ar, augment_1_name);
			CEREAL_NVP_IF_NONZERO(ar, augment_2_id);
			CEREAL_NVP_IF_NOT_EMPTY(ar, augment_2_name);
			CEREAL_NVP_IF_NONZERO(ar, augment_3_id);
			CEREAL_NVP_IF_NOT_EMPTY(ar, augment_3_name);
			CEREAL_NVP_IF_NONZERO(ar, augment_4_id);
			CEREAL_NVP_IF_NOT_EMPTY(ar, augment_4_name);
			CEREAL_NVP_IF_NONZERO(ar, augment_5_id);
			CEREAL_NVP_IF_NOT_EMPTY(ar, augment_5_name);
			CEREAL_NVP_IF_NONZERO(ar, augment_6_id);
			CEREAL_NVP_IF_NOT_EMPTY(ar, augment_6_name);

			ar(
				CEREAL_NVP(item_name),
				CEREAL_NVP(charges)
			);

			CEREAL_NVP_IF_TRUE(ar, in_bag);
		}

		template <class Archive>
		void load(Archive& ar)
		{
			ar(
				CEREAL_NVP(slot),
				CEREAL_NVP(item_id),
				CEREAL_NVP(augment_1_id),
				CEREAL_NVP(augment_1_name),
				CEREAL_NVP(augment_2_id),
				CEREAL_NVP(augment_2_name),
				CEREAL_NVP(augment_3_id),
				CEREAL_NVP(augment_3_name),
				CEREAL_NVP(augment_4_id),
				CEREAL_NVP(augment_4_name),
				CEREAL_NVP(augment_5_id),
				CEREAL_NVP(augment_5_name),
				CEREAL_NVP(augment_6_id),
				CEREAL_NVP(augment_6_name),
				CEREAL_NVP(item_name),
				CEREAL_NVP(charges),
				CEREAL_NVP(in_bag)
			);
		}
	};

	/**
	 * Events
	 */
	struct Money {
		int32 platinum;
		int32 gold;
		int32 silver;
		int32 copper;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(platinum),
				CEREAL_NVP(gold),
				CEREAL_NVP(silver),
				CEREAL_NVP(copper)
			);
		}
	};

	struct TradeEvent {
		uint32                      character_1_id;
		std::string                 character_1_name;
		uint32                      character_2_id;
		std::string                 character_2_name;
		Money                       character_1_give_money;
		Money                       character_2_give_money;
		std::vector<TradeItemEntry> character_1_give_items;
		std::vector<TradeItemEntry> character_2_give_items;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(character_1_id),
				CEREAL_NVP(character_1_name),
				CEREAL_NVP(character_2_id),
				CEREAL_NVP(character_2_name),
				CEREAL_NVP(character_1_give_money),
				CEREAL_NVP(character_2_give_money),
				CEREAL_NVP(character_1_give_items),
				CEREAL_NVP(character_2_give_items)
			);
		}
	};

	struct GMCommandEvent {
		std::string message;
		std::string target;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(message),
				CEREAL_NVP(target)
			);
		}
	};

	struct ZoningEvent {
		std::string from_zone_long_name;
		std::string from_zone_short_name;
		int32       from_zone_id;
		int32       from_instance_id;
		int32       from_instance_version;
		std::string to_zone_long_name;
		std::string to_zone_short_name;
		int32       to_zone_id;
		int32       to_instance_id;
		int32       to_instance_version;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(from_zone_long_name),
				CEREAL_NVP(from_zone_short_name),
				CEREAL_NVP(from_zone_id),
				CEREAL_NVP(from_instance_id),
				CEREAL_NVP(from_instance_version),
				CEREAL_NVP(to_zone_long_name),
				CEREAL_NVP(to_zone_short_name),
				CEREAL_NVP(to_zone_id),
				CEREAL_NVP(to_instance_id),
				CEREAL_NVP(to_instance_version)
			);
		}
	};

	struct AAGainedEvent {
		uint32 aa_gained;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(CEREAL_NVP(aa_gained));
		}
	};

	struct AAPurchasedEvent {
		uint32 aa_id;
		int32  aa_cost;
		int32  aa_previous_id;
		int32  aa_next_id;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(aa_id),
				CEREAL_NVP(aa_cost),
				CEREAL_NVP(aa_previous_id),
				CEREAL_NVP(aa_next_id)
			);
		}
	};

	struct ForageSuccessEvent {
		uint32      item_id;
		uint32      augment_1_id;
		uint32      augment_2_id;
		uint32      augment_3_id;
		uint32      augment_4_id;
		uint32      augment_5_id;
		uint32      augment_6_id;
		std::string item_name;

		template <class Archive>
		void serialize(Archive& ar)
		{
			if constexpr (Archive::is_saving::value) {
				save(ar);
			}
			else {
				load(ar);
			}
		}

		template <class Archive>
		void save(Archive& ar) const
		{
			ar(CEREAL_NVP(item_id));

			CEREAL_NVP_IF_NONZERO(ar, augment_1_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_2_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_3_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_4_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_5_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_6_id);

			ar(CEREAL_NVP(item_name));
		}

		template <class Archive>
		void load(Archive& ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(augment_1_id),
				CEREAL_NVP(augment_2_id),
				CEREAL_NVP(augment_3_id),
				CEREAL_NVP(augment_4_id),
				CEREAL_NVP(augment_5_id),
				CEREAL_NVP(augment_6_id),
				CEREAL_NVP(item_name)
			);
		}
	};

	struct FishSuccessEvent {
		uint32      item_id;
		uint32      augment_1_id;
		uint32      augment_2_id;
		uint32      augment_3_id;
		uint32      augment_4_id;
		uint32      augment_5_id;
		uint32      augment_6_id;
		std::string item_name;

		template <class Archive>
		void serialize(Archive& ar)
		{
			if constexpr (Archive::is_saving::value) {
				save(ar);
			}
			else {
				load(ar);
			}
		}

		template <class Archive>
		void save(Archive& ar) const
		{
			ar(CEREAL_NVP(item_id));

			CEREAL_NVP_IF_NONZERO(ar, augment_1_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_2_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_3_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_4_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_5_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_6_id);

			ar(CEREAL_NVP(item_name));
		}

		template <class Archive>
		void load(Archive& ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(augment_1_id),
				CEREAL_NVP(augment_2_id),
				CEREAL_NVP(augment_3_id),
				CEREAL_NVP(augment_4_id),
				CEREAL_NVP(augment_5_id),
				CEREAL_NVP(augment_6_id),
				CEREAL_NVP(item_name)
			);
		}
	};

	struct DestroyItemEvent {
		uint32      item_id;
		std::string item_name;
		int16       charges;
		uint32      augment_1_id;
		uint32      augment_2_id;
		uint32      augment_3_id;
		uint32      augment_4_id;
		uint32      augment_5_id;
		uint32      augment_6_id;
		bool        attuned;
		std::string reason;

		template <class Archive>
		void serialize(Archive& ar)
		{
			if constexpr (Archive::is_saving::value) {
				save(ar);
			}
			else {
				load(ar);
			}
		}

		template <class Archive>
		void save(Archive& ar) const
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(charges)
			);

			CEREAL_NVP_IF_NONZERO(ar, augment_1_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_2_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_3_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_4_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_5_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_6_id);

			ar(
				CEREAL_NVP(attuned),
				CEREAL_NVP(reason)
			);
		}

		template <class Archive>
		void load(Archive& ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(charges),
				CEREAL_NVP(augment_1_id),
				CEREAL_NVP(augment_2_id),
				CEREAL_NVP(augment_3_id),
				CEREAL_NVP(augment_4_id),
				CEREAL_NVP(augment_5_id),
				CEREAL_NVP(augment_6_id),
				CEREAL_NVP(attuned),
				CEREAL_NVP(reason)
			);
		}
	};


	struct LevelGainedEvent {
		uint32 from_level;
		uint8  to_level;
		int    levels_gained;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(from_level),
				CEREAL_NVP(to_level),
				CEREAL_NVP(levels_gained)
			);
		}
	};

	struct LevelLostEvent {
		uint32 from_level;
		uint8  to_level;
		int    levels_lost;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(from_level),
				CEREAL_NVP(to_level),
				CEREAL_NVP(levels_lost)
			);
		}
	};

	struct LootItemEvent {
		uint32      item_id;
		std::string item_name;
		int16       charges;
		uint32      augment_1_id;
		uint32      augment_2_id;
		uint32      augment_3_id;
		uint32      augment_4_id;
		uint32      augment_5_id;
		uint32      augment_6_id;
		uint32      npc_id;
		std::string corpse_name;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			if constexpr (Archive::is_saving::value) {
				save(ar);
			}
			else {
				load(ar);
			}
		}

		template <class Archive>
		void save(Archive& ar) const
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(charges)
			);

			CEREAL_NVP_IF_NONZERO(ar, augment_1_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_2_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_3_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_4_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_5_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_6_id);

			ar(
				CEREAL_NVP(npc_id),
				CEREAL_NVP(corpse_name)
			);
		}

		template <class Archive>
		void load(Archive& ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(charges),
				CEREAL_NVP(augment_1_id),
				CEREAL_NVP(augment_2_id),
				CEREAL_NVP(augment_3_id),
				CEREAL_NVP(augment_4_id),
				CEREAL_NVP(augment_5_id),
				CEREAL_NVP(augment_6_id),
				CEREAL_NVP(npc_id),
				CEREAL_NVP(corpse_name)
			);
		}
	};

	struct MerchantPurchaseEvent {
		uint32      npc_id;
		std::string merchant_name;
		uint32      merchant_type;
		uint32      item_id;
		std::string item_name;
		int16       charges;
		uint32      cost;
		uint32      alternate_currency_id;
		uint64      player_money_balance;
		uint64      player_currency_balance;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(npc_id),
				CEREAL_NVP(merchant_name),
				CEREAL_NVP(merchant_type),
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(charges),
				CEREAL_NVP(cost),
				CEREAL_NVP(alternate_currency_id),
				CEREAL_NVP(player_money_balance),
				CEREAL_NVP(player_currency_balance)
			);
		}
	};

	struct MerchantSellEvent {
		uint32      npc_id;
		std::string merchant_name;
		uint32      merchant_type;
		uint32      item_id;
		std::string item_name;
		int16       charges;
		uint32      cost;
		uint32      alternate_currency_id;
		uint64      player_money_balance;
		uint64      player_currency_balance;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(npc_id),
				CEREAL_NVP(merchant_name),
				CEREAL_NVP(merchant_type),
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(charges),
				CEREAL_NVP(cost),
				CEREAL_NVP(alternate_currency_id),
				CEREAL_NVP(player_money_balance),
				CEREAL_NVP(player_currency_balance)
			);
		}
	};

	struct SkillUpEvent {
		uint32      skill_id;
		int         value;
		int16       max_skill;
		std::string against_who;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(skill_id),
				CEREAL_NVP(value),
				CEREAL_NVP(max_skill),
				CEREAL_NVP(against_who)
			);
		}
	};

	struct TaskAcceptEvent {
		uint32      npc_id;
		std::string npc_name;
		uint32      task_id;
		std::string task_name;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(npc_id),
				CEREAL_NVP(npc_name),
				CEREAL_NVP(task_id),
				CEREAL_NVP(task_name)
			);
		}
	};

	struct TaskUpdateEvent {
		uint32      task_id;
		std::string task_name;
		uint32      activity_id;
		uint32      done_count;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(task_id),
				CEREAL_NVP(task_name),
				CEREAL_NVP(activity_id),
				CEREAL_NVP(done_count)
			);
		}
	};

	struct TaskCompleteEvent {
		uint32      task_id;
		std::string task_name;
		uint32      activity_id;
		uint32      done_count;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(task_id),
				CEREAL_NVP(task_name),
				CEREAL_NVP(activity_id),
				CEREAL_NVP(done_count)
			);
		}
	};

	struct GroundSpawnPickupEvent {
		uint32      item_id;
		std::string item_name;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_name)
			);
		}
	};

	struct SayEvent {
		std::string message;
		std::string target;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(message),
				CEREAL_NVP(target)
			);
		}
	};

	struct ResurrectAcceptEvent {
		std::string resurrecter_name;
		std::string spell_name;
		uint32      spell_id;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(resurrecter_name),
				CEREAL_NVP(spell_name),
				CEREAL_NVP(spell_id)
			);
		}
	};

	struct CombineEvent {
		uint32      recipe_id;
		std::string recipe_name;
		uint32      made_count;
		uint32      tradeskill_id;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(recipe_id),
				CEREAL_NVP(recipe_name),
				CEREAL_NVP(made_count),
				CEREAL_NVP(tradeskill_id)
			);
		}
	};

	struct DroppedItemEvent {
		uint32      item_id;
		uint32      augment_1_id;
		uint32      augment_2_id;
		uint32      augment_3_id;
		uint32      augment_4_id;
		uint32      augment_5_id;
		uint32      augment_6_id;
		std::string item_name;
		int16       slot_id;
		uint32      charges;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			if constexpr (Archive::is_saving::value) {
				save(ar);
			}
			else {
				load(ar);
			}
		}

		template <class Archive>
		void save(Archive& ar) const
		{
			ar(CEREAL_NVP(item_id));

			CEREAL_NVP_IF_NONZERO(ar, augment_1_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_2_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_3_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_4_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_5_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_6_id);

			ar(
				CEREAL_NVP(item_name),
				CEREAL_NVP(slot_id),
				CEREAL_NVP(charges)
			);
		}

		template <class Archive>
		void load(Archive& ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(augment_1_id),
				CEREAL_NVP(augment_2_id),
				CEREAL_NVP(augment_3_id),
				CEREAL_NVP(augment_4_id),
				CEREAL_NVP(augment_5_id),
				CEREAL_NVP(augment_6_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(slot_id),
				CEREAL_NVP(charges)
			);
		}
	};

	struct DeathEvent {
		uint32      killer_id;
		std::string killer_name;
		int64       damage;
		uint32      spell_id;
		std::string spell_name;
		int         skill_id;
		std::string skill_name;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(killer_id),
				CEREAL_NVP(killer_name),
				CEREAL_NVP(damage),
				CEREAL_NVP(spell_id),
				CEREAL_NVP(spell_name),
				CEREAL_NVP(skill_id),
				CEREAL_NVP(skill_name)
			);
		}
	};

	struct SplitMoneyEvent {
		uint32 copper;
		uint32 silver;
		uint32 gold;
		uint32 platinum;
		uint64 player_money_balance;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(copper),
				CEREAL_NVP(silver),
				CEREAL_NVP(gold),
				CEREAL_NVP(platinum),
				CEREAL_NVP(player_money_balance)
			);
		}
	};

	struct TraderPurchaseEvent {
		uint32      item_id;
		uint32      augment_1_id;
		uint32      augment_2_id;
		uint32      augment_3_id;
		uint32      augment_4_id;
		uint32      augment_5_id;
		uint32      augment_6_id;
		std::string item_name;
		uint32      trader_id;
		std::string trader_name;
		uint32      price;
		uint32      quantity;
		int32       charges;
		uint64      total_cost;
		uint64      player_money_balance;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			if constexpr (Archive::is_saving::value) {
				save(ar);
			}
			else {
				load(ar);
			}
		}

		template <class Archive>
		void save(Archive& ar) const
		{
			ar(CEREAL_NVP(item_id));

			CEREAL_NVP_IF_NONZERO(ar, augment_1_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_2_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_3_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_4_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_5_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_6_id);

			ar(
				CEREAL_NVP(item_name),
				CEREAL_NVP(trader_id),
				CEREAL_NVP(trader_name),
				CEREAL_NVP(price),
				CEREAL_NVP(quantity),
				CEREAL_NVP(charges),
				CEREAL_NVP(total_cost),
				CEREAL_NVP(player_money_balance)
			);
		}

		template <class Archive>
		void load(Archive& ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(augment_1_id),
				CEREAL_NVP(augment_2_id),
				CEREAL_NVP(augment_3_id),
				CEREAL_NVP(augment_4_id),
				CEREAL_NVP(augment_5_id),
				CEREAL_NVP(augment_6_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(trader_id),
				CEREAL_NVP(trader_name),
				CEREAL_NVP(price),
				CEREAL_NVP(quantity),
				CEREAL_NVP(charges),
				CEREAL_NVP(total_cost),
				CEREAL_NVP(player_money_balance)
			);
		}
	};

	struct TraderSellEvent {
		uint32      item_id;
		uint32      augment_1_id;
		uint32      augment_2_id;
		uint32      augment_3_id;
		uint32      augment_4_id;
		uint32      augment_5_id;
		uint32      augment_6_id;
		std::string item_name;
		uint32      buyer_id;
		std::string buyer_name;
		uint32      price;
		uint32      quantity;
		int32       charges;
		uint64      total_cost;
		uint64      player_money_balance;

		template <class Archive>
		void serialize(Archive& ar)
		{
			if constexpr (Archive::is_saving::value) {
				save(ar);
			}
			else {
				load(ar);
			}
		}

		template <class Archive>
		void save(Archive& ar) const
		{
			ar(CEREAL_NVP(item_id));

			CEREAL_NVP_IF_NONZERO(ar, augment_1_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_2_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_3_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_4_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_5_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_6_id);

			ar(
				CEREAL_NVP(item_name),
				CEREAL_NVP(buyer_id),
				CEREAL_NVP(buyer_name),
				CEREAL_NVP(price),
				CEREAL_NVP(quantity),
				CEREAL_NVP(charges),
				CEREAL_NVP(total_cost),
				CEREAL_NVP(player_money_balance)
			);
		}

		template <class Archive>
		void load(Archive& ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(augment_1_id),
				CEREAL_NVP(augment_2_id),
				CEREAL_NVP(augment_3_id),
				CEREAL_NVP(augment_4_id),
				CEREAL_NVP(augment_5_id),
				CEREAL_NVP(augment_6_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(buyer_id),
				CEREAL_NVP(buyer_name),
				CEREAL_NVP(price),
				CEREAL_NVP(quantity),
				CEREAL_NVP(charges),
				CEREAL_NVP(total_cost),
				CEREAL_NVP(player_money_balance)
			);
		}
	};

	struct DiscoverItemEvent {
		uint32      item_id;
		std::string item_name;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_name)
			);
		}
	};

	class HandinEntry {
	public:
		uint32                   item_id;
		std::string              item_name;
		std::vector<uint32>      augment_ids;
		std::vector<std::string> augment_names;
		uint16                   charges;
		bool                     attuned;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(augment_ids),
				CEREAL_NVP(augment_names),
				CEREAL_NVP(charges),
				CEREAL_NVP(attuned)
			);
		}
	};

	class HandinMoney {
	public:
		uint32 copper;
		uint32 silver;
		uint32 gold;
		uint32 platinum;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(copper),
				CEREAL_NVP(silver),
				CEREAL_NVP(gold),
				CEREAL_NVP(platinum)
			);
		}
	};

	struct HandinEvent {
		uint32                   npc_id;
		std::string              npc_name;
		std::vector<HandinEntry> handin_items;
		HandinMoney              handin_money;
		std::vector<HandinEntry> return_items;
		HandinMoney              return_money;
		bool                     is_quest_handin;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(npc_id),
				CEREAL_NVP(npc_name),
				CEREAL_NVP(handin_items),
				CEREAL_NVP(handin_money),
				CEREAL_NVP(return_items),
				CEREAL_NVP(return_money),
				CEREAL_NVP(is_quest_handin)
			);
		}
	};

	struct PossibleHackEvent {
		std::string message;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(message)
			);
		}
	};

	struct KilledNPCEvent {
		uint32      npc_id;
		std::string npc_name;
		uint32      combat_time_seconds;
		uint64      total_damage_per_second_taken;
		uint64      total_heal_per_second_taken;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(npc_id),
				CEREAL_NVP(npc_name),
				CEREAL_NVP(combat_time_seconds),
				CEREAL_NVP(total_damage_per_second_taken),
				CEREAL_NVP(total_heal_per_second_taken)
			);
		}
	};

	struct GuildTributeDonateItem {
		uint32 item_id;
		uint32 augment_1_id;
		uint32 augment_2_id;
		uint32 augment_3_id;
		uint32 augment_4_id;
		uint32 augment_5_id;
		uint32 augment_6_id;
		int16  charges;
		bool   attuned;
		uint32 guild_favor;

		template <class Archive>
		void serialize(Archive& ar)
		{
			if constexpr (Archive::is_saving::value) {
				save(ar);
			}
			else {
				load(ar);
			}
		}

		template <class Archive>
		void save(Archive& ar) const
		{
			ar(CEREAL_NVP(item_id));

			CEREAL_NVP_IF_NONZERO(ar, augment_1_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_2_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_3_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_4_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_5_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_6_id);
			CEREAL_NVP_IF_NONZERO(ar, charges);
			CEREAL_NVP_IF_TRUE(ar, attuned);

			ar(CEREAL_NVP(guild_favor));
		}

		template <class Archive>
		void load(Archive& ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(augment_1_id),
				CEREAL_NVP(augment_2_id),
				CEREAL_NVP(augment_3_id),
				CEREAL_NVP(augment_4_id),
				CEREAL_NVP(augment_5_id),
				CEREAL_NVP(augment_6_id),
				CEREAL_NVP(charges),
				CEREAL_NVP(attuned),
				CEREAL_NVP(guild_favor)
			);
		}
	};

	struct GuildTributeDonatePlat {
		uint32 plat;
		uint32 guild_favor;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(plat),
				CEREAL_NVP(guild_favor)
			);
		}
	};

	struct ParcelRetrieve {
		uint32      item_id;
		uint32      augment_1_id;
		uint32      augment_2_id;
		uint32      augment_3_id;
		uint32      augment_4_id;
		uint32      augment_5_id;
		uint32      augment_6_id;
		uint32      quantity;
		std::string from_player_name;
		uint32      sent_date;

		template <class Archive>
		void serialize(Archive& ar)
		{
			if constexpr (Archive::is_saving::value) {
				save(ar);
			}
			else {
				load(ar);
			}
		}

		template <class Archive>
		void save(Archive& ar) const
		{
			ar(CEREAL_NVP(item_id));

			CEREAL_NVP_IF_NONZERO(ar, augment_1_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_2_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_3_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_4_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_5_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_6_id);

			ar(
				CEREAL_NVP(quantity),
				CEREAL_NVP(from_player_name),
				CEREAL_NVP(sent_date)
			);
		}

		template <class Archive>
		void load(Archive& ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(augment_1_id),
				CEREAL_NVP(augment_2_id),
				CEREAL_NVP(augment_3_id),
				CEREAL_NVP(augment_4_id),
				CEREAL_NVP(augment_5_id),
				CEREAL_NVP(augment_6_id),
				CEREAL_NVP(quantity),
				CEREAL_NVP(from_player_name),
				CEREAL_NVP(sent_date)
			);
		}
	};

	struct ParcelSend {
		uint32      item_id;
		uint32      augment_1_id;
		uint32      augment_2_id;
		uint32      augment_3_id;
		uint32      augment_4_id;
		uint32      augment_5_id;
		uint32      augment_6_id;
		uint32      quantity;
		int32       charges;
		std::string from_player_name;
		std::string to_player_name;
		uint32      sent_date;

		template <class Archive>
		void serialize(Archive& ar)
		{
			if constexpr (Archive::is_saving::value) {
				save(ar);
			}
			else {
				load(ar);
			}
		}

		template <class Archive>
		void save(Archive& ar) const
		{
			ar(CEREAL_NVP(item_id));

			CEREAL_NVP_IF_NONZERO(ar, augment_1_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_2_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_3_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_4_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_5_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_6_id);
			CEREAL_NVP_IF_NONZERO(ar, charges);

			ar(
				CEREAL_NVP(quantity),
				CEREAL_NVP(from_player_name),
				CEREAL_NVP(to_player_name),
				CEREAL_NVP(sent_date)
			);
		}

		template <class Archive>
		void load(Archive& ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(augment_1_id),
				CEREAL_NVP(augment_2_id),
				CEREAL_NVP(augment_3_id),
				CEREAL_NVP(augment_4_id),
				CEREAL_NVP(augment_5_id),
				CEREAL_NVP(augment_6_id),
				CEREAL_NVP(quantity),
				CEREAL_NVP(charges),
				CEREAL_NVP(from_player_name),
				CEREAL_NVP(to_player_name),
				CEREAL_NVP(sent_date)
			);
		}
	};

	struct ParcelDelete {
		uint32      char_id;
		uint32      item_id;
		uint32      augment_1_id;
		uint32      augment_2_id;
		uint32      augment_3_id;
		uint32      augment_4_id;
		uint32      augment_5_id;
		uint32      augment_6_id;
		uint32      quantity;
		uint32      sent_date;
		std::string from_name;
		std::string note;

		template <class Archive>
		void serialize(Archive& ar)
		{
			if constexpr (Archive::is_saving::value) {
				save(ar);
			}
			else {
				load(ar);
			}
		}

		template <class Archive>
		void save(Archive& ar) const
		{
			ar(CEREAL_NVP(item_id));

			CEREAL_NVP_IF_NONZERO(ar, augment_1_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_2_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_3_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_4_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_5_id);
			CEREAL_NVP_IF_NONZERO(ar, augment_6_id);

			ar(
				CEREAL_NVP(quantity),
				CEREAL_NVP(char_id),
				CEREAL_NVP(from_name),
				CEREAL_NVP(note),
				CEREAL_NVP(sent_date)
			);
		}

		template <class Archive>
		void load(Archive& ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(augment_1_id),
				CEREAL_NVP(augment_2_id),
				CEREAL_NVP(augment_3_id),
				CEREAL_NVP(augment_4_id),
				CEREAL_NVP(augment_5_id),
				CEREAL_NVP(augment_6_id),
				CEREAL_NVP(quantity),
				CEREAL_NVP(char_id),
				CEREAL_NVP(from_name),
				CEREAL_NVP(note),
				CEREAL_NVP(sent_date)
			);
		}
	};

	struct BarterTransaction {
		std::string                             status;
		uint32                                  item_id;
		uint32                                  item_quantity;
		std::string                             item_name;
		std::vector<BuyerLineTradeItems_Struct> trade_items;
		std::string                             buyer_name;
		std::string                             seller_name;
		uint64                                  total_cost;
		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(status),
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_quantity),
				CEREAL_NVP(item_name),
				CEREAL_NVP(trade_items),
				CEREAL_NVP(buyer_name),
				CEREAL_NVP(seller_name),
				CEREAL_NVP(total_cost)
			);
		}
	};

	struct EvolveItem {
		std::string status;
		uint32      item_id;
		uint64      unique_id;
		std::string item_name;
		uint32      level;
		double      progression;

		// cereal
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(status),
				CEREAL_NVP(item_id),
				CEREAL_NVP(unique_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(level),
				CEREAL_NVP(progression)
			);
		}
	};

	struct PlayerSpeech {
		std::string to;
		std::string from;
		uint32      guild_id;
		int16       min_status;
		uint32      type;
		std::string message;

		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(
				CEREAL_NVP(to),
				CEREAL_NVP(from),
				CEREAL_NVP(guild_id),
				CEREAL_NVP(min_status),
				CEREAL_NVP(type),
				CEREAL_NVP(message)
			);
		}
	};

	struct GuildBankTransaction {
		uint32 char_id;
		uint32 guild_id;
		uint32 item_id;
		uint32 aug_slot_one;
		uint32 aug_slot_two;
		uint32 aug_slot_three;
		uint32 aug_slot_four;
		uint32 aug_slot_five;
		uint32 aug_slot_six;
		uint32 quantity;
		uint32 permission;

		template <class Archive>
		void serialize(Archive& ar)
		{
			if constexpr (Archive::is_saving::value) {
				save(ar);
			}
			else {
				load(ar);
			}
		}

		template <class Archive>
		void save(Archive& ar) const
		{
			ar(
				CEREAL_NVP(char_id),
				CEREAL_NVP(guild_id),
				CEREAL_NVP(item_id)
			);

			CEREAL_NVP_IF_NONZERO(ar, aug_slot_one);
			CEREAL_NVP_IF_NONZERO(ar, aug_slot_two);
			CEREAL_NVP_IF_NONZERO(ar, aug_slot_three);
			CEREAL_NVP_IF_NONZERO(ar, aug_slot_four);
			CEREAL_NVP_IF_NONZERO(ar, aug_slot_five);
			CEREAL_NVP_IF_NONZERO(ar, aug_slot_six);

			ar(
				CEREAL_NVP(quantity),
				CEREAL_NVP(permission)
			);
		}

		template <class Archive>
		void load(Archive& ar)
		{
			ar(
				CEREAL_NVP(char_id),
				CEREAL_NVP(guild_id),
				CEREAL_NVP(item_id),
				CEREAL_NVP(aug_slot_one),
				CEREAL_NVP(aug_slot_two),
				CEREAL_NVP(aug_slot_three),
				CEREAL_NVP(aug_slot_four),
				CEREAL_NVP(aug_slot_five),
				CEREAL_NVP(aug_slot_six),
				CEREAL_NVP(quantity),
				CEREAL_NVP(permission)
			);
		}
	};
}

#define RecordPlayerEventLog(event_type, event_data) do {\
    if (PlayerEventLogs::Instance()->IsEventEnabled(event_type)) {\
        if (RuleB(Logging, PlayerEventsQSProcess)) {\
            QServ->SendPacket(\
                PlayerEventLogs::Instance()->RecordEvent(\
                    event_type,\
                    GetPlayerEvent(),\
                    event_data\
                ).get()\
            );\
        }                                                                                                          \
        else {                                                                                                     \
            worldserver.SendPacket(\
                PlayerEventLogs::Instance()->RecordEvent(\
                    event_type,\
                    GetPlayerEvent(),\
                    event_data\
                ).get()\
            );\
        }\
    }\
} while (0)

#define RecordPlayerEventLogWithClient(c, event_type, event_data) do {\
    if (PlayerEventLogs::Instance()->IsEventEnabled(event_type)) {\
        if (RuleB(Logging, PlayerEventsQSProcess)) {\
            QServ->SendPacket(\
                PlayerEventLogs::Instance()->RecordEvent(\
                    event_type,\
                    (c)->GetPlayerEvent(),\
                    event_data\
                ).get()\
            );\
        }\
        else {\
            worldserver.SendPacket(\
                PlayerEventLogs::Instance()->RecordEvent(\
                    event_type,\
                    (c)->GetPlayerEvent(),\
                    event_data\
                ).get()\
            );\
        }\
    }\
} while (0)
