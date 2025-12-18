/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY except by those people which sell it, which
are required to give you total support for your newly bought product;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "hate_list.h"

#include "common/data_verification.h"
#include "zone/client.h"
#include "zone/entity.h"
#include "zone/groups.h"
#include "zone/mob.h"
#include "zone/quest_parser_collection.h"
#include "zone/raids.h"
#include "zone/water_map.h"
#include "zone/zone.h"

#include <list>

extern Zone *zone;

HateList::HateList()
{
	hate_owner = nullptr;
}

HateList::~HateList()
{
}

void HateList::WipeHateList(bool npc_only) {
	auto iterator = list.begin();
	while (iterator != list.end()) {
		Mob *m = (*iterator)->entity_on_hatelist;
		if (
			m &&
			(
				m->IsOfClientBotMerc() ||
				(m->IsPet() && m->GetOwner() && m->GetOwner()->IsOfClientBotMerc())
			) &&
			npc_only
		) {
			iterator++;
		} else {
			if (m) {
				if (parse->HasQuestSub(hate_owner->GetNPCTypeID(), EVENT_HATE_LIST)) {
					parse->EventNPC(EVENT_HATE_LIST, hate_owner->CastToNPC(), m, "0", 0);
				}

				if (m->IsClient()) {
					m->CastToClient()->DecrementAggroCount();
					m->CastToClient()->RemoveXTarget(hate_owner, true);
				}

				delete (*iterator);
				iterator = list.erase(iterator);
			}
		}
	}
}

bool HateList::IsEntOnHateList(Mob* m)
{
	return m ? Find(m) != nullptr : false;
}

struct_HateList* HateList::Find(Mob* m)
{
	if (!m || list.empty()) {
		return nullptr;
	}

	for (auto e : list) {
		if (e->entity_on_hatelist && e->entity_on_hatelist == m) {
			return e;
		}
	}

	return nullptr;
}

void HateList::SetHateAmountOnEnt(Mob* other, int64 in_hate, uint64 in_damage)
{
	struct_HateList *entity = Find(other);
	if (entity)
	{
		if (in_damage > 0)
			entity->hatelist_damage = in_damage;
		if (in_hate > 0)
			entity->stored_hate_amount = in_hate;
		entity->last_modified = Timer::GetCurrentTime();
	}
}

Mob* HateList::GetDamageTopOnHateList(Mob* hater)
{
	Mob*   c = nullptr;
	Mob*   m = nullptr;
	Group* g = nullptr;
	Raid*  r = nullptr;

	uint64 damage = 0;

	for (const auto& e : list) {
		c = e->entity_on_hatelist;

		if (!c) {
			continue;
		}

		g = nullptr;
		r = nullptr;

		if (c->IsBot()) {
			r = entity_list.GetRaidByBot(c->CastToBot());
		} else if (c->IsClient()) {
			r = entity_list.GetRaidByClient(c->CastToClient());
		}

		g = entity_list.GetGroupByMob(c);

		if (r) {
			if (r->GetTotalRaidDamage(hater) >= damage) {
				m = c;
				damage = r->GetTotalRaidDamage(hater);
			}
		} else if (g) {
			if (g->GetTotalGroupDamage(hater) >= damage) {
				m = c;
				damage = g->GetTotalGroupDamage(hater);
			}
		} else if (static_cast<uint64>(e->hatelist_damage) >= damage) {
			m = c;
			damage = static_cast<uint64>(e->hatelist_damage);
		}
	}

	return m;
}

Mob* HateList::GetClosestEntOnHateList(Mob *hater, bool skip_mezzed, EntityFilterType filter_type) {
	Mob* close_entity = nullptr;
	float close_distance = 99999.9f;
	float this_distance;

	for (const auto& e : list) {
		if (!e->entity_on_hatelist) {
			continue;
		}

		if (skip_mezzed && e->entity_on_hatelist->IsMezzed()) {
			continue;
		}

		switch (filter_type) {
			case EntityFilterType::Bots:
				if (!e->entity_on_hatelist->IsBot()) {
					continue;
				}
				break;
			case EntityFilterType::Clients:
				if (!e->entity_on_hatelist->IsClient()) {
					continue;
				}
				break;
			case EntityFilterType::NPCs:
				if (!e->entity_on_hatelist->IsNPC()) {
					continue;
				}
				break;
			case EntityFilterType::All:
			default:
				break;
		}

		this_distance = DistanceSquaredNoZ(e->entity_on_hatelist->GetPosition(), hater->GetPosition());
		if (this_distance <= close_distance) {
			close_distance = this_distance;
			close_entity   = e->entity_on_hatelist;
		}
	}

	if (
		(!close_entity && hater->IsNPC()) ||
		(close_entity && close_entity->DivineAura())
	) {
		close_entity = hater->CastToNPC()->GetHateTop();
	}

	return close_entity;
}

void HateList::AddEntToHateList(Mob *in_entity, int64 in_hate, int64 in_damage, bool in_is_entity_frenzied, bool iAddIfNotExist)
{
	if (!in_entity) {
		return;
	}

	if (in_entity->IsCorpse()) {
		return;
	}

	if (in_entity->IsClient() && in_entity->CastToClient()->IsDead()) {
		return;
	}

	struct_HateList *entity = Find(in_entity);
	if (entity) {
		entity->hatelist_damage += (in_damage >= 0) ? in_damage : 0;
		entity->stored_hate_amount += in_hate;
		entity->is_entity_frenzy = in_is_entity_frenzied;
		entity->last_modified = Timer::GetCurrentTime();

		LogHate(
			"AddEntToHateList in_entity [{}] ({}) in_hate [{}] in_damage [{}] stored_hate_amount [{}] hatelist_damage [{}]",
			in_entity->GetCleanName(),
			in_entity->GetID(),
			in_hate,
			in_damage,
			entity->stored_hate_amount,
			entity->hatelist_damage
		);
	} else if (iAddIfNotExist) {
		entity = new struct_HateList;
		entity->entity_on_hatelist = in_entity;
		entity->hatelist_damage = (in_damage >= 0) ? in_damage : 0;
		entity->stored_hate_amount = in_hate;
		entity->is_entity_frenzy = in_is_entity_frenzied;
		entity->oor_count = 0;
		entity->last_modified = Timer::GetCurrentTime();
		list.push_back(entity);

		if (parse->HasQuestSub(hate_owner->GetNPCTypeID(), EVENT_HATE_LIST)) {
			parse->EventNPC(EVENT_HATE_LIST, hate_owner->CastToNPC(), in_entity, "1", 0);
		}

		if (in_entity->IsClient()) {
			in_entity->CastToClient()->IncrementAggroCount(hate_owner->CastToNPC()->IsRaidTarget());
		}
	}
}

bool HateList::RemoveEntFromHateList(Mob *in_entity)
{
	if (!in_entity) {
		return false;
	}

	bool is_found = false;
	auto iterator = list.begin();

	while (iterator != list.end()) {
		if ((*iterator)->entity_on_hatelist == in_entity) {
			is_found = true;

			if (in_entity && in_entity->IsClient()) {
				in_entity->CastToClient()->DecrementAggroCount();
			}

			delete (*iterator);
			iterator = list.erase(iterator);

			if (in_entity) {
				if (parse->HasQuestSub(hate_owner->GetNPCTypeID(), EVENT_HATE_LIST)) {
					parse->EventNPC(EVENT_HATE_LIST, hate_owner->CastToNPC(), in_entity, "0", 0);
				}
			}
		} else {
			++iterator;
		}
	}
	return is_found;
}

// so if faction_id and faction_value are set, we do RewardFaction, otherwise old stuff
void HateList::DoFactionHits(int64 npc_faction_level_id, int32 faction_id, int32 faction_value) {
	if (npc_faction_level_id <= 0 && faction_id <= 0 && faction_value == 0)
		return;
	auto iterator = list.begin();
	while (iterator != list.end())
	{
		Client *client;

		if ((*iterator)->entity_on_hatelist && (*iterator)->entity_on_hatelist->IsClient())
			client = (*iterator)->entity_on_hatelist->CastToClient();
		else
			client = nullptr;

		if (client) {
			if (faction_id != 0 && faction_value != 0) {
				client->RewardFaction(faction_id, faction_value);
			} else {
				client->SetFactionLevel(client->CharacterID(), npc_faction_level_id, client->GetBaseClass(), client->GetBaseRace(), client->GetDeity());
			}
		}
		++iterator;
	}
}

int HateList::GetSummonedPetCountOnHateList() {

	//Function to get number of 'Summoned' pets on a targets hate list to allow calculations for certian spell effects.
	//Unclear from description that pets are required to be 'summoned body type'. Will not require at this time.
	int pet_count = 0;
	auto iterator = list.begin();
	while (iterator != list.end()) {

		if ((*iterator)->entity_on_hatelist != nullptr && (*iterator)->entity_on_hatelist->IsNPC() && ((*iterator)->entity_on_hatelist->CastToNPC()->IsPet() || ((*iterator)->entity_on_hatelist->CastToNPC()->GetSwarmOwner() > 0)))
		{
			++pet_count;
		}

		++iterator;
	}

	return pet_count;
}

int HateList::GetHateRatio(Mob *top, Mob *other)
{
	auto other_entry = Find(other);

	if (!other_entry || other_entry->stored_hate_amount < 1)
		return 0;

	auto top_entry = Find(top);

	if (!top_entry || top_entry->stored_hate_amount < 1)
		return 999; // shouldn't happen if you call it right :P

	return EQ::Clamp(static_cast<int>((other_entry->stored_hate_amount * 100) / top_entry->stored_hate_amount), 1, 999);
}

// skip is used to ignore a certain mob on the list
// Currently used for getting 2nd on list for aggro meter
Mob *HateList::GetMobWithMostHateOnList(
	Mob *center,
	Mob *skip,
	bool skip_mezzed,
	EntityFilterType filter_type
)
{
	if (!zone->IsLoaded()) { // hack fix for zone shutdown crashes on some servers
		return nullptr;
	}

	Mob   *top_hate = nullptr;
	int64 hate      = -1;

	if (!center) {
		return nullptr;
	}

	if (RuleB(Aggro, SmartAggroList)) {
		Mob   *top_client_type_in_range = nullptr;
		int64 hate_client_type_in_range = -1;
		int   skipped_count             = 0;

		auto iterator = list.begin();
		while (iterator != list.end()) {
			struct_HateList *cur      = (*iterator);
			int16           aggro_mod = 0;

			if (!cur) {
				++iterator;
				continue;
			}

			Mob *m = cur->entity_on_hatelist;

			if (!m) {
				++iterator;
				continue;
			}

			if (m == skip) {
				++iterator;
				continue;
			}

			if (skip_mezzed && m->IsMezzed()) {
				++iterator;
				continue;
			}

			if (
				(filter_type == EntityFilterType::Bots && !m->IsBot()) ||
				(filter_type == EntityFilterType::Clients && !m->IsClient()) ||
				(filter_type == EntityFilterType::NPCs && !m->IsNPC())
			) {
				++iterator;
				continue;
			}

			if (m->Sanctuary()) {
				if (hate == -1) {
					top_hate = m;
					hate     = 1;
				}

				++iterator;
				continue;
			}

			if (m->DivineAura() || m->IsMezzed() || m->IsFeared()) {
				if (hate == -1) {
					top_hate = m;
					hate     = 0;
				}

				++iterator;
				continue;
			}

			int64 current_hate = cur->stored_hate_amount;

			if (m->IsOfClientBot()) {
				if (m->IsClient() && m->CastToClient()->IsSitting()) {
					aggro_mod += RuleI(Aggro, SittingAggroMod);
				}

				if (center) {
					if (center->GetTarget() == m) {
						aggro_mod += RuleI(Aggro, CurrentTargetAggroMod);
					}

					if (RuleI(Aggro, MeleeRangeAggroMod) != 0) {
						if (center->CombatRange(m)) {
							aggro_mod += RuleI(Aggro, MeleeRangeAggroMod);

							if (current_hate > hate_client_type_in_range || cur->is_entity_frenzy) {
								hate_client_type_in_range = current_hate;
								top_client_type_in_range  = m;
							}
						}
					}
				}
			} else {
				if (center) {
					if (center->GetTarget() == m) {
						aggro_mod += RuleI(Aggro, CurrentTargetAggroMod);
					}

					if (RuleI(Aggro, MeleeRangeAggroMod) != 0) {
						if (center->CombatRange(m)) {
							aggro_mod += RuleI(Aggro, MeleeRangeAggroMod);
						}
					}
				}
			}

			if (m->GetMaxHP() != 0 && ((m->GetHP() * 100 / m->GetMaxHP()) < 20)) {
				aggro_mod += RuleI(Aggro, CriticallyWoundedAggroMod);
			}

			if (aggro_mod) {
				current_hate += (current_hate * aggro_mod / 100);
			}

			if (current_hate > hate || cur->is_entity_frenzy) {
				hate     = current_hate;
				top_hate = m;
			}

			++iterator;
		}

		if (top_client_type_in_range && top_hate) {
			bool is_top_client_type = top_hate->IsClient();
			if (!is_top_client_type) {
				if (top_hate->IsBot()) {
					is_top_client_type          = true;
					top_client_type_in_range = top_hate;
				}
			}

			if (!is_top_client_type) {
				if (top_hate->IsMerc()) {
					is_top_client_type          = true;
					top_client_type_in_range = top_hate;
				}
			}

			if (!is_top_client_type) {
				if (top_hate->GetSpecialAbility(SpecialAbility::AllowedToTank)) {
					is_top_client_type          = true;
					top_client_type_in_range = top_hate;
				}
			}

			if (!is_top_client_type) {
				return top_client_type_in_range ? top_client_type_in_range : nullptr;
			}

			return top_hate ? top_hate : nullptr;
		} else {
			if (!top_hate && skipped_count > 0) {
				return center->GetTarget() ? center->GetTarget() : nullptr;
			}

			return top_hate ? top_hate : nullptr;
		}
	} else {
		auto iterator      = list.begin();
		int  skipped_count = 0;
		while (iterator != list.end()) {
			struct_HateList *cur = (*iterator);
			if (cur) {
				Mob *m = cur->entity_on_hatelist;

				if (!m) {
					++iterator;
					continue;
				}

				if (m == skip) {
					++iterator;
					continue;
				}

				if (skip_mezzed && m->IsMezzed()) {
					++iterator;
					continue;
				}

				if ((cur->stored_hate_amount > hate) || cur->is_entity_frenzy) {
					top_hate = m;
					hate     = cur->stored_hate_amount;
				}
			}
			++iterator;
		}

		if (!top_hate && skipped_count > 0) {
			return center->GetTarget() ? center->GetTarget() : nullptr;
		}

		return top_hate ? top_hate : nullptr;
	}

	return nullptr;
}

Mob *HateList::GetMobWithMostHateOnList(bool skip_mezzed){
	Mob* top = nullptr;
	int64 hate = -1;

	auto iterator = list.begin();
	while (iterator != list.end())
	{
		struct_HateList *cur = (*iterator);

		if (cur) {
			LogHateDetail(
				"Looping GetMobWithMostHateOnList1 [{}] cur [{}] hate [{}] calc [{}]",
				cur->entity_on_hatelist->GetMobDescription(),
				cur->stored_hate_amount,
				hate,
				(cur->stored_hate_amount > hate)
			);

			if (cur->entity_on_hatelist != nullptr && (cur->stored_hate_amount > hate))
			{
				LogHateDetail(
					"Looping GetMobWithMostHateOnList2 [{}]",
					cur->entity_on_hatelist->GetMobDescription()
				);

				if (!skip_mezzed || !cur->entity_on_hatelist->IsMezzed()) {
					top = cur->entity_on_hatelist;
					hate = cur->stored_hate_amount;
				}
			}
		}
		++iterator;
	}
	return top;
}


Mob *HateList::GetRandomMobOnHateList(EntityFilterType filter_type)
{
	const auto &l = GetFilteredHateList(filter_type);

	int count = l.size();
	if (count <= 0) { // If we don't have any entries it'll crash getting a random 0, -1 position.
		return nullptr;
	}

	if (count == 1) { // No need to do all that extra work if we only have one hate entry
		auto c = *l.begin();
		if (c) {
			Mob *m = c->entity_on_hatelist;
			if (!m) {
				return nullptr;
			}

			return m;
		}

		return nullptr;
	}

	auto r            = l.begin();
	int  random_index = rand() % count;

	std::advance(r, random_index);

	auto e = *r;

	Mob *m = e->entity_on_hatelist;
	if (m) {
		return m;
	}

	return nullptr;
}

Mob *HateList::GetEscapingMobOnHateList(Mob *center, float range, bool first) {
	// function is still in design stage

	if (!center)
		return nullptr;

	Mob *escaping_mob = nullptr;
	float mob_distance = 0.0f;

	for (auto iter : list) {
		if (!iter->entity_on_hatelist)
			continue;

		if (!iter->entity_on_hatelist->IsFeared())
			continue;

		if (iter->entity_on_hatelist->IsRooted())
			continue;
		if (iter->entity_on_hatelist->IsMezzed())
			continue;
		if (iter->entity_on_hatelist->IsStunned())
			continue;

		float distance_test = DistanceSquared(center->GetPosition(), iter->entity_on_hatelist->GetPosition());

		if (range > 0.0f && distance_test > range)
			continue;

		if (first)
			return iter->entity_on_hatelist;

		if (distance_test > mob_distance) {
			escaping_mob = iter->entity_on_hatelist;
			mob_distance = distance_test;
		}
	}

	return escaping_mob;
}

int64 HateList::GetEntHateAmount(Mob *in_entity, bool damage)
{
	struct_HateList *entity;

	entity = Find(in_entity);

	if (entity && damage)
		return entity->hatelist_damage;
	else if (entity)
		return entity->stored_hate_amount;
	else
		return 0;
}

bool HateList::IsHateListEmpty() {
	return list.empty();
}

uint32 HateList::GetHateListCount(HateListCountType count_type)
{
	if (count_type == HateListCountType::All) {
		return list.size();
	}

	uint32 count = 0;

	for (const auto& e : list) {
		Mob* m = e->entity_on_hatelist;

		if (
			m &&
			(
				(count_type == HateListCountType::Bot && m->IsBot()) ||
				(count_type == HateListCountType::Client && m->IsClient()) ||
				(count_type == HateListCountType::NPC && m->IsNPC())
			)
		) {
			count++;
		}
	}

	return count;
}

void HateList::PrintHateListToClient(Client *c)
{
	if (list.size()) {
		c->Message(
			Chat::White,
			fmt::format(
				"Displaying hate list for {}.",
				c->GetTargetDescription(hate_owner)
			).c_str()
		);

		auto entity_number = 1;
		for (const auto& hate_entity : list) {
			if (hate_entity->entity_on_hatelist) {
				c->Message(
					Chat::White,
					fmt::format(
						"Hate Entity {} | Name: {} ({}) Damage: {} Hate: {}",
						entity_number,
						hate_entity->entity_on_hatelist->GetName(),
						hate_entity->entity_on_hatelist->GetID(),
						hate_entity->hatelist_damage,
						hate_entity->stored_hate_amount
					).c_str()
				);
			} else {
				c->Message(
					Chat::White,
					fmt::format(
						"Hate Entity {} | Damage: {} Hate: {}",
						entity_number,
						hate_entity->hatelist_damage,
						hate_entity->stored_hate_amount
					).c_str()
				);
			}

			entity_number++;
		}
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"{} has nothing on its hatelist.",
				c->GetTargetDescription(hate_owner)
			).c_str()
		);
	}
}

int HateList::AreaRampage(Mob *caster, Mob *target, int count, ExtraAttackOptions *opts)
{
	if (!target || !caster) {
		return 0;
	}

	// tank will be hit ONLY if they are the only target on the hate list
	// if there is anyone else on the hate list, the tank will not be hit, even if those others aren't hit either
	if (list.size() == 1) {
		caster->ProcessAttackRounds(target, opts);
		return 1;
	}

	int hit_count = 0;
	// This should prevent crashes if something dies (or mainly more than 1 thing goes away)
	// This is a temp solution until the hate lists can be rewritten to not have that issue
	std::vector<uint16> id_list;
	for (auto &h : list) {
		if (h->entity_on_hatelist && h->entity_on_hatelist != caster && h->entity_on_hatelist != target &&
			caster->CombatRange(h->entity_on_hatelist, 1.0, true, opts)) {
			id_list.push_back(h->entity_on_hatelist->GetID());
		}

		if (count != -1 && id_list.size() > count) {
			break;
		}
	}

	for (auto &id : id_list) {
		auto mob = entity_list.GetMobID(id);
		if (mob) {
			++hit_count;
			caster->ProcessAttackRounds(mob, opts);
		}
	}
	return hit_count;
}

void HateList::SpellCast(Mob *caster, uint32 spell_id, float range, Mob* ae_center)
{
	if (!caster)
		return;

	Mob* center = caster;

	if (ae_center)
		center = ae_center;

	//this is slower than just iterating through the list but avoids
	//crashes when people kick the bucket in the middle of this call
	//that invalidates our iterator but there's no way to know sadly
	//So keep a list of entity ids and look up after
	std::list<uint32> id_list;
	range = range * range;
	float min_range2 = spells[spell_id].min_range * spells[spell_id].min_range;
	float dist_targ = 0;
	auto iterator = list.begin();
	while (iterator != list.end())
	{
		struct_HateList *h = (*iterator);
		if (range > 0)
		{
			dist_targ = DistanceSquared(center->GetPosition(), h->entity_on_hatelist->GetPosition());
			if (dist_targ <= range && dist_targ >= min_range2)
			{
				id_list.push_back(h->entity_on_hatelist->GetID());
				h->entity_on_hatelist->CalcSpellPowerDistanceMod(spell_id, dist_targ);
			}
		}
		else
		{
			id_list.push_back(h->entity_on_hatelist->GetID());
			h->entity_on_hatelist->CalcSpellPowerDistanceMod(spell_id, 0, caster);
		}
		++iterator;
	}

	auto iter = id_list.begin();
	while (iter != id_list.end())
	{
		Mob *cur = entity_list.GetMobID((*iter));
		if (cur)
		{
			caster->SpellOnTarget(spell_id, cur);
		}
		iter++;
	}
}

void HateList::RemoveStaleEntries(int time_ms, float dist)
{
	auto it = list.begin();

	auto cur_time = Timer::GetCurrentTime();

	auto dist2 = dist * dist;

	while (it != list.end()) {
		auto m = (*it)->entity_on_hatelist;
		if (m) {
			bool remove = false;

			if (cur_time - (*it)->last_modified > time_ms) {
				remove = true;
			}

			if (!remove && DistanceSquaredNoZ(hate_owner->GetPosition(), m->GetPosition()) > dist2) {
				(*it)->oor_count++;
				if ((*it)->oor_count == 2) {
					remove = true;
				}
			} else if ((*it)->oor_count != 0) {
				(*it)->oor_count = 0;
			}

			if (remove) {
				if (parse->HasQuestSub(hate_owner->GetNPCTypeID(), EVENT_HATE_LIST)) {
					parse->EventNPC(EVENT_HATE_LIST, hate_owner->CastToNPC(), m, "0", 0);
				}

				if (m->IsClient()) {
					m->CastToClient()->DecrementAggroCount();
					m->CastToClient()->RemoveXTarget(hate_owner, true);
				}

				delete (*it);
				it = list.erase(it);
				continue;
			}
		}
		++it;
	}
}

void HateList::DamageHateList(int64 damage, uint32 distance, EntityFilterType filter_type, bool is_percentage)
{
	if (damage <= 0) {
		return;
	}

	const auto& l = GetFilteredHateList(filter_type, distance);
	for (const auto& h : l) {
		auto e = h->entity_on_hatelist;
		if (is_percentage) {
			const auto damage_percentage = EQ::Clamp(damage, static_cast<int64>(1), static_cast<int64>(100));
			const auto total_damage = (e->GetMaxHP() / 100) * damage_percentage;
			e->Damage(hate_owner, total_damage, SPELL_UNKNOWN, EQ::skills::SkillEagleStrike);
		} else {
			e->Damage(hate_owner, damage, SPELL_UNKNOWN, EQ::skills::SkillEagleStrike);
		}
	}
}

std::list<struct_HateList*> HateList::GetFilteredHateList(EntityFilterType filter_type, uint32 distance)
{
	std::list<struct_HateList*> l;
	const auto squared_distance = (distance * distance);
	for (auto h : list) {
		auto e = h->entity_on_hatelist;
		if (!e) {
			continue;
		}

		if (
			distance &&
			DistanceSquaredNoZ(
				hate_owner->GetPosition(),
				e->GetPosition()
			) > squared_distance
		) {
			continue;
		}

		if (
			(filter_type == EntityFilterType::Bots && !e->IsBot()) ||
			(filter_type == EntityFilterType::Clients && !e->IsClient()) ||
			(filter_type == EntityFilterType::NPCs && !e->IsNPC())
		) {
			continue;
		}

		l.push_back(h);
	}

	return l;
}
