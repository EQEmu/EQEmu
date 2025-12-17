#pragma once

#include "common/events/player_events.h"
#include "common/repositories/base/base_player_event_logs_repository.h"

#include "cereal/archives/json.hpp"
#include "cereal/types/vector.hpp"

#include <string>

struct DiscordField {
	std::string name;
	std::string value;
	bool        is_inline;

	// cereal
	template<class Archive>
	void serialize(Archive &ar)
	{
		ar(
			CEREAL_NVP(name),
			CEREAL_NVP(value),
			cereal::make_nvp("inline", is_inline)
		);
	}
};

struct DiscordAuthor {
	std::string name;
	std::string icon_url;
	std::string url;

	// cereal
	template<class Archive>
	void serialize(Archive &ar)
	{
		ar(
			CEREAL_NVP(name),
			CEREAL_NVP(icon_url),
			CEREAL_NVP(url)
		);
	}
};

struct DiscordEmbed {
	std::vector<DiscordField> fields;
	std::string               title;
	std::string               description;
	std::string               timestamp;
	DiscordAuthor             author;


	// cereal
	template<class Archive>
	void serialize(Archive &ar)
	{
		ar(
			CEREAL_NVP(fields),
			CEREAL_NVP(title),
			CEREAL_NVP(description),
			CEREAL_NVP(timestamp),
			CEREAL_NVP(author)
		);
	}
};

struct DiscordWebhook {
	std::vector<DiscordEmbed> embeds;
	std::string               content;
	std::string               avatar_url;

	// cereal
	template<class Archive>
	void serialize(Archive &ar)
	{
		ar(
			CEREAL_NVP(embeds),
			CEREAL_NVP(avatar_url),
			CEREAL_NVP(content)
		);
	}
};


class PlayerEventDiscordFormatter {
public:
	static std::string GetCurrentTimestamp();
	static std::string FormatEventSay(const PlayerEvent::PlayerEventContainer &c, const PlayerEvent::SayEvent &e);
	static std::string
	FormatGMCommand(const PlayerEvent::PlayerEventContainer &c, const PlayerEvent::GMCommandEvent &e);
	static void BuildDiscordField(
		std::vector<DiscordField> *f,
		const std::string &name,
		const std::string &value,
		bool is_inline = true
	);
	static void BuildBaseEmbed(
		std::vector<DiscordEmbed> *e,
		const std::vector<DiscordField> &f,
		PlayerEvent::PlayerEventContainer c
	);
	static std::string FormatWithNodata(const PlayerEvent::PlayerEventContainer &c);

	static std::string FormatAAGainedEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::AAGainedEvent &e
	);
	static std::string FormatAAPurchasedEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::AAPurchasedEvent &e
	);
	static std::string FormatDeathEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::DeathEvent &e
	);
	static std::string FormatFishSuccessEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::FishSuccessEvent &e
	);
	static std::string FormatForageSuccessEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::ForageSuccessEvent &e
	);
	static std::string FormatDestroyItemEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::DestroyItemEvent &e
	);
	static std::string FormatDiscoverItemEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::DiscoverItemEvent &e
	);
	static std::string FormatDroppedItemEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::DroppedItemEvent &e
	);
	static std::string FormatLevelGainedEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::LevelGainedEvent &e
	);
	static std::string FormatLevelLostEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::LevelLostEvent &e
	);
	static std::string FormatLootItemEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::LootItemEvent &e
	);
	static std::string FormatGroundSpawnPickupEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::GroundSpawnPickupEvent &e
	);
	static std::string FormatMerchantPurchaseEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::MerchantPurchaseEvent &e
	);
	static std::string FormatMerchantSellEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::MerchantSellEvent &e
	);
	static std::string FormatNPCHandinEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::HandinEvent &e
	);
	static std::string FormatSkillUpEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::SkillUpEvent &e
	);
	static std::string FormatTaskAcceptEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::TaskAcceptEvent &e
	);
	static std::string FormatTaskCompleteEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::TaskCompleteEvent &e
	);
	static std::string FormatTaskUpdateEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::TaskUpdateEvent &e
	);
	static std::string FormatTradeEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::TradeEvent &e
	);
	static std::string FormatTraderPurchaseEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::TraderPurchaseEvent &e
	);
	static std::string FormatTraderSellEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::TraderSellEvent &e
	);
	static std::string FormatResurrectAcceptEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::ResurrectAcceptEvent &e
	);
	static std::string FormatSplitMoneyEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::SplitMoneyEvent &e
	);
	static std::string FormatCombineEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::CombineEvent &e
	);
	static std::string FormatZoningEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::ZoningEvent &e
	);
	static DiscordWebhook BuildDiscordWebhook(
		const PlayerEvent::PlayerEventContainer &p,
		std::vector<DiscordEmbed> &embeds
	);
};
