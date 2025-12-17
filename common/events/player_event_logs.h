#pragma once

#include "common/eqemu_config.h"
#include "common/json/json_archive_single_line.h"
#include "common/repositories/player_event_aa_purchase_repository.h"
#include "common/repositories/player_event_killed_named_npc_repository.h"
#include "common/repositories/player_event_killed_npc_repository.h"
#include "common/repositories/player_event_killed_raid_npc_repository.h"
#include "common/repositories/player_event_log_settings_repository.h"
#include "common/repositories/player_event_logs_repository.h"
#include "common/repositories/player_event_loot_items_repository.h"
#include "common/repositories/player_event_merchant_purchase_repository.h"
#include "common/repositories/player_event_merchant_sell_repository.h"
#include "common/repositories/player_event_npc_handin_entries_repository.h"
#include "common/repositories/player_event_npc_handin_repository.h"
#include "common/repositories/player_event_speech_repository.h"
#include "common/repositories/player_event_trade_entries_repository.h"
#include "common/repositories/player_event_trade_repository.h"
#include "common/servertalk.h"
#include "common/timer.h"

#include "cereal/archives/json.hpp"
#include <mutex>


class PlayerEventLogs {
public:
	Database player_event_database{};

	void Init();
	bool LoadDatabaseConnection();
	void ReloadSettings();
	void LoadEtlIds();
	PlayerEventLogs *SetDatabase(Database *db);
	bool ValidateDatabaseConnection();
	bool IsEventEnabled(PlayerEvent::EventType event);

	void Process();

	// batch queue
	void AddToQueue(PlayerEventLogsRepository::PlayerEventLogs &logs);

	// main event record generic function
	// can ingest any struct event types
	template<typename T>
	std::unique_ptr<ServerPacket> RecordEvent(
		PlayerEvent::EventType t,
		const PlayerEvent::PlayerEvent &p,
		T e
	)
	{
		auto n = PlayerEventLogsRepository::NewEntity();
		FillPlayerEvent(p, n);
		n.event_type_id = t;

		std::stringstream ss;
		{
			cereal::JSONOutputArchiveSingleLine ar(ss);
			e.serialize(ar);
		}

		n.event_type_name = PlayerEvent::EventName[t];
		n.event_data      = Strings::Contains(ss.str(), "noop") ? "{}" : ss.str();
		n.created_at      = std::time(nullptr);

		auto c = PlayerEvent::PlayerEventContainer{
			.player_event = p,
			.player_event_log = n
		};

		return BuildPlayerEventPacket(c);
	}

	[[nodiscard]] const PlayerEventLogSettingsRepository::PlayerEventLogSettings * GetSettings() const;
	bool                                                                           IsEventDiscordEnabled(int32_t event_type_id);
	std::string                                                                    GetDiscordWebhookUrlFromEventType(int32_t event_type_id);

	void LoadPlayerEventSettingsFromQS(const std::vector<PlayerEventLogSettingsRepository::PlayerEventLogSettings>& settings);

	static std::string GetDiscordPayloadFromEvent(const PlayerEvent::PlayerEventContainer &e);

	struct EtlQueues {
		std::vector<PlayerEventLootItemsRepository::PlayerEventLootItems>               loot_items;
		std::vector<PlayerEventMerchantPurchaseRepository::PlayerEventMerchantPurchase> merchant_purchase;
		std::vector<PlayerEventMerchantSellRepository::PlayerEventMerchantSell>         merchant_sell;
		std::vector<PlayerEventNpcHandinRepository::PlayerEventNpcHandin>               npc_handin;
		std::vector<PlayerEventNpcHandinEntriesRepository::PlayerEventNpcHandinEntries> npc_handin_entries;
		std::vector<PlayerEventTradeRepository::PlayerEventTrade>                       trade;
		std::vector<PlayerEventTradeEntriesRepository::PlayerEventTradeEntries>         trade_entries;
		std::vector<PlayerEventSpeechRepository::PlayerEventSpeech>                     speech;
		std::vector<PlayerEventKilledNpcRepository::PlayerEventKilledNpc>               killed_npc;
		std::vector<PlayerEventKilledNamedNpcRepository::PlayerEventKilledNamedNpc>     killed_named_npc;
		std::vector<PlayerEventKilledRaidNpcRepository::PlayerEventKilledRaidNpc>       killed_raid_npc;
		std::vector<PlayerEventAaPurchaseRepository::PlayerEventAaPurchase>             aa_purchase;
	};

	static PlayerEventLogs* Instance()
	{
		static PlayerEventLogs instance;
		return &instance;
	}

private:
	struct EtlSettings {
		bool        enabled;
		std::string table_name;
		int64       next_id;
	};

	Database                                                 *m_database; // reference to database
	PlayerEventLogSettingsRepository::PlayerEventLogSettings m_settings[PlayerEvent::EventType::MAX]{};

	// batch queue is used to record events in batch
	std::vector<PlayerEventLogsRepository::PlayerEventLogs> m_record_batch_queue{};
	static void FillPlayerEvent(const PlayerEvent::PlayerEvent &p, PlayerEventLogsRepository::PlayerEventLogs &n);
	static std::unique_ptr<ServerPacket>
	BuildPlayerEventPacket(const PlayerEvent::PlayerEventContainer &e);

	std::map<PlayerEvent::EventType, EtlSettings>  m_etl_settings{};

	// timers
	Timer m_database_ping_timer; // database ping timer
	Timer m_process_batch_events_timer; // events processing timer
	Timer m_process_retention_truncation_timer; // timer for truncating events based on retention settings

	// processing
	std::mutex m_batch_queue_lock{};
	void ProcessBatchQueue();
	void ProcessRetentionTruncation();
	void SetSettingsDefaults();

public:
	std::map<PlayerEvent::EventType, EtlSettings> &GetEtlSettings() { return m_etl_settings;}
};
