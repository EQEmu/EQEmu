#include "evolving_items.h"

#include "common/events/player_event_logs.h"
#include "common/item_instance.h"
#include "common/repositories/character_evolving_items_repository.h"

EvolvingItemsManager::EvolvingItemsManager()
{
	m_db         = nullptr;
	m_content_db = nullptr;
}

void EvolvingItemsManager::LoadEvolvingItems() const
{
	auto const &results = ItemsEvolvingDetailsRepository::All(*m_content_db);

	if (results.empty()) {
		return;
	}

	std::ranges::transform(
		results.begin(),
		results.end(),
		std::inserter(
			EvolvingItemsManager::Instance()->GetEvolvingItemsCache(),
			EvolvingItemsManager::Instance()->GetEvolvingItemsCache().end()
		),
		[](const ItemsEvolvingDetailsRepository::ItemsEvolvingDetails &x) {
			return std::make_pair(x.item_id, x);
		}
	);
}

void EvolvingItemsManager::SetDatabase(Database *db)
{
	m_db = db;
}

void EvolvingItemsManager::SetContentDatabase(Database *db)
{
	m_content_db = db;
}

double EvolvingItemsManager::CalculateProgression(const uint64 current_amount, const uint32 item_id)
{
	if (!EvolvingItemsManager::Instance()->GetEvolvingItemsCache().contains(item_id)) {
		return 0;
	}

	return EvolvingItemsManager::Instance()->GetEvolvingItemsCache().at(item_id).required_amount > 0
		? static_cast<double>(current_amount)
		  / static_cast<double>(EvolvingItemsManager::Instance()->GetEvolvingItemsCache().at(item_id).required_amount) * 100
		: 0;
}

void EvolvingItemsManager::DoLootChecks(const uint32 char_id, const uint16 slot_id, const EQ::ItemInstance &inst) const
{
	if (!inst) {
		return;
	}

	inst.SetEvolveEquipped(false);
	if (inst.IsEvolving() && slot_id <= EQ::invslot::EQUIPMENT_END && slot_id >= EQ::invslot::EQUIPMENT_BEGIN) {
		inst.SetEvolveEquipped(true);
	}

	if (!inst.IsEvolving()) {
		return;
	}

	if (!inst.GetEvolveUniqueID()) {
		auto e = CharacterEvolvingItemsRepository::NewEntity();

		e.character_id  = char_id;
		e.item_id       = inst.GetID();
		e.equipped      = inst.GetEvolveEquipped();
		e.final_item_id = EvolvingItemsManager::Instance()->GetFinalItemID(inst);
		if (inst.GetEvolveCurrentAmount() > 0) {
			e.current_amount = inst.GetEvolveCurrentAmount();
			inst.CalculateEvolveProgression();
		}

		auto r = CharacterEvolvingItemsRepository::InsertOne(*m_db, e);
		e.id   = r.id;

		inst.SetEvolveUniqueID(e.id);
		inst.SetEvolveCharID(e.character_id);
		inst.SetEvolveItemID(e.item_id);
		inst.SetEvolveFinalItemID(e.final_item_id);

		return;
	}

	CharacterEvolvingItemsRepository::SetEquipped(*m_db, inst.GetEvolveUniqueID(), inst.GetEvolveEquipped());
}

uint32 EvolvingItemsManager::GetFinalItemID(const EQ::ItemInstance &inst) const
{
	if (!inst) {
		return 0;
	}

	const auto start_iterator = std::ranges::find_if(
		EvolvingItemsManager::Instance()->GetEvolvingItemsCache().cbegin(),
		EvolvingItemsManager::Instance()->GetEvolvingItemsCache().cend(),
		[&](const std::pair<uint32, ItemsEvolvingDetailsRepository::ItemsEvolvingDetails> &a) {
			return a.second.item_evo_id == inst.GetEvolveLoreID();
		}
	);

	if (start_iterator == std::end(EvolvingItemsManager::Instance()->GetEvolvingItemsCache())) {
		return 0;
	}

	const auto final_id = std::ranges::max_element(
		start_iterator,
		EvolvingItemsManager::Instance()->GetEvolvingItemsCache().cend(),
		[&](
		const std::pair<uint32, ItemsEvolvingDetailsRepository::ItemsEvolvingDetails> &a,
		const std::pair<uint32, ItemsEvolvingDetailsRepository::ItemsEvolvingDetails> &b
	) {
			return a.second.item_evo_id == b.second.item_evo_id &&
			       a.second.item_evolve_level < b.second.item_evolve_level;
		}
	);

	return final_id->first;
}

uint32 EvolvingItemsManager::GetNextEvolveItemID(const EQ::ItemInstance &inst) const
{
	if (!inst) {
		return 0;
	}

	int8 const current_level = inst.GetEvolveLvl();

	const auto iterator = std::ranges::find_if(
		EvolvingItemsManager::Instance()->GetEvolvingItemsCache().cbegin(),
		EvolvingItemsManager::Instance()->GetEvolvingItemsCache().cend(),
		[&](const std::pair<uint32, ItemsEvolvingDetailsRepository::ItemsEvolvingDetails> &a) {
			return a.second.item_evo_id == inst.GetEvolveLoreID() &&
			       a.second.item_evolve_level == current_level + 1;
		}
	);

	if (iterator == std::end(EvolvingItemsManager::Instance()->GetEvolvingItemsCache())) {
		return 0;
	}

	return iterator->first;
}

ItemsEvolvingDetailsRepository::ItemsEvolvingDetails EvolvingItemsManager::GetEvolveItemDetails(const uint64 unique_id)
{
	if (GetEvolvingItemsCache().contains(unique_id)) {
		return GetEvolvingItemsCache().at(unique_id);
	}

	return ItemsEvolvingDetailsRepository::NewEntity();
}

std::vector<ItemsEvolvingDetailsRepository::ItemsEvolvingDetails> EvolvingItemsManager::GetEvolveIDItems(
	const uint32 evolve_id
)
{
	std::vector<ItemsEvolvingDetailsRepository::ItemsEvolvingDetails> e{};

	for (auto const &[key, value]: GetEvolvingItemsCache()) {
		if (value.item_evo_id == evolve_id) {
			e.push_back(value);
		}
	}

	std::ranges::sort(
		e.begin(),
		e.end(),
		[&](
		ItemsEvolvingDetailsRepository::ItemsEvolvingDetails const &a,
		ItemsEvolvingDetailsRepository::ItemsEvolvingDetails const &b
	) {
			return a.item_evolve_level < b.item_evolve_level;
		}
	);

	return e;
}

uint64 EvolvingItemsManager::GetTotalEarnedXP(const EQ::ItemInstance &inst)
{
	if (!inst) {
		return 0;
	}

	uint64 xp                 = inst.GetEvolveCurrentAmount();
	auto evolve_id_item_cache = GetEvolveIDItems(inst.GetEvolveLoreID());
	auto current_level        = inst.GetEvolveLvl();

	for (auto const &i: evolve_id_item_cache) {
		if (i.item_evolve_level < current_level) {
			xp += i.required_amount;
		}
	}

	return xp;
}

EvolveGetNextItem EvolvingItemsManager::GetNextItemByXP(const EQ::ItemInstance &inst_in, const int64 in_xp)
{
	EvolveGetNextItem ets{};
	if (!inst_in) {
		return ets;
	}

	const auto        evolve_items   = GetEvolveIDItems(inst_in.GetEvolveLoreID());
	uint32            max_transfer_level = 0;
	int64             xp                  = in_xp;

	for (auto const &e: evolve_items) {
		if (e.item_evolve_level < inst_in.GetEvolveLvl()) {
			continue;
		}

		int64 have = 0;
		if (e.item_evolve_level == inst_in.GetEvolveLvl()) {
			have = inst_in.GetEvolveCurrentAmount();
		}

		const auto required = e.required_amount;
		const int64 need    = required - have;
		const int64 balance = xp - need;

		if (balance <= 0) {
			ets.new_current_amount  = have + xp;
			ets.new_item_id         = e.item_id;
			ets.from_current_amount = 0;
			ets.max_transfer_level  = max_transfer_level;
			return ets;
		}

		xp = balance;
		max_transfer_level += 1;

		ets.new_current_amount  = required;
		ets.new_item_id         = e.item_id;
		ets.from_current_amount = balance - required;
		ets.max_transfer_level  = max_transfer_level;
	}

	return ets;
}

EvolveTransfer EvolvingItemsManager::DetermineTransferResults(
	const EQ::ItemInstance &inst_from,
	const EQ::ItemInstance &inst_to
)
{
	EvolveTransfer ets{};
	if (!inst_from || !inst_to) {
		return ets;
	}

	auto evolving_details_inst_from = EvolvingItemsManager::Instance()->GetEvolveItemDetails(inst_from.GetID());
	auto evolving_details_inst_to   = EvolvingItemsManager::Instance()->GetEvolveItemDetails(inst_to.GetID());

	if (!evolving_details_inst_from.id || !evolving_details_inst_to.id) {
		return ets;
	}

	if (evolving_details_inst_from.type == evolving_details_inst_to.type) {
		uint32 compatibility = 0;
		uint64 xp            = 0;
		if (evolving_details_inst_from.sub_type == evolving_details_inst_to.sub_type) {
			compatibility = 100;
		}
		else {
			compatibility = 30;
		}

		xp           = EvolvingItemsManager::Instance()->GetTotalEarnedXP(inst_from) * compatibility / 100;
		auto results = EvolvingItemsManager::Instance()->GetNextItemByXP(inst_to, xp);

		ets.item_from_id             = EvolvingItemsManager::Instance()->GetFirstItemInLoreGroup(inst_from.GetEvolveLoreID());
		ets.item_from_current_amount = results.from_current_amount;
		ets.item_to_id               = results.new_item_id;
		ets.item_to_current_amount   = results.new_current_amount;
		ets.compatibility            = compatibility;
		ets.max_transfer_level       = results.max_transfer_level;
	}

	return ets;
}

uint32 EvolvingItemsManager::GetFirstItemInLoreGroup(const uint32 lore_id)
{
	for (auto const &[key, value]: GetEvolvingItemsCache()) {
		if (value.item_evo_id == lore_id && value.item_evolve_level == 1) {
			return key;
		}
	}

	return 0;
}

uint32 EvolvingItemsManager::GetFirstItemInLoreGroupByItemID(const uint32 item_id)
{
	for (auto const &[key, value]: GetEvolvingItemsCache()) {
		if (value.item_id == item_id) {
			for (auto const &[key2, value2]: GetEvolvingItemsCache()) {
				if (value2.item_evo_id == value.item_evo_id && value2.item_evolve_level == 1) {
					return key;
				}
			}
		}
	}

	return 0;
}

void EvolvingItemsManager::LoadPlayerEvent(const EQ::ItemInstance &inst, PlayerEvent::EvolveItem &e)
{
	if (!inst) {
		return;
	}

	e.item_id     = inst.GetID();
	e.item_name   = inst.GetItem() ? inst.GetItem()->Name : std::string();
	e.level       = inst.GetEvolveLvl();
	e.progression = inst.GetEvolveProgression();
	e.unique_id   = inst.GetEvolveUniqueID();
}
