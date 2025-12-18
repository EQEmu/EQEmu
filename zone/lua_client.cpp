#ifdef LUA_EQEMU

#include "lua_client.h"

#include "zone/client.h"
#include "zone/dialogue_window.h"
#include "zone/dynamic_zone.h"
#include "zone/expedition_request.h"
#include "zone/lua_expedition.h"
#include "zone/lua_group.h"
#include "zone/lua_inventory.h"
#include "zone/lua_item.h"
#include "zone/lua_iteminst.h"
#include "zone/lua_npc.h"
#include "zone/lua_packet.h"
#include "zone/lua_raid.h"
#include "zone/titles.h"

#include "lua.hpp"
#include "luabind/luabind.hpp"

struct InventoryWhere { };

void Lua_Client::SendSound() {
	Lua_Safe_Call_Void();
	self->SendSound();
}

void Lua_Client::Save() {
	Lua_Safe_Call_Void();
	self->Save();
}

void Lua_Client::Save(int commit_now) {
	Lua_Safe_Call_Void();
	self->Save(commit_now);
}

bool Lua_Client::Connected() {
	Lua_Safe_Call_Bool();
	return self->Connected();
}

bool Lua_Client::InZone() {
	Lua_Safe_Call_Bool();
	return self->InZone();
}

void Lua_Client::Kick() {
	Lua_Safe_Call_Void();
	self->Kick("Lua Quest");
}

void Lua_Client::Disconnect() {
	Lua_Safe_Call_Void();
	self->Disconnect();
}

bool Lua_Client::IsLD() {
	Lua_Safe_Call_Bool();
	return self->IsLD();
}

void Lua_Client::WorldKick() {
	Lua_Safe_Call_Void();
	self->WorldKick();
}

int Lua_Client::GetAFK() {
	Lua_Safe_Call_Int();
	return self->GetAFK();
}

void Lua_Client::SetAFK(uint8 afk_flag) {
	Lua_Safe_Call_Void();
	self->SetAFK(afk_flag);
}

int Lua_Client::GetAnon() {
	Lua_Safe_Call_Int();
	return self->GetAnon();
}

void Lua_Client::SetAnon(uint8 anon_flag) {
	Lua_Safe_Call_Void();
	self->SetAnon(anon_flag);
}

void Lua_Client::Duck() {
	Lua_Safe_Call_Void();
	self->Duck();
}

void Lua_Client::Sit() {
	Lua_Safe_Call_Void();
	self->Sit();
}

void Lua_Client::DyeArmorBySlot(uint8 slot, uint8 red, uint8 green, uint8 blue) {
	Lua_Safe_Call_Void();
	self->DyeArmorBySlot(slot, red, green, blue);
}

void Lua_Client::DyeArmorBySlot(uint8 slot, uint8 red, uint8 green, uint8 blue, uint8 use_tint) {
	Lua_Safe_Call_Void();
	self->DyeArmorBySlot(slot, red, green, blue, use_tint);
}

void Lua_Client::Stand() {
	Lua_Safe_Call_Void();
	self->Stand();
}

void Lua_Client::SetGM(bool v) {
	Lua_Safe_Call_Void();
	self->SetGM(v);
}

void Lua_Client::SetPVP(bool v) {
	Lua_Safe_Call_Void();
	self->SetPVP(v);
}

void Lua_Client::SendToGuildHall() {
	Lua_Safe_Call_Void();
	self->SendToGuildHall();
}

bool Lua_Client::GetPVP() {
	Lua_Safe_Call_Bool();
	return self->GetPVP();
}

bool Lua_Client::GetGM() {
	Lua_Safe_Call_Bool();
	return self->GetGM();
}

void Lua_Client::SetBaseClass(int v) {
	Lua_Safe_Call_Void();
	self->SetBaseClass(v);
}

void Lua_Client::SetBaseRace(int v) {
	Lua_Safe_Call_Void();
	self->SetBaseRace(v);
}

void Lua_Client::SetBaseGender(int v) {
	Lua_Safe_Call_Void();
	self->SetBaseGender(v);
}

uint16 Lua_Client::GetClassBitmask() {
	Lua_Safe_Call_Int();
	return GetPlayerClassBit(self->GetClass());
}

uint32 Lua_Client::GetDeityBitmask() {
	Lua_Safe_Call_Int();
	return Deity::GetBitmask(GetDeity());
}

uint16 Lua_Client::GetRaceBitmask() {
	Lua_Safe_Call_Int();
	return GetPlayerRaceBit(self->GetBaseRace());
}

int Lua_Client::GetBaseFace() {
	Lua_Safe_Call_Int();
	return self->GetBaseFace();
}

int Lua_Client::GetLanguageSkill(int skill_id) {
	Lua_Safe_Call_Int();
	return self->GetLanguageSkill(skill_id);
}

int Lua_Client::GetLDoNPointsTheme(int theme) {
	Lua_Safe_Call_Int();
	return self->GetLDoNPointsTheme(theme);
}

int Lua_Client::GetBaseSTR() {
	Lua_Safe_Call_Int();
	return self->GetBaseSTR();
}

int Lua_Client::GetBaseSTA() {
	Lua_Safe_Call_Int();
	return self->GetBaseSTA();
}

int Lua_Client::GetBaseCHA() {
	Lua_Safe_Call_Int();
	return self->GetBaseCHA();
}

int Lua_Client::GetBaseDEX() {
	Lua_Safe_Call_Int();
	return self->GetBaseDEX();
}

int Lua_Client::GetBaseINT() {
	Lua_Safe_Call_Int();
	return self->GetBaseINT();
}

int Lua_Client::GetBaseAGI() {
	Lua_Safe_Call_Int();
	return self->GetBaseAGI();
}

int Lua_Client::GetBaseWIS() {
	Lua_Safe_Call_Int();
	return self->GetBaseWIS();
}

int Lua_Client::GetWeight() {
	Lua_Safe_Call_Int();
	return self->GetWeight();
}

uint32 Lua_Client::GetEXP() {
	Lua_Safe_Call_Int();
	return self->GetEXP();
}

uint32 Lua_Client::GetAAExp() {
	Lua_Safe_Call_Int();
	return self->GetAAXP();
}

uint32 Lua_Client::GetAAPercent() {
	Lua_Safe_Call_Int();
	return self->GetAAPercent();
}

uint32 Lua_Client::GetTotalSecondsPlayed() {
	Lua_Safe_Call_Int();
	return self->GetTotalSecondsPlayed();
}

void Lua_Client::UpdateLDoNPoints(uint32 theme_id, int points) {
	Lua_Safe_Call_Void();
	self->UpdateLDoNPoints(theme_id, points);
}

void Lua_Client::SetDeity(int v) {
	Lua_Safe_Call_Void();
	self->SetDeity(v);
}

void Lua_Client::AddEXP(uint32 add_exp) {
	Lua_Safe_Call_Void();
	self->AddEXP(ExpSource::Quest, add_exp);
}

void Lua_Client::AddEXP(uint32 add_exp, int conlevel) {
	Lua_Safe_Call_Void();
	self->AddEXP(ExpSource::Quest, add_exp, conlevel);
}

void Lua_Client::AddEXP(uint32 add_exp, int conlevel, bool resexp) {
	Lua_Safe_Call_Void();
	self->AddEXP(ExpSource::Quest, add_exp, conlevel, resexp);
}

void Lua_Client::SetEXP(uint64 set_exp, uint64 set_aaxp) {
	Lua_Safe_Call_Void();
	self->SetEXP(ExpSource::Quest, set_exp, set_aaxp);
}

void Lua_Client::SetEXP(uint64 set_exp, uint64 set_aaxp, bool resexp) {
	Lua_Safe_Call_Void();
	self->SetEXP(ExpSource::Quest, set_exp, set_aaxp, resexp);
}

void Lua_Client::SetBindPoint() {
	Lua_Safe_Call_Void();
	self->SetBindPoint();
}

void Lua_Client::SetBindPoint(int to_zone) {
	Lua_Safe_Call_Void();
	self->SetBindPoint(0, to_zone);
}

void Lua_Client::SetBindPoint(int to_zone, int to_instance) {
	Lua_Safe_Call_Void();
	self->SetBindPoint(0, to_zone, to_instance);
}

void Lua_Client::SetBindPoint(int to_zone, int to_instance, float new_x) {
	Lua_Safe_Call_Void();
	self->SetBindPoint(0, to_zone, to_instance, glm::vec3(new_x,0.0f,0.0f));
}

void Lua_Client::SetBindPoint(int to_zone, int to_instance, float new_x, float new_y) {
	Lua_Safe_Call_Void();
	self->SetBindPoint(0, to_zone, to_instance, glm::vec3(new_x, new_y, 0.0f));
}

void Lua_Client::SetBindPoint(int to_zone, int to_instance, float new_x, float new_y, float new_z) {
	Lua_Safe_Call_Void();
	self->SetBindPoint(0, to_zone, to_instance, glm::vec3(new_x, new_y, new_z));
}

void Lua_Client::SetBindPoint(int to_zone, int to_instance, float new_x, float new_y, float new_z, float new_heading) {
	Lua_Safe_Call_Void();
	self->SetBindPoint2(0, to_zone, to_instance, glm::vec4(new_x, new_y, new_z, new_heading));
}

float Lua_Client::GetBindX() {
	Lua_Safe_Call_Real();
	return self->GetBindX();
}

float Lua_Client::GetBindX(int index) {
	Lua_Safe_Call_Real();
	return self->GetBindX(index);
}

float Lua_Client::GetBindY() {
	Lua_Safe_Call_Real();
	return self->GetBindY();
}

float Lua_Client::GetBindY(int index) {
	Lua_Safe_Call_Real();
	return self->GetBindY(index);
}

float Lua_Client::GetBindZ() {
	Lua_Safe_Call_Real();
	return self->GetBindZ();
}

float Lua_Client::GetBindZ(int index) {
	Lua_Safe_Call_Real();
	return self->GetBindZ(index);
}

float Lua_Client::GetBindHeading() {
	Lua_Safe_Call_Real();
	return self->GetBindHeading();
}

float Lua_Client::GetBindHeading(int index) {
	Lua_Safe_Call_Real();
	return self->GetBindHeading(index);
}

uint32 Lua_Client::GetBindZoneID() {
	Lua_Safe_Call_Int();
	return self->GetBindZoneID();
}

uint32 Lua_Client::GetBindZoneID(int index) {
	Lua_Safe_Call_Int();
	return self->GetBindZoneID(index);
}

float Lua_Client::GetTargetRingX() {
	Lua_Safe_Call_Real();
	return self->GetTargetRingX();
}

float Lua_Client::GetTargetRingY() {
	Lua_Safe_Call_Real();
	return self->GetTargetRingY();
}

float Lua_Client::GetTargetRingZ() {
	Lua_Safe_Call_Real();
	return self->GetTargetRingZ();
}

void Lua_Client::MovePC(int zone, float x, float y, float z, float heading) {
	Lua_Safe_Call_Void();
	self->MovePC(zone, x, y, z, heading);
}

void Lua_Client::MovePCInstance(int zone, int instance, float x, float y, float z, float heading) {
	Lua_Safe_Call_Void();
	self->MovePC(zone, instance, x, y, z, heading);
}

void Lua_Client::ChangeLastName(std::string last_name) {
	Lua_Safe_Call_Void();
	self->ChangeLastName(last_name);
}

int Lua_Client::GetFactionLevel(uint32 char_id, uint32 npc_id, uint32 race, uint32 class_, uint32 deity, uint32 faction, Lua_NPC npc) {
	Lua_Safe_Call_Int();
	return static_cast<int>(self->GetFactionLevel(char_id, npc_id, race, class_, deity, faction, npc));
}

void Lua_Client::SetFactionLevel(uint32 char_id, uint32 npc_faction_id, int char_class, int char_race, int char_deity) {
	Lua_Safe_Call_Void();
	self->SetFactionLevel(char_id, npc_faction_id, char_class, char_race, char_deity);
}

void Lua_Client::SetFactionLevel2(uint32 char_id, int faction_id, int char_class, int char_race, int char_deity, int value, int temp) {
	Lua_Safe_Call_Void();
	self->SetFactionLevel2(char_id, faction_id, char_class, char_race, char_deity, value, temp);
}

void Lua_Client::RewardFaction(int id, int amount) {
	Lua_Safe_Call_Void();
	self->RewardFaction(id, amount);
}

int Lua_Client::GetRawItemAC() {
	Lua_Safe_Call_Int();
	return self->GetRawItemAC();
}

uint32 Lua_Client::AccountID() {
	Lua_Safe_Call_Int();
	return self->AccountID();
}

const char *Lua_Client::AccountName() {
	Lua_Safe_Call_String();
	return self->AccountName();
}

int Lua_Client::GetAccountAge() {
	Lua_Safe_Call_Int();
	return time(nullptr) - self->GetAccountCreation();
}

int16 Lua_Client::Admin() {
	Lua_Safe_Call_Int();
	return self->Admin();
}

uint32 Lua_Client::CharacterID() {
	Lua_Safe_Call_Int();
	return self->CharacterID();
}

int Lua_Client::GuildRank() {
	Lua_Safe_Call_Int();
	return self->GuildRank();
}

uint32 Lua_Client::GuildID() {
	Lua_Safe_Call_Int();
	return self->GuildID();
}

int Lua_Client::GetFace() {
	Lua_Safe_Call_Int();
	return self->GetFace();
}

bool Lua_Client::TakeMoneyFromPP(uint64 copper) {
	Lua_Safe_Call_Bool();
	return self->TakeMoneyFromPP(copper);
}

bool Lua_Client::TakeMoneyFromPP(uint64 copper, bool update_client) {
	Lua_Safe_Call_Bool();
	return self->TakeMoneyFromPP(copper, update_client);
}

void Lua_Client::AddMoneyToPP(uint32 copper, uint32 silver, uint32 gold, uint32 platinum) {
	Lua_Safe_Call_Void();
	self->AddMoneyToPP(copper, silver, gold, platinum);
}
void Lua_Client::AddMoneyToPP(uint32 copper, uint32 silver, uint32 gold, uint32 platinum, bool update_client) {
	Lua_Safe_Call_Void();
	self->AddMoneyToPP(copper, silver, gold, platinum, update_client);
}

bool Lua_Client::TGB() {
	Lua_Safe_Call_Bool();
	return self->TGB();
}

int Lua_Client::GetSkillPoints() {
	Lua_Safe_Call_Int();
	return self->GetSkillPoints();
}

void Lua_Client::SetSkillPoints(int skill) {
	Lua_Safe_Call_Void();
	self->SetSkillPoints(skill);
}

void Lua_Client::IncreaseSkill(int skill_id) {
	Lua_Safe_Call_Void();
	self->IncreaseSkill(skill_id);
}

void Lua_Client::IncreaseSkill(int skill_id, int value) {
	Lua_Safe_Call_Void();
	self->IncreaseSkill(skill_id, value);
}

void Lua_Client::IncreaseLanguageSkill(uint8 language_id) {
	Lua_Safe_Call_Void();
	self->IncreaseLanguageSkill(language_id);
}

void Lua_Client::IncreaseLanguageSkill(uint8 language_id, uint8 increase) {
	Lua_Safe_Call_Void();
	self->IncreaseLanguageSkill(language_id, increase);
}

int Lua_Client::GetRawSkill(int skill_id) {
	Lua_Safe_Call_Int();
	return self->GetRawSkill(static_cast<EQ::skills::SkillType>(skill_id));
}

bool Lua_Client::HasSkill(int skill_id) {
	Lua_Safe_Call_Bool();
	return self->HasSkill(static_cast<EQ::skills::SkillType>(skill_id));
}

bool Lua_Client::CanHaveSkill(int skill_id) {
	Lua_Safe_Call_Bool();
	return self->CanHaveSkill(static_cast<EQ::skills::SkillType>(skill_id));
}

void Lua_Client::SetSkill(int skill_id, int value) {
	Lua_Safe_Call_Void();
	self->SetSkill(static_cast<EQ::skills::SkillType>(skill_id), value);
}

void Lua_Client::AddSkill(int skill_id, int value) {
	Lua_Safe_Call_Void();
	self->AddSkill(static_cast<EQ::skills::SkillType>(skill_id), value);
}

void Lua_Client::CheckSpecializeIncrease(int spell_id) {
	Lua_Safe_Call_Void();
	self->CheckSpecializeIncrease(spell_id);
}

void Lua_Client::CheckIncreaseSkill(int skill_id, Lua_Mob target) {
	Lua_Safe_Call_Void();
	self->CheckIncreaseSkill(static_cast<EQ::skills::SkillType>(skill_id), target);
}

void Lua_Client::CheckIncreaseSkill(int skill_id, Lua_Mob target, int chance_mod) {
	Lua_Safe_Call_Void();
	self->CheckIncreaseSkill(static_cast<EQ::skills::SkillType>(skill_id), target, chance_mod);
}

void Lua_Client::SetLanguageSkill(uint8 language_id, uint8 language_skill) {
	Lua_Safe_Call_Void();
	self->SetLanguageSkill(language_id, language_skill);
}

int Lua_Client::MaxSkill(int skill_id) {
	Lua_Safe_Call_Int();
	return self->MaxSkill(static_cast<EQ::skills::SkillType>(skill_id));
}

bool Lua_Client::IsMedding() {
	Lua_Safe_Call_Bool();
	return self->IsMedding();
}

int Lua_Client::GetDuelTarget() {
	Lua_Safe_Call_Int();
	return self->GetDuelTarget();
}

bool Lua_Client::IsDueling() {
	Lua_Safe_Call_Bool();
	return self->IsDueling();
}

void Lua_Client::SetDuelTarget(int c) {
	Lua_Safe_Call_Void();
	self->SetDuelTarget(c);
}

void Lua_Client::SetDueling(bool v) {
	Lua_Safe_Call_Void();
	self->SetDueling(v);
}

void Lua_Client::ResetAA() {
	Lua_Safe_Call_Void();
	self->ResetAA();
}

void Lua_Client::MemSpell(int spell_id, int slot) {
	Lua_Safe_Call_Void();
	self->MemSpell(spell_id, slot);
}

void Lua_Client::MemSpell(int spell_id, int slot, bool update_client) {
	Lua_Safe_Call_Void();
	self->MemSpell(spell_id, slot, update_client);
}

void Lua_Client::UnmemSpell(int slot) {
	Lua_Safe_Call_Void();
	self->UnmemSpell(slot);
}

void Lua_Client::UnmemSpell(int slot, bool update_client) {
	Lua_Safe_Call_Void();
	self->UnmemSpell(slot, update_client);
}

void Lua_Client::UnmemSpellBySpellID(int32 spell_id) {
	Lua_Safe_Call_Void();
	self->UnmemSpellBySpellID(spell_id);
}

void Lua_Client::UnmemSpellAll() {
	Lua_Safe_Call_Void();
	self->UnmemSpellAll();
}

void Lua_Client::UnmemSpellAll(bool update_client) {
	Lua_Safe_Call_Void();
	self->UnmemSpellAll(update_client);
}

uint16 Lua_Client::FindMemmedSpellBySlot(int slot) {
	Lua_Safe_Call_Int();
	return self->FindMemmedSpellBySlot(slot);
}

int Lua_Client::FindMemmedSpellBySpellID(uint16 spell_id) {
	Lua_Safe_Call_Int();
	return self->FindMemmedSpellBySpellID(spell_id);
}

int Lua_Client::FindEmptyMemSlot() {
	Lua_Safe_Call_Int();
	return self->FindEmptyMemSlot();
}

int Lua_Client::MemmedCount() {
	Lua_Safe_Call_Int();
	return self->MemmedCount();
}

luabind::object Lua_Client::GetLearnableDisciplines(lua_State* L) {
	auto lua_table = luabind::newtable(L);
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto learnable_disciplines = self->GetLearnableDisciplines();
		int index = 1;
		for (auto spell_id : learnable_disciplines) {
			lua_table[index] = spell_id;
			index++;
		}
	}
	return lua_table;
}

luabind::object Lua_Client::GetLearnableDisciplines(lua_State* L, uint8 min_level) {
	auto lua_table = luabind::newtable(L);
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto learnable_disciplines = self->GetLearnableDisciplines(min_level);
		int index = 1;
		for (auto spell_id : learnable_disciplines) {
			lua_table[index] = spell_id;
			index++;
		}
	}
	return lua_table;
}

luabind::object Lua_Client::GetLearnableDisciplines(lua_State* L, uint8 min_level, uint8 max_level) {
	auto lua_table = luabind::newtable(L);
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto learnable_disciplines = self->GetLearnableDisciplines(min_level, max_level);
		int index = 1;
		for (auto spell_id : learnable_disciplines) {
			lua_table[index] = spell_id;
			index++;
		}
	}
	return lua_table;
}

luabind::object Lua_Client::GetLearnedDisciplines(lua_State* L) {
	auto lua_table = luabind::newtable(L);
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto learned_disciplines = self->GetLearnedDisciplines();
		int index = 1;
		for (auto spell_id : learned_disciplines) {
			lua_table[index] = spell_id;
			index++;
		}
	}
	return lua_table;
}

luabind::object Lua_Client::GetMemmedSpells(lua_State* L) {
	auto lua_table = luabind::newtable(L);
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto memmed_spells = self->GetMemmedSpells();
		int index = 1;
		for (auto spell_id : memmed_spells) {
			lua_table[index] = spell_id;
			index++;
		}
	}
	return lua_table;
}

luabind::object Lua_Client::GetScribeableSpells(lua_State* L) {
	auto lua_table = luabind::newtable(L);
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto scribeable_spells = self->GetScribeableSpells();
		int index = 1;
		for (auto spell_id : scribeable_spells) {
			lua_table[index] = spell_id;
			index++;
		}
	}
	return lua_table;
}

luabind::object Lua_Client::GetScribeableSpells(lua_State* L, uint8 min_level) {
	auto lua_table = luabind::newtable(L);
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto scribeable_spells = self->GetScribeableSpells(min_level);
		int index = 1;
		for (auto spell_id : scribeable_spells) {
			lua_table[index] = spell_id;
			index++;
		}
	}
	return lua_table;
}

luabind::object Lua_Client::GetScribeableSpells(lua_State* L, uint8 min_level, uint8 max_level) {
	auto lua_table = luabind::newtable(L);
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto scribeable_spells = self->GetScribeableSpells(min_level, max_level);
		int index = 1;
		for (auto spell_id : scribeable_spells) {
			lua_table[index] = spell_id;
			index++;
		}
	}
	return lua_table;
}

luabind::object Lua_Client::GetScribedSpells(lua_State* L) {
	auto lua_table = luabind::newtable(L);
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto scribed_spells = self->GetScribedSpells();
		int index = 1;
		for (auto spell_id : scribed_spells) {
			lua_table[index] = spell_id;
			index++;
		}
	}
	return lua_table;
}

void Lua_Client::ScribeSpell(int spell_id, int slot) {
	Lua_Safe_Call_Void();
	self->ScribeSpell(spell_id, slot);
}

void Lua_Client::ScribeSpell(int spell_id, int slot, bool update_client) {
	Lua_Safe_Call_Void();
	self->ScribeSpell(spell_id, slot, update_client);
}

void Lua_Client::UnscribeSpell(int slot) {
	Lua_Safe_Call_Void();
	self->UnscribeSpell(slot);
}

void Lua_Client::UnscribeSpell(int slot, bool update_client) {
	Lua_Safe_Call_Void();
	self->UnscribeSpell(slot, update_client);
}

void Lua_Client::UnscribeSpellAll() {
	Lua_Safe_Call_Void();
	self->UnscribeSpellAll();
}

void Lua_Client::UnscribeSpellAll(bool update_client) {
	Lua_Safe_Call_Void();
	self->UnscribeSpellAll(update_client);
}

void Lua_Client::TrainDisc(int itemid) {
	Lua_Safe_Call_Void();
	self->TrainDiscipline(itemid);
}

void Lua_Client::TrainDiscBySpellID(int32 spell_id) {
	Lua_Safe_Call_Void();
	self->TrainDiscBySpellID(spell_id);
}

int Lua_Client::GetDiscSlotBySpellID(int32 spell_id) {
	Lua_Safe_Call_Int();
	return self->GetDiscSlotBySpellID(spell_id);
}

void Lua_Client::UntrainDisc(int slot) {
	Lua_Safe_Call_Void();
	self->UntrainDisc(slot);
}

void Lua_Client::UntrainDisc(int slot, bool update_client) {
	Lua_Safe_Call_Void();
	self->UntrainDisc(slot, update_client);
}

void Lua_Client::UntrainDiscAll() {
	Lua_Safe_Call_Void();
	self->UntrainDiscAll();
}

void Lua_Client::UntrainDiscAll(bool update_client) {
	Lua_Safe_Call_Void();
	self->UntrainDiscAll(update_client);
}

bool Lua_Client::IsStanding() {
	Lua_Safe_Call_Bool();
	return self->IsStanding();
}

bool Lua_Client::IsSitting() {
	Lua_Safe_Call_Bool();
	return self->IsSitting();
}

bool Lua_Client::IsCrouching() {
	Lua_Safe_Call_Bool();
	return self->IsCrouching();
}

void Lua_Client::SetFeigned(bool v) {
	Lua_Safe_Call_Void();
	self->SetFeigned(v);
}

bool Lua_Client::GetFeigned() {
	Lua_Safe_Call_Bool();
	return self->GetFeigned();
}

bool Lua_Client::AutoSplitEnabled() {
	Lua_Safe_Call_Bool();
	return self->AutoSplitEnabled();
}

void Lua_Client::SetHorseId(int id) {
	Lua_Safe_Call_Void();
	self->SetHorseId(id);
}

int Lua_Client::GetHorseId() {
	Lua_Safe_Call_Int();
	return self->GetHorseId();
}

void Lua_Client::NukeItem(uint32 item_num) {
	Lua_Safe_Call_Void();
	self->NukeItem(item_num, 0xFF);
}

void Lua_Client::NukeItem(uint32 item_num, int where_to_check) {
	Lua_Safe_Call_Void();
	self->NukeItem(item_num, where_to_check);
}

void Lua_Client::SetTint(int slot_id, uint32 color) {
	Lua_Safe_Call_Void();
	self->SetTint(slot_id, color);
}

void Lua_Client::SetMaterial(int slot_id, uint32 item_id) {
	Lua_Safe_Call_Void();
	self->SetMaterial(slot_id, item_id);
}

void Lua_Client::Undye() {
	Lua_Safe_Call_Void();
	self->Undye();
}

int Lua_Client::GetItemIDAt(int slot_id) {
	Lua_Safe_Call_Int();
	return self->GetItemIDAt(slot_id);
}

int Lua_Client::GetAugmentIDAt(int slot_id, int aug_slot) {
	Lua_Safe_Call_Int();
	return self->GetAugmentIDAt(slot_id, aug_slot);
}

void Lua_Client::DeleteItemInInventory(int slot_id, int quantity) {
	Lua_Safe_Call_Void();
	self->DeleteItemInInventory(slot_id, quantity);
}

void Lua_Client::DeleteItemInInventory(int slot_id, int quantity, bool update_client) {
	Lua_Safe_Call_Void();
	self->DeleteItemInInventory(slot_id, quantity, update_client);
}

void Lua_Client::SummonItem(uint32 item_id) {
	Lua_Safe_Call_Void();
	self->SummonItem(item_id);
}

void Lua_Client::SummonItem(uint32 item_id, int charges) {
	Lua_Safe_Call_Void();
	self->SummonItem(item_id, charges);
}

void Lua_Client::SummonItem(uint32 item_id, int charges, uint32 aug1) {
	Lua_Safe_Call_Void();
	self->SummonItem(item_id, charges, aug1);
}

void Lua_Client::SummonItem(uint32 item_id, int charges, uint32 aug1, uint32 aug2) {
	Lua_Safe_Call_Void();
	self->SummonItem(item_id, charges, aug1, aug2);
}

void Lua_Client::SummonItem(uint32 item_id, int charges, uint32 aug1, uint32 aug2, uint32 aug3) {
	Lua_Safe_Call_Void();
	self->SummonItem(item_id, charges, aug1, aug2, aug3);
}

void Lua_Client::SummonItem(uint32 item_id, int charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4) {
	Lua_Safe_Call_Void();
	self->SummonItem(item_id, charges, aug1, aug2, aug3, aug4);
}

void Lua_Client::SummonItem(uint32 item_id, int charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5) {
	Lua_Safe_Call_Void();
	self->SummonItem(item_id, charges, aug1, aug2, aug3, aug4, aug5);
}

void Lua_Client::SummonItem(uint32 item_id, int charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5, bool attuned) {
	Lua_Safe_Call_Void();
	self->SummonItem(item_id, charges, aug1, aug2, aug3, aug4, aug5, 0, attuned);
}

void Lua_Client::SummonItem(uint32 item_id, int charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5, bool attuned, int to_slot) {
	Lua_Safe_Call_Void();
	self->SummonItem(item_id, charges, aug1, aug2, aug3, aug4, aug5, 0, attuned, to_slot);
}

void Lua_Client::SetStats(int type, int value) {
	Lua_Safe_Call_Void();
	self->SetStats(type, value);
}

void Lua_Client::IncStats(int type, int value) {
	Lua_Safe_Call_Void();
	self->IncStats(type, value);
}

void Lua_Client::DropItem(int slot_id) {
	Lua_Safe_Call_Void();
	self->DropItem(slot_id);
}

void Lua_Client::BreakInvis() {
	Lua_Safe_Call_Void();
	self->BreakInvis();
}

void Lua_Client::LeaveGroup() {
	Lua_Safe_Call_Void();
	self->LeaveGroup();
}

bool Lua_Client::IsGrouped() {
	Lua_Safe_Call_Bool();
	return self->IsGrouped();
}

bool Lua_Client::IsRaidGrouped() {
	Lua_Safe_Call_Bool();
	return self->IsRaidGrouped();
}

bool Lua_Client::Hungry() {
	Lua_Safe_Call_Bool();
	return self->Hungry();
}

bool Lua_Client::Thirsty() {
	Lua_Safe_Call_Bool();
	return self->Thirsty();
}

int Lua_Client::GetInstrumentMod(int spell_id) {
	Lua_Safe_Call_Int();
	return self->GetInstrumentMod(spell_id);
}

bool Lua_Client::DecreaseByID(uint32 type, int amt) {
	Lua_Safe_Call_Bool();
	return self->DecreaseByID(type, amt);
}

void Lua_Client::Escape() {
	Lua_Safe_Call_Void();
	self->Escape();
}

void Lua_Client::GoFish() {
	Lua_Safe_Call_Void();
	self->GoFish();
}

void Lua_Client::ForageItem() {
	Lua_Safe_Call_Void();
	self->ForageItem();
}

void Lua_Client::ForageItem(bool guarantee) {
	Lua_Safe_Call_Void();
	self->ForageItem(guarantee);
}

float Lua_Client::CalcPriceMod(Lua_Mob other, bool reverse) {
	Lua_Safe_Call_Real();
	return self->CalcPriceMod(other, reverse);
}

void Lua_Client::ResetTrade() {
	Lua_Safe_Call_Void();
	self->ResetTrade();
}

uint32 Lua_Client::GetDisciplineTimer(uint32 timer_id) {
	Lua_Safe_Call_Int();
	return self->GetDisciplineTimer(timer_id);
}

void Lua_Client::ResetDisciplineTimer(uint32 timer_id) {
	Lua_Safe_Call_Void();
	self->ResetDisciplineTimer(timer_id);
}

bool Lua_Client::UseDiscipline(int spell_id, int target_id) {
	Lua_Safe_Call_Bool();
	return self->UseDiscipline(spell_id, target_id);
}

bool Lua_Client::HasDisciplineLearned(uint16 spell_id) {
	Lua_Safe_Call_Bool();
	return self->HasDisciplineLearned(spell_id);
}

int Lua_Client::GetCharacterFactionLevel(int faction_id) {
	Lua_Safe_Call_Int();
	return self->GetCharacterFactionLevel(faction_id);
}

void Lua_Client::SetZoneFlag(uint32 zone_id) {
	Lua_Safe_Call_Void();
	self->SetZoneFlag(zone_id);
}

void Lua_Client::ClearZoneFlag(uint32 zone_id) {
	Lua_Safe_Call_Void();
	self->ClearZoneFlag(zone_id);
}

bool Lua_Client::HasZoneFlag(uint32 zone_id) {
	Lua_Safe_Call_Bool();
	return self->HasZoneFlag(zone_id);
}

void Lua_Client::SendZoneFlagInfo(Lua_Client to) {
	Lua_Safe_Call_Void();
	self->SendZoneFlagInfo(to);
}

void Lua_Client::SetAATitle(std::string title) {
	Lua_Safe_Call_Void();
	self->SetAATitle(title);
}

void Lua_Client::SetAATitle(std::string title, bool save_to_database) {
	Lua_Safe_Call_Void();
	if (!save_to_database) {
		self->SetAATitle(title);
	} else {
		title_manager.CreateNewPlayerTitle(self, title);
	}
}

int Lua_Client::GetClientVersion() {
	Lua_Safe_Call_Int();
	return static_cast<unsigned int>(self->ClientVersion());
}

uint32 Lua_Client::GetClientVersionBit() {
	Lua_Safe_Call_Int();
	return self->ClientVersionBit();
}

void Lua_Client::SetTitleSuffix(const char *text) {
	Lua_Safe_Call_Void();
	self->SetTitleSuffix(text);
}

void Lua_Client::SetAAPoints(int points) {
	Lua_Safe_Call_Void();
	self->SetAAPoints(points);
}

int Lua_Client::GetAAPoints() {
	Lua_Safe_Call_Int();
	return self->GetAAPoints();
}

int Lua_Client::GetSpentAA() {
	Lua_Safe_Call_Int();
	return self->GetSpentAA();
}

void Lua_Client::AddAAPoints(int points) {
	Lua_Safe_Call_Void();
	self->AddAAPoints(points);
}

void Lua_Client::RefundAA() {
	Lua_Safe_Call_Void();
	self->RefundAA();
}

int Lua_Client::GetModCharacterFactionLevel(int faction) {
	Lua_Safe_Call_Int();
	return self->GetModCharacterFactionLevel(faction);
}

int Lua_Client::GetLDoNWins() {
	Lua_Safe_Call_Int();
	return self->GetLDoNWins();
}

int Lua_Client::GetLDoNLosses() {
	Lua_Safe_Call_Int();
	return self->GetLDoNLosses();
}

int Lua_Client::GetLDoNWinsTheme(int theme) {
	Lua_Safe_Call_Int();
	return self->GetLDoNWinsTheme(theme);
}

int Lua_Client::GetLDoNLossesTheme(int theme) {
	Lua_Safe_Call_Int();
	return self->GetLDoNLossesTheme(theme);
}

int Lua_Client::GetStartZone() {
	Lua_Safe_Call_Int();
	return self->GetStartZone();
}

void Lua_Client::SetStartZone(int zone_id) {
	Lua_Safe_Call_Void();
	self->SetStartZone(zone_id);
}

void Lua_Client::SetStartZone(int zone_id, float x) {
	Lua_Safe_Call_Void();
	self->SetStartZone(zone_id, x);
}

void Lua_Client::SetStartZone(int zone_id, float x, float y) {
	Lua_Safe_Call_Void();
	self->SetStartZone(zone_id, x, y);
}

void Lua_Client::SetStartZone(int zone_id, float x, float y, float z) {
	Lua_Safe_Call_Void();
	self->SetStartZone(zone_id, x, y, z);
}

void Lua_Client::AddPVPPoints(uint32 points) {
	Lua_Safe_Call_Void();
	self->AddPVPPoints(points);
}

void Lua_Client::AddCrystals(uint32 radiant_count, uint32 ebon_count) {
	Lua_Safe_Call_Void();

	if (ebon_count != 0) {
		if (ebon_count > 0) {
			self->AddEbonCrystals(ebon_count);
		} else {
			self->RemoveEbonCrystals(ebon_count);
		}
	}

	if (radiant_count != 0) {
		if (radiant_count > 0) {
			self->AddRadiantCrystals(radiant_count);
		} else {
			self->RemoveRadiantCrystals(radiant_count);
		}
	}
}

void Lua_Client::SetEbonCrystals(uint32 value) {
	Lua_Safe_Call_Void();
	self->SetEbonCrystals(value);
}

void Lua_Client::SetRadiantCrystals(uint32 value) {
	Lua_Safe_Call_Void();
	self->SetRadiantCrystals(value);
}

uint32 Lua_Client::GetPVPPoints() {
	Lua_Safe_Call_Int();
	return self->GetPVPPoints();
}

uint32 Lua_Client::GetRadiantCrystals() {
	Lua_Safe_Call_Int();
	return self->GetRadiantCrystals();
}

uint32 Lua_Client::GetEbonCrystals() {
	Lua_Safe_Call_Int();
	return self->GetEbonCrystals();
}

void Lua_Client::QuestReadBook(const char *text, int type) {
	Lua_Safe_Call_Void();
	self->QuestReadBook(text, type);
}

void Lua_Client::UpdateGroupAAs(int points, uint32 type) {
	Lua_Safe_Call_Void();
	self->UpdateGroupAAs(points, type);
}

uint32 Lua_Client::GetGroupPoints() {
	Lua_Safe_Call_Int();
	return self->GetGroupPoints();
}

uint32 Lua_Client::GetRaidPoints() {
	Lua_Safe_Call_Int();
	return self->GetRaidPoints();
}

void Lua_Client::LearnRecipe(uint32 recipe) {
	Lua_Safe_Call_Void();
	self->LearnRecipe(recipe);
}

int Lua_Client::GetEndurance() {
	Lua_Safe_Call_Int();
	return self->GetEndurance();
}

int Lua_Client::GetMaxEndurance() {
	Lua_Safe_Call_Int();
	return self->GetMaxEndurance();
}

int Lua_Client::GetEndurancePercent() {
	Lua_Safe_Call_Int();
	return self->GetEndurancePercent();
}

void Lua_Client::SetEndurance(int endur) {
	Lua_Safe_Call_Void();
	self->SetEndurance(endur);
}

void Lua_Client::SendOPTranslocateConfirm(Lua_Mob caster, int spell_id) {
	Lua_Safe_Call_Void();
	self->SendOPTranslocateConfirm(caster, spell_id);
}

uint32 Lua_Client::GetIP() {
	Lua_Safe_Call_Int();
	return self->GetIP();
}

void Lua_Client::AddLevelBasedExp(int exp_pct) {
	Lua_Safe_Call_Void();
	self->AddLevelBasedExp(ExpSource::Quest, exp_pct);
}

void Lua_Client::AddLevelBasedExp(int exp_pct, int max_level) {
	Lua_Safe_Call_Void();
	self->AddLevelBasedExp(ExpSource::Quest, exp_pct, max_level);
}

void Lua_Client::AddLevelBasedExp(int exp_pct, int max_level, bool ignore_mods) {
	Lua_Safe_Call_Void();
	self->AddLevelBasedExp(ExpSource::Quest, exp_pct, max_level, ignore_mods);
}

void Lua_Client::IncrementAA(int aa) {
	Lua_Safe_Call_Void();
	self->IncrementAlternateAdvancementRank(aa);
}

bool Lua_Client::GrantAlternateAdvancementAbility(int aa_id, int points) {
	Lua_Safe_Call_Bool();
	return self->GrantAlternateAdvancementAbility(aa_id, points);
}

bool Lua_Client::GrantAlternateAdvancementAbility(int aa_id, int points, bool ignore_cost) {
	Lua_Safe_Call_Bool();
	return self->GrantAlternateAdvancementAbility(aa_id, points, ignore_cost);
}

void Lua_Client::ResetAlternateAdvancementRank(int aa_id) {
	Lua_Safe_Call_Void();
	self->ResetAlternateAdvancementRank(aa_id);
}

void Lua_Client::MarkSingleCompassLoc(float in_x, float in_y, float in_z) {
	Lua_Safe_Call_Void();
	self->MarkSingleCompassLoc(in_x, in_y, in_z);
}

void Lua_Client::MarkSingleCompassLoc(float in_x, float in_y, float in_z, int count) {
	Lua_Safe_Call_Void();
	self->MarkSingleCompassLoc(in_x, in_y, in_z, count);
}

void Lua_Client::ClearCompassMark() {
	Lua_Safe_Call_Void();
	self->MarkSingleCompassLoc(0,0,0,0);
}

int Lua_Client::GetNextAvailableSpellBookSlot() {
	Lua_Safe_Call_Int();
	return self->GetNextAvailableSpellBookSlot();
}

uint32 Lua_Client::GetSpellIDByBookSlot(int slot_id) {
	Lua_Safe_Call_Int();
	return self->GetSpellIDByBookSlot(slot_id);
}

int Lua_Client::GetNextAvailableSpellBookSlot(int start) {
	Lua_Safe_Call_Int();
	return self->GetNextAvailableSpellBookSlot(start);
}

int Lua_Client::FindSpellBookSlotBySpellID(int spell_id) {
	Lua_Safe_Call_Int();
	return self->FindSpellBookSlotBySpellID(spell_id);
}

void Lua_Client::UpdateTaskActivity(int task, int activity, int count) {
	Lua_Safe_Call_Void();
	self->UpdateTaskActivity(task, activity, count);
}

void Lua_Client::AssignTask(int task_id) {
	Lua_Safe_Call_Void();
	self->AssignTask(task_id);
}

void Lua_Client::AssignTask(int task_id, int npc_id) {
	Lua_Safe_Call_Void();
	self->AssignTask(task_id, npc_id);
}

void Lua_Client::AssignTask(int task_id, int npc_id, bool enforce_level_requirement) {
	Lua_Safe_Call_Void();
	self->AssignTask(task_id, npc_id, enforce_level_requirement);
}

void Lua_Client::FailTask(int task) {
	Lua_Safe_Call_Void();
	self->FailTask(task);
}

bool Lua_Client::IsTaskCompleted(int task_id) {
	Lua_Safe_Call_Bool();
	return self->IsTaskCompleted(task_id);
}

bool Lua_Client::IsTaskActive(int task) {
	Lua_Safe_Call_Bool();
	return self->IsTaskActive(task);
}

bool Lua_Client::IsTaskActivityActive(int task, int activity) {
	Lua_Safe_Call_Bool();
	return self->IsTaskActivityActive(task, activity);
}

void Lua_Client::LockSharedTask(bool lock) {
	Lua_Safe_Call_Void();
	return self->LockSharedTask(lock);
}

void Lua_Client::EndSharedTask() {
	Lua_Safe_Call_Void();
	self->EndSharedTask();
}

void Lua_Client::EndSharedTask(bool send_fail) {
	Lua_Safe_Call_Void();
	self->EndSharedTask(send_fail);
}

int64 Lua_Client::GetCorpseCount() {
	Lua_Safe_Call_Int();
	return self->GetCorpseCount();
}

int Lua_Client::GetCorpseID(int corpse) {
	Lua_Safe_Call_Int();
	return self->GetCorpseID(corpse);
}

int Lua_Client::GetCorpseItemAt(int corpse, int slot) {
	Lua_Safe_Call_Int();
	return self->GetCorpseItemAt(corpse, slot);
}

void Lua_Client::AssignToInstance(int instance_id) {
	Lua_Safe_Call_Void();
	self->AssignToInstance(instance_id);
}

void Lua_Client::Freeze() {
	Lua_Safe_Call_Void();
	self->SendAppearancePacket(AppearanceType::Animation, Animation::Freeze);
}

void Lua_Client::UnFreeze() {
	Lua_Safe_Call_Void();
	self->SendAppearancePacket(AppearanceType::Animation, Animation::Standing);
}

uint32 Lua_Client::GetAggroCount() {
	Lua_Safe_Call_Int();
	return self->GetAggroCount();
}

uint64 Lua_Client::GetCarriedMoney() {
	Lua_Safe_Call_Int();
	return self->GetCarriedMoney();
}

uint64 Lua_Client::GetAllMoney() {
	Lua_Safe_Call_Int();
	return self->GetAllMoney();
}

uint32 Lua_Client::GetMoney(uint8 type, uint8 subtype) {
	Lua_Safe_Call_Int();
	return self->GetMoney(type, subtype);
}

void Lua_Client::OpenLFGuildWindow() {
	Lua_Safe_Call_Void();
	self->OpenLFGuildWindow();
}

void Lua_Client::NotifyNewTitlesAvailable() {
	Lua_Safe_Call_Void();
	self->NotifyNewTitlesAvailable();
}

void Lua_Client::Signal(int signal_id) {
	Lua_Safe_Call_Void();
	self->Signal(signal_id);
}

void Lua_Client::AddAlternateCurrencyValue(uint32 currency, int amount) {
	Lua_Safe_Call_Void();
	self->AddAlternateCurrencyValue(currency, amount, true);
}

void Lua_Client::SetAlternateCurrencyValue(uint32 currency, uint32 amount) {
	Lua_Safe_Call_Void();
	self->SetAlternateCurrencyValue(currency, amount);
}

uint32 Lua_Client::GetAlternateCurrencyValue(uint32 currency) {
	Lua_Safe_Call_Int();
	return self->GetAlternateCurrencyValue(currency);
}

void Lua_Client::SendWebLink(const char *site) {
	Lua_Safe_Call_Void();
	self->SendWebLink(site);
}

bool Lua_Client::HasSpellScribed(int spell_id) {
	Lua_Safe_Call_Bool();
	return self->HasSpellScribed(spell_id);
}

void Lua_Client::ClearAccountFlag(const std::string& flag) {
	Lua_Safe_Call_Void();
	self->ClearAccountFlag(flag);
}

void Lua_Client::SetAccountFlag(const std::string& flag, const std::string& value) {
	Lua_Safe_Call_Void();
	self->SetAccountFlag(flag, value);
}

std::string Lua_Client::GetAccountFlag(const std::string& flag) {
	Lua_Safe_Call_String();
	return self->GetAccountFlag(flag);
}

luabind::object Lua_Client::GetAccountFlags(lua_State* L) {
	auto t = luabind::newtable(L);
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto l = self->GetAccountFlags();
		int i = 1;
		for (const auto& e : l) {
			t[i] = e;
			i++;
		}
	}

	return t;
}

Lua_Group Lua_Client::GetGroup() {
	Lua_Safe_Call_Class(Lua_Group);
	return self->GetGroup();
}

Lua_Raid Lua_Client::GetRaid() {
	Lua_Safe_Call_Class(Lua_Raid);
	return self->GetRaid();
}

bool Lua_Client::PutItemInInventory(int slot_id, Lua_ItemInst inst) {
	Lua_Safe_Call_Bool();
	EQ::ItemInstance *rinst = inst;
	return self->PutItemInInventory(slot_id, *rinst, true);
}

bool Lua_Client::PushItemOnCursor(Lua_ItemInst inst) {
	Lua_Safe_Call_Bool();
	EQ::ItemInstance *rinst = inst;
	return self->PushItemOnCursor(*rinst, true);
}

Lua_Inventory Lua_Client::GetInventory() {
	Lua_Safe_Call_Class(Lua_Inventory);
	return &self->GetInv();
}

void Lua_Client::SendItemScale(Lua_ItemInst inst) {
	Lua_Safe_Call_Void();
	self->SendItemScale(inst);
}

void Lua_Client::QueuePacket(Lua_Packet app) {
	Lua_Safe_Call_Void();
	self->QueuePacket(app);
}

void Lua_Client::QueuePacket(Lua_Packet app, bool ack_req) {
	Lua_Safe_Call_Void();
	self->QueuePacket(app, ack_req);
}

void Lua_Client::QueuePacket(Lua_Packet app, bool ack_req, int client_connection_status) {
	Lua_Safe_Call_Void();
	self->QueuePacket(app, ack_req, static_cast<Mob::CLIENT_CONN_STATUS>(client_connection_status));
}

void Lua_Client::QueuePacket(Lua_Packet app, bool ack_req, int client_connection_status, int filter) {
	Lua_Safe_Call_Void();
	self->QueuePacket(app, ack_req, static_cast<Mob::CLIENT_CONN_STATUS>(client_connection_status), static_cast<eqFilterType>(filter));
}

int Lua_Client::GetHunger() {
	Lua_Safe_Call_Int();
	return self->GetHunger();
}

int Lua_Client::GetThirst() {
	Lua_Safe_Call_Int();
	return self->GetThirst();
}

void Lua_Client::SetHunger(int in_hunger) {
	Lua_Safe_Call_Void();
	self->SetHunger(in_hunger);
}

void Lua_Client::SetThirst(int in_thirst) {
	Lua_Safe_Call_Void();
	self->SetThirst(in_thirst);
}

void Lua_Client::SetConsumption(int in_hunger, int in_thirst) {
	Lua_Safe_Call_Void();
	self->SetConsumption(in_hunger, in_thirst);
}

void Lua_Client::SendColoredText(uint32 type, std::string msg) {
	Lua_Safe_Call_Void();
	self->SendColoredText(type, msg);
}

void Lua_Client::PlayMP3(std::string file)
{
	Lua_Safe_Call_Void();
	self->PlayMP3(file.c_str());
}

void Lua_Client::QuestReward(Lua_Mob target) {
	Lua_Safe_Call_Void();
	self->QuestReward(target);
}

void Lua_Client::QuestReward(Lua_Mob target, uint32 copper) {
	Lua_Safe_Call_Void();
	self->QuestReward(target, copper);
}

void Lua_Client::QuestReward(Lua_Mob target, uint32 copper, uint32 silver) {
	Lua_Safe_Call_Void();
	self->QuestReward(target, copper, silver);
}

void Lua_Client::QuestReward(Lua_Mob target, uint32 copper, uint32 silver, uint32 gold) {
	Lua_Safe_Call_Void();
	self->QuestReward(target, copper, silver, gold);
}

void Lua_Client::QuestReward(Lua_Mob target, uint32 copper, uint32 silver, uint32 gold, uint32 platinum) {
	Lua_Safe_Call_Void();
	self->QuestReward(target, copper, silver, gold, platinum);
}

void Lua_Client::QuestReward(Lua_Mob target, uint32 copper, uint32 silver, uint32 gold, uint32 platinum, uint32 itemid) {
	Lua_Safe_Call_Void();
	self->QuestReward(target, copper, silver, gold, platinum, itemid);
}

void Lua_Client::QuestReward(Lua_Mob target, uint32 copper, uint32 silver, uint32 gold, uint32 platinum, uint32 itemid, uint32 exp) {
	Lua_Safe_Call_Void();
	self->QuestReward(target, copper, silver, gold, platinum, itemid, exp);
}

void Lua_Client::QuestReward(Lua_Mob target, uint32 copper, uint32 silver, uint32 gold, uint32 platinum, uint32 itemid, uint32 exp, bool faction) {
	Lua_Safe_Call_Void();
	self->QuestReward(target, copper, silver, gold, platinum, itemid, exp, faction);
}

void Lua_Client::QuestReward(Lua_Mob target, luabind::adl::object reward) {
	Lua_Safe_Call_Void();

	if (luabind::type(reward) != LUA_TTABLE) {
		return;
	}

	QuestReward_Struct quest_reward;
	quest_reward.mob_id = 0;
	quest_reward.target_id = self->GetID();
	quest_reward.copper = luabind::type(reward["copper"]) != LUA_TNIL ? luabind::object_cast<uint32>(reward["copper"]) : 0;
	quest_reward.silver = luabind::type(reward["silver"]) != LUA_TNIL ? luabind::object_cast<uint32>(reward["silver"]) : 0;
	quest_reward.gold = luabind::type(reward["gold"]) != LUA_TNIL ? luabind::object_cast<uint32>(reward["gold"]) : 0;
	quest_reward.platinum = luabind::type(reward["platinum"]) != LUA_TNIL ? luabind::object_cast<uint32>(reward["platinum"]) : 0;
	quest_reward.exp_reward = luabind::type(reward["exp"]) != LUA_TNIL ? luabind::object_cast<uint32>(reward["exp"]) : 0;
	quest_reward.faction = 0;
	quest_reward.faction_mod = 0;
	bool faction = false;
	std::fill(std::begin(quest_reward.item_id), std::end(quest_reward.item_id), -1);

	auto item_id = reward["itemid"];
	if (luabind::type(item_id) != LUA_TNIL) {
		quest_reward.item_id[0] = luabind::object_cast<uint32>(item_id);
	}

	// if you define both an itemid and items table, the itemid is thrown away
	// should we error?
	auto items = reward["items"];
	if (luabind::type(items) == LUA_TTABLE) {
		// assume they defined a compatible table
		for (int i = 1; i <= QUESTREWARD_COUNT; ++i) {
			auto item = items[i];
			int cur_value = -1;
			if (luabind::type(item) != LUA_TNIL) {
				cur_value = luabind::object_cast<uint32>(item);
			} else {
				break;
			}
			quest_reward.item_id[i - 1] = cur_value;
		}
	}

	auto lua_faction = reward["faction"];
	if (luabind::type(lua_faction) != LUA_TNIL) {
		// if it's a table it will be {faction, faction_mod}
		if (luabind::type(lua_faction) == LUA_TTABLE) {
			auto item = lua_faction[1];
			if (luabind::type(item) != LUA_TNIL) {
				quest_reward.faction = luabind::object_cast<uint32>(item);
			}
			item = lua_faction[2];
			if (luabind::type(item) != LUA_TNIL) {
				quest_reward.faction_mod = luabind::object_cast<uint32>(item);
			}
		} else {
			faction = luabind::object_cast<bool>(lua_faction);
		}
	}

	self->QuestReward(target, quest_reward, faction);
}

void Lua_Client::CashReward(uint32 copper, uint32 silver, uint32 gold, uint32 platinum) {
	Lua_Safe_Call_Void();
	self->CashReward(copper, silver, gold, platinum);
}

bool Lua_Client::IsDead() {
	Lua_Safe_Call_Bool();
	return self->IsDead();
}

int Lua_Client::CalcCurrentWeight() {
	Lua_Safe_Call_Int();
	return self->CalcCurrentWeight();
}

int Lua_Client::CalcATK() {
	Lua_Safe_Call_Int();
	return self->CalcATK();
}

void Lua_Client::FilteredMessage(Lua_Mob sender, uint32 type, int filter, const char *message)
{
	Lua_Safe_Call_Void();
	self->FilteredMessage(sender, type, (eqFilterType)filter, message);
}

void Lua_Client::EnableAreaHPRegen(int value)
{
	Lua_Safe_Call_Void();
	self->EnableAreaHPRegen(value);
}

void Lua_Client::DisableAreaHPRegen()
{
	Lua_Safe_Call_Void();
	self->DisableAreaHPRegen();
}

void Lua_Client::EnableAreaManaRegen(int value)
{
	Lua_Safe_Call_Void();
	self->EnableAreaManaRegen(value);
}

void Lua_Client::DisableAreaManaRegen()
{
	Lua_Safe_Call_Void();
	self->DisableAreaManaRegen();
}

void Lua_Client::EnableAreaEndRegen(int value)
{
	Lua_Safe_Call_Void();
	self->EnableAreaEndRegen(value);
}

void Lua_Client::DisableAreaEndRegen()
{
	Lua_Safe_Call_Void();
	self->DisableAreaEndRegen();
}

void Lua_Client::EnableAreaRegens(int value)
{
	Lua_Safe_Call_Void();
	self->EnableAreaRegens(value);
}

void Lua_Client::DisableAreaRegens()
{
	Lua_Safe_Call_Void();
	self->DisableAreaRegens();
}

void Lua_Client::SetPrimaryWeaponOrnamentation(uint32 model_id) {
	Lua_Safe_Call_Void();
	self->SetPrimaryWeaponOrnamentation(model_id);
}

void Lua_Client::SetSecondaryWeaponOrnamentation(uint32 model_id) {
	Lua_Safe_Call_Void();
	self->SetSecondaryWeaponOrnamentation(model_id);
}

void Lua_Client::SetClientMaxLevel(uint8 max_level) {
	Lua_Safe_Call_Void();
	self->SetClientMaxLevel(max_level);
}

uint8 Lua_Client::GetClientMaxLevel() {
	Lua_Safe_Call_Int();
	return self->GetClientMaxLevel();
}

void Lua_Client::DialogueWindow(std::string markdown) {
	Lua_Safe_Call_Void();
	DialogueWindow::Render(self, std::move(markdown));
}

DynamicZoneLocation GetDynamicZoneLocationFromTable(const luabind::object& lua_table)
{
	DynamicZoneLocation zone_location;

	if (luabind::type(lua_table) == LUA_TTABLE)
	{
		luabind::object lua_zone = lua_table["zone"];

		// default invalid/missing args to 0
		uint32_t zone_id = 0;
		if (luabind::type(lua_zone) == LUA_TSTRING)
		{
			zone_id = ZoneID(luabind::object_cast<std::string>(lua_zone));
		}
		else if (luabind::type(lua_zone) == LUA_TNUMBER)
		{
			zone_id = luabind::object_cast<uint32_t>(lua_zone);
		}

		float x = (luabind::type(lua_table["x"]) != LUA_TNIL) ? luabind::object_cast<float>(lua_table["x"]) : 0.0f;
		float y = (luabind::type(lua_table["y"]) != LUA_TNIL) ? luabind::object_cast<float>(lua_table["y"]) : 0.0f;
		float z = (luabind::type(lua_table["z"]) != LUA_TNIL) ? luabind::object_cast<float>(lua_table["z"]) : 0.0f;
		float h = (luabind::type(lua_table["h"]) != LUA_TNIL) ? luabind::object_cast<float>(lua_table["h"]) : 0.0f;

		zone_location = { zone_id, x, y, z, h };
	}

	return zone_location;
}

Lua_Expedition Lua_Client::CreateExpedition(luabind::object expedition_table) {
	Lua_Safe_Call_Class(Lua_Expedition);

	if (luabind::type(expedition_table) != LUA_TTABLE)
	{
		return nullptr;
	}

	// luabind will catch thrown cast_failed exceptions for invalid/missing args
	luabind::object instance_info = expedition_table["instance"];
	luabind::object expedition_info = expedition_table["expedition"];
	luabind::object zone = instance_info["zone"];

	uint32_t zone_id = 0;
	if (luabind::type(zone) == LUA_TSTRING)
	{
		zone_id = ZoneID(luabind::object_cast<std::string>(zone));
	}
	else if (luabind::type(zone) == LUA_TNUMBER)
	{
		zone_id = luabind::object_cast<uint32_t>(zone);
	}

	uint32_t zone_version  = luabind::object_cast<uint32_t>(instance_info["version"]);
	uint32_t zone_duration = luabind::object_cast<uint32_t>(instance_info["duration"]);

	DynamicZone dz{ zone_id, zone_version, zone_duration, DynamicZoneType::Expedition };
	dz.SetName(luabind::object_cast<std::string>(expedition_info["name"]));
	dz.SetMinPlayers(luabind::object_cast<uint32_t>(expedition_info["min_players"]));
	dz.SetMaxPlayers(luabind::object_cast<uint32_t>(expedition_info["max_players"]));

	// the dz_info table supports optional hash entries for 'compass', 'safereturn', and 'zonein' data
	if (luabind::type(expedition_table["compass"]) == LUA_TTABLE)
	{
		auto compass_loc = GetDynamicZoneLocationFromTable(expedition_table["compass"]);
		dz.SetCompass(compass_loc);
	}

	if (luabind::type(expedition_table["safereturn"]) == LUA_TTABLE)
	{
		auto safereturn_loc = GetDynamicZoneLocationFromTable(expedition_table["safereturn"]);
		dz.SetSafeReturn(safereturn_loc);
	}

	if (luabind::type(expedition_table["zonein"]) == LUA_TTABLE)
	{
		auto zonein_loc = GetDynamicZoneLocationFromTable(expedition_table["zonein"]);
		dz.SetZoneInLocation(zonein_loc);
	}

	if (luabind::type(expedition_table["switchid"]) == LUA_TNUMBER)
	{
		dz.SetSwitchID(luabind::object_cast<int>(expedition_table["switchid"]));
	}

	bool disable_messages = false;
	if (luabind::type(expedition_info["disable_messages"]) == LUA_TBOOLEAN)
	{
		disable_messages = luabind::object_cast<bool>(expedition_info["disable_messages"]);
	}

	return self->CreateExpedition(dz, disable_messages);
}

Lua_Expedition Lua_Client::CreateExpedition(std::string zone_name, uint32 version, uint32 duration, std::string expedition_name, uint32 min_players, uint32 max_players) {
	Lua_Safe_Call_Class(Lua_Expedition);
	return self->CreateExpedition(ZoneID(zone_name), version, duration, expedition_name, min_players, max_players);
}

Lua_Expedition Lua_Client::CreateExpedition(std::string zone_name, uint32 version, uint32 duration, std::string expedition_name, uint32 min_players, uint32 max_players, bool disable_messages) {
	Lua_Safe_Call_Class(Lua_Expedition);
	return self->CreateExpedition(ZoneID(zone_name), version, duration, expedition_name, min_players, max_players, disable_messages);
}

Lua_Expedition Lua_Client::CreateExpeditionFromTemplate(uint32_t dz_template_id) {
	Lua_Safe_Call_Class(Lua_Expedition);
	return self->CreateExpeditionFromTemplate(dz_template_id);
}

Lua_Expedition Lua_Client::GetExpedition() {
	Lua_Safe_Call_Class(Lua_Expedition);
	return self->GetExpedition();
}

luabind::object Lua_Client::GetExpeditionLockouts(lua_State* L)
{
	auto lua_table = luabind::newtable(L);
	if (d_)
	{
		auto self = reinterpret_cast<NativeType*>(d_);
		const auto& lockouts = self->GetDzLockouts();
		for (const auto& lockout : lockouts)
		{
			auto lockout_table = lua_table[lockout.DzName()];
			if (luabind::type(lockout_table) != LUA_TTABLE)
			{
				lockout_table = luabind::newtable(L);
			}
			lockout_table[lockout.Event()] = lockout.GetSecondsRemaining();
		}
	}
	return lua_table;
}

luabind::object Lua_Client::GetExpeditionLockouts(lua_State* L, std::string expedition_name)
{
	auto lua_table = luabind::newtable(L);
	if (d_)
	{
		auto self = reinterpret_cast<NativeType*>(d_);
		const auto& lockouts = self->GetDzLockouts();
		for (const auto& lockout : lockouts)
		{
			if (lockout.DzName() == expedition_name)
			{
				lua_table[lockout.Event()] = lockout.GetSecondsRemaining();
			}
		}
	}
	return lua_table;
}

std::string Lua_Client::GetLockoutExpeditionUUID(std::string expedition_name, std::string event_name) {
	Lua_Safe_Call_String();
	std::string uuid;
	auto lockout = self->GetDzLockout(expedition_name, event_name);
	if (lockout)
	{
		uuid = lockout->UUID();
	}
	return uuid;
}

void Lua_Client::AddExpeditionLockout(std::string expedition_name, std::string event_name, uint32 seconds) {
	Lua_Safe_Call_Void();
	self->AddDzLockout(expedition_name, event_name, seconds);
}

void Lua_Client::AddExpeditionLockout(std::string expedition_name, std::string event_name, uint32 seconds, std::string uuid) {
	Lua_Safe_Call_Void();
	self->AddDzLockout(expedition_name, event_name, seconds, uuid);
}

void Lua_Client::AddExpeditionLockoutDuration(std::string expedition_name, std::string event_name, int seconds) {
	Lua_Safe_Call_Void();
	auto lockout = DzLockout::Create(expedition_name, event_name, seconds);
	self->AddDzLockoutDuration(lockout, seconds, {}, true);
}

void Lua_Client::AddExpeditionLockoutDuration(std::string expedition_name, std::string event_name, int seconds, std::string uuid) {
	Lua_Safe_Call_Void();
	auto lockout = DzLockout::Create(expedition_name, event_name, seconds, uuid);
	self->AddDzLockoutDuration(lockout, seconds, uuid, true);
}

void Lua_Client::RemoveAllExpeditionLockouts() {
	Lua_Safe_Call_Void();
	self->RemoveDzLockouts({}, true);
}

void Lua_Client::RemoveAllExpeditionLockouts(std::string expedition_name) {
	Lua_Safe_Call_Void();
	self->RemoveDzLockouts(expedition_name, true);
}

void Lua_Client::RemoveExpeditionLockout(std::string expedition_name, std::string event_name) {
	Lua_Safe_Call_Void();
	self->RemoveDzLockout(expedition_name, event_name, true);
}

bool Lua_Client::HasExpeditionLockout(std::string expedition_name, std::string event_name) {
	Lua_Safe_Call_Bool();
	return self->HasDzLockout(expedition_name, event_name);
}

void Lua_Client::MovePCDynamicZone(uint32 zone_id) {
	Lua_Safe_Call_Void();
	return self->MovePCDynamicZone(zone_id);
}

void Lua_Client::MovePCDynamicZone(uint32 zone_id, int zone_version) {
	Lua_Safe_Call_Void();
	return self->MovePCDynamicZone(zone_id, zone_version);
}

void Lua_Client::MovePCDynamicZone(uint32 zone_id, int zone_version, bool msg_if_invalid) {
	Lua_Safe_Call_Void();
	return self->MovePCDynamicZone(zone_id, zone_version, msg_if_invalid);
}

void Lua_Client::MovePCDynamicZone(std::string zone_name) {
	Lua_Safe_Call_Void();
	return self->MovePCDynamicZone(zone_name);
}

void Lua_Client::MovePCDynamicZone(std::string zone_name, int zone_version) {
	Lua_Safe_Call_Void();
	return self->MovePCDynamicZone(zone_name, zone_version);
}

void Lua_Client::MovePCDynamicZone(std::string zone_name, int zone_version, bool msg_if_invalid) {
	Lua_Safe_Call_Void();
	return self->MovePCDynamicZone(zone_name, zone_version, msg_if_invalid);
}

void Lua_Client::CreateTaskDynamicZone(int task_id, luabind::object dz_table) {
	Lua_Safe_Call_Void();

	if (luabind::type(dz_table) != LUA_TTABLE)
	{
		return;
	}

	// luabind will catch thrown cast_failed exceptions for invalid/missing args
	luabind::object instance_info = dz_table["instance"];
	luabind::object zone = instance_info["zone"];

	uint32_t zone_id = 0;
	if (luabind::type(zone) == LUA_TSTRING)
	{
		zone_id = ZoneID(luabind::object_cast<std::string>(zone));
	}
	else if (luabind::type(zone) == LUA_TNUMBER)
	{
		zone_id = luabind::object_cast<uint32_t>(zone);
	}

	uint32_t zone_version  = luabind::object_cast<uint32_t>(instance_info["version"]);

	// tasks override dz duration so duration is ignored here
	DynamicZone dz{ zone_id, zone_version, 0, DynamicZoneType::None };

	// the dz_info table supports optional hash entries for 'compass', 'safereturn', and 'zonein' data
	if (luabind::type(dz_table["compass"]) == LUA_TTABLE)
	{
		auto compass_loc = GetDynamicZoneLocationFromTable(dz_table["compass"]);
		dz.SetCompass(compass_loc);
	}

	if (luabind::type(dz_table["safereturn"]) == LUA_TTABLE)
	{
		auto safereturn_loc = GetDynamicZoneLocationFromTable(dz_table["safereturn"]);
		dz.SetSafeReturn(safereturn_loc);
	}

	if (luabind::type(dz_table["zonein"]) == LUA_TTABLE)
	{
		auto zonein_loc = GetDynamicZoneLocationFromTable(dz_table["zonein"]);
		dz.SetZoneInLocation(zonein_loc);
	}

	if (luabind::type(dz_table["switchid"]) == LUA_TNUMBER)
	{
		dz.SetSwitchID(luabind::object_cast<int>(dz_table["switchid"]));
	}

	self->CreateTaskDynamicZone(task_id, dz);
}

void Lua_Client::Fling(float target_x, float target_y, float target_z) {
	Lua_Safe_Call_Void();
	self->Fling(0, target_x, target_y, target_z, false, false, true);
}

void Lua_Client::Fling(float target_x, float target_y, float target_z, bool ignore_los) {
	Lua_Safe_Call_Void();
	self->Fling(0, target_x, target_y, target_z, ignore_los, false, true);
}

void Lua_Client::Fling(float target_x, float target_y, float target_z, bool ignore_los, bool clip_through_walls) {
	Lua_Safe_Call_Void();
	self->Fling(0, target_x, target_y, target_z, ignore_los, clip_through_walls, true);
}

void Lua_Client::Fling(float value, float target_x, float target_y, float target_z) {
	Lua_Safe_Call_Void();
	self->Fling(value, target_x, target_y, target_z);
}

void Lua_Client::Fling(float value, float target_x, float target_y, float target_z, bool ignore_los) {
	Lua_Safe_Call_Void();
	self->Fling(value, target_x, target_y, target_z, ignore_los);
}

void Lua_Client::Fling(float value, float target_x, float target_y, float target_z, bool ignore_los, bool clip_through_walls) {
	Lua_Safe_Call_Void();
	self->Fling(value, target_x, target_y, target_z, ignore_los, clip_through_walls);
}

float Lua_Client::GetAAEXPModifier() {
	Lua_Safe_Call_Real();
	return zone->GetAAEXPModifier(self);
}

float Lua_Client::GetAAEXPModifier(uint32 zone_id) {
	Lua_Safe_Call_Real();
	return self->GetAAEXPModifier(zone_id);
}

float Lua_Client::GetAAEXPModifier(uint32 zone_id, int16 instance_version) {
	Lua_Safe_Call_Real();
	return self->GetAAEXPModifier(zone_id, instance_version);
}

float Lua_Client::GetEXPModifier() {
	Lua_Safe_Call_Real();
	return zone->GetEXPModifier(self);
}

float Lua_Client::GetEXPModifier(uint32 zone_id) {
	Lua_Safe_Call_Real();
	return self->GetEXPModifier(zone_id);
}

float Lua_Client::GetEXPModifier(uint32 zone_id, int16 instance_version) {
	Lua_Safe_Call_Real();
	return self->GetEXPModifier(zone_id, instance_version);
}

void Lua_Client::SetAAEXPModifier(float aa_modifier) {
	Lua_Safe_Call_Void();
	zone->SetAAEXPModifier(self, aa_modifier);
}

void Lua_Client::SetAAEXPModifier(uint32 zone_id, float aa_modifier) {
	Lua_Safe_Call_Void();
	self->SetAAEXPModifier(zone_id, aa_modifier);
}

void Lua_Client::SetAAEXPModifier(uint32 zone_id, float aa_modifier, int16 instance_version) {
	Lua_Safe_Call_Void();
	self->SetAAEXPModifier(zone_id, aa_modifier, instance_version);
}

void Lua_Client::SetEXPModifier(float exp_modifier) {
	Lua_Safe_Call_Void();
	zone->SetEXPModifier(self, exp_modifier);
}

void Lua_Client::SetEXPModifier(uint32 zone_id, float exp_modifier) {
	Lua_Safe_Call_Void();
	self->SetEXPModifier(zone_id, exp_modifier);
}

void Lua_Client::SetEXPModifier(uint32 zone_id, float exp_modifier, int16 instance_version) {
	Lua_Safe_Call_Void();
	self->SetEXPModifier(zone_id, exp_modifier, instance_version);
}

void Lua_Client::AddLDoNLoss(uint32 theme_id) {
	Lua_Safe_Call_Void();
	self->UpdateLDoNWinLoss(theme_id);
}

void Lua_Client::AddLDoNWin(uint32 theme_id) {
	Lua_Safe_Call_Void();
	self->UpdateLDoNWinLoss(theme_id, true);
}

void Lua_Client::SetHideMe(bool hide_me_state) {
	Lua_Safe_Call_Void();
	self->SetHideMe(hide_me_state);
}

void Lua_Client::Popup(const char* title, const char* text) {
	Lua_Safe_Call_Void();
	self->SendFullPopup(title, text);
}

void Lua_Client::Popup(const char* title, const char* text, uint32 popup_id) {
	Lua_Safe_Call_Void();
	self->SendFullPopup(title, text, popup_id);
}

void Lua_Client::Popup(const char* title, const char* text, uint32 popup_id, uint32 negative_id) {
	Lua_Safe_Call_Void();
	self->SendFullPopup(title, text, popup_id, negative_id);
}

void Lua_Client::Popup(const char* title, const char* text, uint32 popup_id, uint32 negative_id, uint32 button_type) {
	Lua_Safe_Call_Void();
	self->SendFullPopup(title, text, popup_id, negative_id, button_type);
}

void Lua_Client::Popup(const char* title, const char* text, uint32 popup_id, uint32 negative_id, uint32 button_type, uint32 duration) {
	Lua_Safe_Call_Void();
	self->SendFullPopup(title, text, popup_id, negative_id, button_type, duration);
}

void Lua_Client::Popup(const char* title, const char* text, uint32 popup_id, uint32 negative_id, uint32 button_type, uint32 duration, const char* button_name_one, const char* button_name_two) {
	Lua_Safe_Call_Void();
	self->SendFullPopup(title, text, popup_id, negative_id, button_type, duration, button_name_one, button_name_two);
}

void Lua_Client::Popup(const char* title, const char* text, uint32 popup_id, uint32 negative_id, uint32 button_type, uint32 duration, const char* button_name_one, const char* button_name_two, uint32 sound_controls) {
	Lua_Safe_Call_Void();
	self->SendFullPopup(title, text, popup_id, negative_id, button_type, duration, button_name_one, button_name_two, sound_controls);
}

void Lua_Client::ResetAllDisciplineTimers() {
	Lua_Safe_Call_Void();
	self->ResetAllDisciplineTimers();
}

void Lua_Client::SendToInstance(std::string instance_type, std::string zone_short_name, uint32 instance_version, float x, float y, float z, float heading, std::string instance_identifier, uint32 duration) {
	Lua_Safe_Call_Void();
	self->SendToInstance(instance_type, zone_short_name, instance_version, x, y, z, heading, instance_identifier, duration);
}

uint32 Lua_Client::CountItem(uint32 item_id) {
	Lua_Safe_Call_Int();
	return self->CountItem(item_id);
}

void Lua_Client::RemoveItem(uint32 item_id) {
	Lua_Safe_Call_Void();
	self->RemoveItem(item_id);
}

void Lua_Client::RemoveItem(uint32 item_id, uint32 quantity) {
	Lua_Safe_Call_Void();
	self->RemoveItem(item_id, quantity);
}

void Lua_Client::SetGMStatus(int new_status) {
	Lua_Safe_Call_Void();
	self->SetGMStatus(new_status);
}

int16 Lua_Client::GetGMStatus() {
	Lua_Safe_Call_Int();
	return self->Admin();
}

void Lua_Client::UntrainDiscBySpellID(uint16 spell_id) {
	Lua_Safe_Call_Void();
	self->UntrainDiscBySpellID(spell_id);
}

void Lua_Client::UntrainDiscBySpellID(uint16 spell_id, bool update_client) {
	Lua_Safe_Call_Void();
	self->UntrainDiscBySpellID(spell_id, update_client);
}

int Lua_Client::GetIPExemption() {
	Lua_Safe_Call_Int();
	return self->GetIPExemption();
}

std::string Lua_Client::GetIPString() {
	Lua_Safe_Call_String();
	return self->GetIPString();
}

void Lua_Client::SetIPExemption(int exemption_amount) {
	Lua_Safe_Call_Void();
	self->SetIPExemption(exemption_amount);
}

void Lua_Client::ReadBookByName(std::string book_name, uint8 book_type) {
	Lua_Safe_Call_Void();
	self->ReadBookByName(book_name, book_type);
}

void Lua_Client::SummonBaggedItems(uint32 bag_item_id, luabind::adl::object bag_items_table) {
	Lua_Safe_Call_Void();
	if (luabind::type(bag_items_table) != LUA_TTABLE) {
		return;
	}

	std::vector<LootItem> bagged_items;

	luabind::raw_iterator end; // raw_iterator uses lua_rawget
	for (luabind::raw_iterator it(bag_items_table); it != end; ++it)
	{
		// verify array element is a table for item details
		if (luabind::type(*it) == LUA_TTABLE)
		{
			// no need to try/catch, quest lua parser already catches exceptions
			LootItem item{};
			item.item_id = luabind::object_cast<uint32>((*it)["item_id"]);
			item.charges = luabind::object_cast<int16>((*it)["charges"]);
			item.attuned = luabind::type((*it)["attuned"]) != LUA_TNIL ? luabind::object_cast<uint8>((*it)["attuned"]) : 0;
			item.aug_1 = luabind::type((*it)["augment_one"]) != LUA_TNIL ? luabind::object_cast<uint32>((*it)["augment_one"]) : 0;
			item.aug_2 = luabind::type((*it)["augment_two"]) != LUA_TNIL ? luabind::object_cast<uint32>((*it)["augment_two"]) : 0;
			item.aug_3 = luabind::type((*it)["augment_three"]) != LUA_TNIL ? luabind::object_cast<uint32>((*it)["augment_three"]) : 0;
			item.aug_4 = luabind::type((*it)["augment_four"]) != LUA_TNIL ? luabind::object_cast<uint32>((*it)["augment_four"]) : 0;
			item.aug_5 = luabind::type((*it)["augment_five"]) != LUA_TNIL ? luabind::object_cast<uint32>((*it)["augment_five"]) : 0;
			item.aug_6 = luabind::type((*it)["augment_six"]) != LUA_TNIL ? luabind::object_cast<uint32>((*it)["augment_six"]) : 0;
			bagged_items.emplace_back(item);
		}
	}

	self->SummonBaggedItems(bag_item_id, bagged_items);
}

void Lua_Client::RemoveLDoNLoss(uint32 theme_id) {
	Lua_Safe_Call_Void();
	self->UpdateLDoNWinLoss(theme_id, false, true);
}

void Lua_Client::RemoveLDoNWin(uint32 theme_id) {
	Lua_Safe_Call_Void();
	self->UpdateLDoNWinLoss(theme_id, true, true);
}

uint16 Lua_Client::ScribeSpells(uint8 min_level, uint8 max_level) {
	Lua_Safe_Call_Int();
	return self->ScribeSpells(min_level, max_level);
}

uint16 Lua_Client::LearnDisciplines(uint8 min_level, uint8 max_level) {
	Lua_Safe_Call_Int();
	return self->LearnDisciplines(min_level, max_level);
}

int Lua_Client::GetNextAvailableDisciplineSlot() {
	Lua_Safe_Call_Int();
	return self->GetNextAvailableDisciplineSlot();
}

int Lua_Client::GetNextAvailableDisciplineSlot(int starting_slot) {
	Lua_Safe_Call_Int();
	return self->GetNextAvailableDisciplineSlot(starting_slot);
}

void Lua_Client::ResetCastbarCooldownBySlot(int slot) {
	Lua_Safe_Call_Void();
	self->ResetCastbarCooldownBySlot(slot);
}

void Lua_Client::ResetAllCastbarCooldowns() {
	Lua_Safe_Call_Void();
	self->ResetAllCastbarCooldowns();
}

void Lua_Client::ResetCastbarCooldownBySpellID(uint32 spell_id) {
	Lua_Safe_Call_Void();
	self->ResetCastbarCooldownBySpellID(spell_id);
}

void Lua_Client::UnscribeSpellBySpellID(uint16 spell_id) {
	Lua_Safe_Call_Void();
	self->UnscribeSpellBySpellID(spell_id);
}

void Lua_Client::UnscribeSpellBySpellID(uint16 spell_id, bool update_client) {
	Lua_Safe_Call_Void();
	self->UnscribeSpellBySpellID(spell_id, update_client);
}

int Lua_Client::GetEnvironmentDamageModifier() {
	Lua_Safe_Call_Int();
	return self->GetEnvironmentDamageModifier();
}

void Lua_Client::SetEnvironmentDamageModifier(int value) {
	Lua_Safe_Call_Void();
	self->SetEnvironmentDamageModifier(value);
}

bool Lua_Client::GetInvulnerableEnvironmentDamage() {
	Lua_Safe_Call_Bool();
	return self->GetInvulnerableEnvironmentDamage();
}

void Lua_Client::SetInvulnerableEnvironmentDamage(bool value) {
	Lua_Safe_Call_Void();
	self->SetInvulnerableEnvironmentDamage(value);
}

void Lua_Client::AddItem(luabind::object item_table) {
	Lua_Safe_Call_Void();
	if (luabind::type(item_table) != LUA_TTABLE) {
		return;
	}

	uint32 item_id = luabind::object_cast<uint32>(item_table["item_id"]);
	int16 charges = luabind::object_cast<uint32>(item_table["charges"]);
	uint32 augment_one = luabind::type(item_table["augment_one"]) != LUA_TNIL ? luabind::object_cast<uint32>(item_table["augment_one"]) : 0;
	uint32 augment_two = luabind::type(item_table["augment_two"]) != LUA_TNIL ? luabind::object_cast<uint32>(item_table["augment_two"]) : 0;
	uint32 augment_three = luabind::type(item_table["augment_three"]) != LUA_TNIL ? luabind::object_cast<uint32>(item_table["augment_three"]) : 0;
	uint32 augment_four = luabind::type(item_table["augment_four"]) != LUA_TNIL ? luabind::object_cast<uint32>(item_table["augment_four"]) : 0;
	uint32 augment_five = luabind::type(item_table["augment_five"]) != LUA_TNIL ? luabind::object_cast<uint32>(item_table["augment_five"]) : 0;
	uint32 augment_six = luabind::type(item_table["augment_six"]) != LUA_TNIL ? luabind::object_cast<uint32>(item_table["augment_six"]) : 0;
	bool attuned = luabind::type(item_table["attuned"]) != LUA_TNIL ? luabind::object_cast<bool>(item_table["attuned"]) : false;
	uint16 slot_id = luabind::type(item_table["slot_id"]) != LUA_TNIL ? luabind::object_cast<uint16>(item_table["slot_id"]) : EQ::invslot::slotCursor;

	self->SummonItem(
		item_id,
		charges,
		augment_one,
		augment_two,
		augment_three,
		augment_four,
		augment_five,
		augment_six,
		attuned,
		slot_id
	);
}

uint32 Lua_Client::CountAugmentEquippedByID(uint32 item_id) {
	Lua_Safe_Call_Int();
	return self->GetInv().CountAugmentEquippedByID(item_id);
}

bool Lua_Client::HasAugmentEquippedByID(uint32 item_id) {
	Lua_Safe_Call_Bool();
	return self->GetInv().HasAugmentEquippedByID(item_id);
}

uint32 Lua_Client::CountItemEquippedByID(uint32 item_id) {
	Lua_Safe_Call_Int();
	return self->GetInv().CountItemEquippedByID(item_id);
}

bool Lua_Client::HasItemEquippedByID(uint32 item_id) {
	Lua_Safe_Call_Bool();
	return self->GetInv().HasItemEquippedByID(item_id);
}

void Lua_Client::AddPlatinum(uint32 platinum) {
	Lua_Safe_Call_Void();
	self->AddPlatinum(platinum);
}

void Lua_Client::AddPlatinum(uint32 platinum, bool update_client) {
	Lua_Safe_Call_Void();
	self->AddPlatinum(platinum, update_client);
}

uint32 Lua_Client::GetCarriedPlatinum() {
	Lua_Safe_Call_Int();
	return self->GetCarriedPlatinum();
}

bool Lua_Client::TakePlatinum(uint32 platinum) {
	Lua_Safe_Call_Bool();
	return self->TakePlatinum(platinum);
}

bool Lua_Client::TakePlatinum(uint32 platinum, bool update_client) {
	Lua_Safe_Call_Bool();
	return self->TakePlatinum(platinum, update_client);
}

void Lua_Client::LoadZoneFlags() {
	Lua_Safe_Call_Void();
	self->LoadZoneFlags();
}

void Lua_Client::ClearPEQZoneFlag(uint32 zone_id) {
	Lua_Safe_Call_Void();
	self->ClearPEQZoneFlag(zone_id);
}

bool Lua_Client::HasPEQZoneFlag(uint32 zone_id) {
	Lua_Safe_Call_Bool();
	return self->HasPEQZoneFlag(zone_id);
}

void Lua_Client::LoadPEQZoneFlags() {
	Lua_Safe_Call_Void();
	self->LoadPEQZoneFlags();
}

void Lua_Client::SendPEQZoneFlagInfo(Lua_Client to) {
	Lua_Safe_Call_Void();
	self->SendPEQZoneFlagInfo(to);
}

void Lua_Client::SetPEQZoneFlag(uint32 zone_id) {
	Lua_Safe_Call_Void();
	self->SetPEQZoneFlag(zone_id);
}

int Lua_Client::GetHealAmount() {
	Lua_Safe_Call_Int();
	return self->GetHealAmt();
}

int Lua_Client::GetSpellDamage() {
	Lua_Safe_Call_Int();
	return self->GetSpellDmg();
}
int Lua_Client::GetIntoxication() {
	Lua_Safe_Call_Int();
	return self->GetIntoxication();
}

void Lua_Client::TaskSelector(luabind::adl::object table) {
	TaskSelector(table, false);
}

void Lua_Client::TaskSelector(luabind::adl::object table, bool ignore_cooldown) {
	Lua_Safe_Call_Void();

	if(luabind::type(table) != LUA_TTABLE) {
		return;
	}

	std::vector<int> tasks;
	for(int i = 1; i <= MAXCHOOSERENTRIES; ++i) {
		auto cur = table[i];
		if (luabind::type(cur) == LUA_TNUMBER) {
			tasks.push_back(luabind::object_cast<int>(cur));
		}
	}

	self->TaskQuestSetSelector(self, tasks, ignore_cooldown);
}

bool Lua_Client::TeleportToPlayerByCharID(uint32 character_id) {
	Lua_Safe_Call_Bool();
	return self->GotoPlayer(database.GetCharNameByID(character_id));
}

bool Lua_Client::TeleportToPlayerByName(std::string player_name) {
	Lua_Safe_Call_Bool();
	return self->GotoPlayer(player_name);
}

bool Lua_Client::TeleportGroupToPlayerByCharID(uint32 character_id) {
	Lua_Safe_Call_Bool();
	return self->GotoPlayerGroup(database.GetCharNameByID(character_id));
}

bool Lua_Client::TeleportGroupToPlayerByName(std::string player_name) {
	Lua_Safe_Call_Bool();
	return self->GotoPlayerGroup(player_name);
}

bool Lua_Client::TeleportRaidToPlayerByCharID(uint32 character_id) {
	Lua_Safe_Call_Bool();
	return self->GotoPlayerRaid(database.GetCharNameByID(character_id));
}

bool Lua_Client::TeleportRaidToPlayerByName(std::string player_name) {
	Lua_Safe_Call_Bool();
	return self->GotoPlayerRaid(player_name);
}

int Lua_Client::GetRecipeMadeCount(uint32 recipe_id) {
	Lua_Safe_Call_Int();
	return self->GetRecipeMadeCount(recipe_id);
}

bool Lua_Client::HasRecipeLearned(uint32 recipe_id) {
	Lua_Safe_Call_Bool();
	return self->HasRecipeLearned(recipe_id);
}

bool Lua_Client::SendGMCommand(std::string message) {
	Lua_Safe_Call_Bool();
	return self->SendGMCommand(message);
}

bool Lua_Client::SendGMCommand(std::string message, bool ignore_status) {
	Lua_Safe_Call_Bool();
	return self->SendGMCommand(message, ignore_status);
}

void Lua_Client::SendMarqueeMessage(uint32 type, std::string message) {
	Lua_Safe_Call_Void();
	self->SendMarqueeMessage(type, message);
}

void Lua_Client::SendMarqueeMessage(uint32 type, std::string message, uint32 duration) {
	Lua_Safe_Call_Void();
	self->SendMarqueeMessage(type, message, duration);
}

void Lua_Client::SendMarqueeMessage(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, std::string message) {
	Lua_Safe_Call_Void();
	self->SendMarqueeMessage(type, priority, fade_in, fade_out, duration, message);
}

void Lua_Client::MoveZone(const char *zone_short_name) {
	Lua_Safe_Call_Void();
	self->MoveZone(zone_short_name);
}

void Lua_Client::MoveZone(const char *zone_short_name, float x, float y, float z) {
	Lua_Safe_Call_Void();
	self->MoveZone(zone_short_name, glm::vec4(x, y, z, 0.0f));
}

void Lua_Client::MoveZone(const char *zone_short_name, float x, float y, float z, float heading) {
	Lua_Safe_Call_Void();
	self->MoveZone(zone_short_name, glm::vec4(x, y, z, heading));
}

void Lua_Client::MoveZoneGroup(const char *zone_short_name) {
	Lua_Safe_Call_Void();
	self->MoveZoneGroup(zone_short_name);
}

void Lua_Client::MoveZoneGroup(const char *zone_short_name, float x, float y, float z) {
	Lua_Safe_Call_Void();
	self->MoveZoneGroup(zone_short_name, glm::vec4(x, y, z, 0.0f));
}

void Lua_Client::MoveZoneGroup(const char *zone_short_name, float x, float y, float z, float heading) {
	Lua_Safe_Call_Void();
	self->MoveZoneGroup(zone_short_name, glm::vec4(x, y, z, heading));
}

void Lua_Client::MoveZoneRaid(const char *zone_short_name) {
	Lua_Safe_Call_Void();
	self->MoveZoneRaid(zone_short_name);
}

void Lua_Client::MoveZoneRaid(const char *zone_short_name, float x, float y, float z) {
	Lua_Safe_Call_Void();
	self->MoveZoneRaid(zone_short_name, glm::vec4(x, y, z, 0.0f));
}

void Lua_Client::MoveZoneRaid(const char *zone_short_name, float x, float y, float z, float heading) {
	Lua_Safe_Call_Void();
	self->MoveZoneRaid(zone_short_name, glm::vec4(x, y, z, heading));
}

void Lua_Client::MoveZoneInstance(uint16 instance_id) {
	Lua_Safe_Call_Void();
	self->MoveZoneInstance(instance_id);
}

void Lua_Client::MoveZoneInstance(uint16 instance_id, float x, float y, float z) {
	Lua_Safe_Call_Void();
	self->MoveZoneInstance(instance_id, glm::vec4(x, y, z, 0.0f));
}

void Lua_Client::MoveZoneInstance(uint16 instance_id, float x, float y, float z, float heading) {
	Lua_Safe_Call_Void();
	self->MoveZoneInstance(instance_id, glm::vec4(x, y, z, heading));
}

void Lua_Client::MoveZoneInstanceGroup(uint16 instance_id) {
	Lua_Safe_Call_Void();
	self->MoveZoneInstanceGroup(instance_id);
}

void Lua_Client::MoveZoneInstanceGroup(uint16 instance_id, float x, float y, float z) {
	Lua_Safe_Call_Void();
	self->MoveZoneInstanceGroup(instance_id, glm::vec4(x, y, z, 0.0f));
}

void Lua_Client::MoveZoneInstanceGroup(uint16 instance_id, float x, float y, float z, float heading) {
	Lua_Safe_Call_Void();
	self->MoveZoneInstanceGroup(instance_id, glm::vec4(x, y, z, heading));
}

void Lua_Client::MoveZoneInstanceRaid(uint16 instance_id) {
	Lua_Safe_Call_Void();
	self->MoveZoneInstanceRaid(instance_id);
}

void Lua_Client::MoveZoneInstanceRaid(uint16 instance_id, float x, float y, float z) {
	Lua_Safe_Call_Void();
	self->MoveZoneInstanceRaid(instance_id, glm::vec4(x, y, z, 0.0f));
}

void Lua_Client::MoveZoneInstanceRaid(uint16 instance_id, float x, float y, float z, float heading) {
	Lua_Safe_Call_Void();
	self->MoveZoneInstanceRaid(instance_id, glm::vec4(x, y, z, heading));
}

void Lua_Client::ApplySpell(int spell_id) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id);
}

void Lua_Client::ApplySpell(int spell_id, int duration) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration);
}

void Lua_Client::ApplySpell(int spell_id, int duration, int level) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration, level);
}

void Lua_Client::ApplySpell(int spell_id, int duration, int level, bool allow_pets) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Solo, allow_pets);
}

void Lua_Client::ApplySpell(int spell_id, int duration, int level, bool allow_pets, bool allow_bots) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Solo, allow_pets, true, allow_bots);
}

void Lua_Client::ApplySpellGroup(int spell_id) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, 0, -1, ApplySpellType::Group);
}

void Lua_Client::ApplySpellGroup(int spell_id, int duration) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration, -1, ApplySpellType::Group);
}

void Lua_Client::ApplySpellGroup(int spell_id, int duration, int level) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Group);
}

void Lua_Client::ApplySpellGroup(int spell_id, int duration, int level, bool allow_pets) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Group, allow_pets);
}

void Lua_Client::ApplySpellGroup(int spell_id, int duration, int level, bool allow_pets, bool allow_bots) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Group, allow_pets, true, allow_bots);
}

void Lua_Client::ApplySpellRaid(int spell_id) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, 0, -1, ApplySpellType::Raid);
}

void Lua_Client::ApplySpellRaid(int spell_id, int duration) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration, -1, ApplySpellType::Raid);
}

void Lua_Client::ApplySpellRaid(int spell_id, int duration, int level) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Raid);
}

void Lua_Client::ApplySpellRaid(int spell_id, int duration, int level, bool allow_pets) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Raid, allow_pets);
}

void Lua_Client::ApplySpellRaid(int spell_id, int duration, int level, bool allow_pets, bool is_raid_group_only) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Raid, allow_pets, is_raid_group_only);
}

void Lua_Client::ApplySpellRaid(int spell_id, int duration, int level, bool allow_pets, bool is_raid_group_only, bool allow_bots) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Raid, allow_pets, is_raid_group_only, allow_bots);
}

void Lua_Client::SetSpellDuration(int spell_id) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id);
}

void Lua_Client::SetSpellDuration(int spell_id, int duration) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration);
}

void Lua_Client::SetSpellDuration(int spell_id, int duration, int level) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration, level);
}

void Lua_Client::SetSpellDuration(int spell_id, int duration, int level, bool allow_pets) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration, level, ApplySpellType::Solo, allow_pets);
}

void Lua_Client::SetSpellDuration(int spell_id, int duration, int level, bool allow_pets, bool allow_bots) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration, level, ApplySpellType::Solo, allow_pets, true, allow_bots);
}

void Lua_Client::SetSpellDurationGroup(int spell_id) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, 0, -1, ApplySpellType::Group);
}

void Lua_Client::SetSpellDurationGroup(int spell_id, int duration) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration, -1, ApplySpellType::Group);
}

void Lua_Client::SetSpellDurationGroup(int spell_id, int duration, int level) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration, level, ApplySpellType::Group);
}

void Lua_Client::SetSpellDurationGroup(int spell_id, int duration, int level, bool allow_pets) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration, level, ApplySpellType::Group, allow_pets);
}

void Lua_Client::SetSpellDurationGroup(int spell_id, int duration, int level, bool allow_pets, bool allow_bots) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration, level, ApplySpellType::Group, allow_pets, true, allow_bots);
}

void Lua_Client::SetSpellDurationRaid(int spell_id) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, 0, -1, ApplySpellType::Raid);
}

void Lua_Client::SetSpellDurationRaid(int spell_id, int duration) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration, -1, ApplySpellType::Raid);
}

void Lua_Client::SetSpellDurationRaid(int spell_id, int duration, int level) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration, level, ApplySpellType::Raid);
}

void Lua_Client::SetSpellDurationRaid(int spell_id, int duration, int level, bool allow_pets) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration, level, ApplySpellType::Raid, allow_pets);
}

void Lua_Client::SetSpellDurationRaid(int spell_id, int duration, int level, bool allow_pets, bool is_raid_group_only) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration, level, ApplySpellType::Raid, allow_pets, is_raid_group_only);
}

void Lua_Client::SetSpellDurationRaid(int spell_id, int duration, int level, bool allow_pets, bool is_raid_group_only, bool allow_bots) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration, level, ApplySpellType::Group, allow_pets, is_raid_group_only, allow_bots);
}

void Lua_Client::UpdateAdmin() {
	Lua_Safe_Call_Void();
	self->UpdateAdmin();
}

void Lua_Client::UpdateAdmin(bool from_database) {
	Lua_Safe_Call_Void();
	self->UpdateAdmin(from_database);
}

luabind::object Lua_Client::GetPEQZoneFlags(lua_State* L) {
	auto t = luabind::newtable(L);
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto l = self->GetPEQZoneFlags();
		int i = 1;
		for (const auto& f : l) {
			t[i] = f;
			i++;
		}
	}

	return t;
}

luabind::object Lua_Client::GetZoneFlags(lua_State* L) {
	auto t = luabind::newtable(L);
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto l = self->GetZoneFlags();
		int i = 1;
		for (const auto& f : l) {
			t[i] = f;
			i++;
		}
	}

	return t;
}

void Lua_Client::SendPayload(int payload_id) {
	Lua_Safe_Call_Void();
	self->SendPayload(payload_id);
}

void Lua_Client::SendPayload(int payload_id, std::string payload_value) {
	Lua_Safe_Call_Void();
	self->SendPayload(payload_id, payload_value);
}

std::string Lua_Client::GetGuildPublicNote()
{
	Lua_Safe_Call_String();
	return self->GetGuildPublicNote();
}

void Lua_Client::MaxSkills()
{
	Lua_Safe_Call_Void();
	self->MaxSkills();
}

luabind::object Lua_Client::GetAugmentIDsBySlotID(lua_State* L, int16 slot_id) {
	auto lua_table = luabind::newtable(L);
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto augments = self->GetInv().GetAugmentIDsBySlotID(slot_id);
		int index = 1;
		for (auto item_id : augments) {
			lua_table[index] = item_id;
			index++;
		}
	}
	return lua_table;
}

bool Lua_Client::IsEXPEnabled() {
	Lua_Safe_Call_Bool();
	return self->IsEXPEnabled();
}

void Lua_Client::SetEXPEnabled(bool is_exp_enabled) {
	Lua_Safe_Call_Void();
	self->SetEXPEnabled(is_exp_enabled);
}

uint64 Lua_Client::CalcEXP(uint8 consider_level) {
	Lua_Safe_Call_Int();
	return self->CalcEXP(consider_level);
}

uint64 Lua_Client::CalcEXP(uint8 consider_level, bool ignore_modifiers) {
	Lua_Safe_Call_Int();
	return self->CalcEXP(consider_level, ignore_modifiers);
}

bool Lua_Client::CanEnterZone(std::string zone_short_name) {
	Lua_Safe_Call_Bool();
	return self->CanEnterZone(zone_short_name);
}

bool Lua_Client::CanEnterZone(std::string zone_short_name, int16 instance_version) {
	Lua_Safe_Call_Bool();
	return self->CanEnterZone(zone_short_name, instance_version);
}

void Lua_Client::SendPath(Lua_Mob target)
{
	Lua_Safe_Call_Void();
	self->SendPath(target);
}

int Lua_Client::GetBotRequiredLevel()
{
	Lua_Safe_Call_Int();
	return self->GetBotRequiredLevel();
}

int Lua_Client::GetBotRequiredLevel(uint8 class_id)
{
	Lua_Safe_Call_Int();
	return self->GetBotRequiredLevel(class_id);
}

uint32 Lua_Client::GetBotCreationLimit()
{
	Lua_Safe_Call_Int();
	return self->GetBotCreationLimit();
}

uint32 Lua_Client::GetBotCreationLimit(uint8 class_id)
{
	Lua_Safe_Call_Int();
	return self->GetBotCreationLimit(class_id);
}

int Lua_Client::GetBotSpawnLimit()
{
	Lua_Safe_Call_Int();
	return self->GetBotSpawnLimit();
}

int Lua_Client::GetBotSpawnLimit(uint8 class_id)
{
	Lua_Safe_Call_Int();
	return self->GetBotSpawnLimit(class_id);
}

void Lua_Client::SetBotRequiredLevel(int new_required_level)
{
	Lua_Safe_Call_Void();
	self->SetBotRequiredLevel(new_required_level);
}

void Lua_Client::SetBotRequiredLevel(int new_required_level, uint8 class_id)
{
	Lua_Safe_Call_Void();
	self->SetBotRequiredLevel(new_required_level, class_id);
}

void Lua_Client::SetBotCreationLimit(uint32 new_creation_limit)
{
	Lua_Safe_Call_Void();
	self->SetBotCreationLimit(new_creation_limit);
}

void Lua_Client::SetBotCreationLimit(uint32 new_creation_limit, uint8 class_id)
{
	Lua_Safe_Call_Void();
	self->SetBotCreationLimit(new_creation_limit, class_id);
}

void Lua_Client::SetBotSpawnLimit(int new_spawn_limit)
{
	Lua_Safe_Call_Void();
	self->SetBotSpawnLimit(new_spawn_limit);
}

void Lua_Client::SetBotSpawnLimit(int new_spawn_limit, uint8 class_id)
{
	Lua_Safe_Call_Void();
	self->SetBotSpawnLimit(new_spawn_limit, class_id);
}

void Lua_Client::CampAllBots()
{
	Lua_Safe_Call_Void();
	self->CampAllBots();
}

void Lua_Client::CampAllBots(uint8 class_id)
{
	Lua_Safe_Call_Void();
	self->CampAllBots(class_id);
}

void Lua_Client::ResetItemCooldown(uint32 item_id)
{
	Lua_Safe_Call_Void();
	self->ResetItemCooldown(item_id);
}

void Lua_Client::SetItemCooldown(uint32 item_id, uint32 in_time)
{
	Lua_Safe_Call_Void();
	self->SetItemCooldown(item_id, false, in_time);
}

uint32 Lua_Client::GetItemCooldown(uint32 item_id)
{
	Lua_Safe_Call_Int();
	return self->GetItemCooldown(item_id);
}

void Lua_Client::UseAugmentContainer(int container_slot)
{
	Lua_Safe_Call_Void();
	self->UseAugmentContainer(container_slot);
}


bool Lua_Client::IsAutoAttackEnabled()
{
	Lua_Safe_Call_Bool();
	return self->AutoAttackEnabled();
}

bool Lua_Client::IsAutoFireEnabled()
{
	Lua_Safe_Call_Bool();
	return self->AutoFireEnabled();
}

bool Lua_Client::ReloadDataBuckets() {
	Lua_Safe_Call_Bool();
	return self->LoadDataBucketsCache();
}

uint32 Lua_Client::GetEXPForLevel(uint16 check_level)
{
	Lua_Safe_Call_Int();
	return self->GetEXPForLevel(check_level);
}

std::string Lua_Client::GetClassAbbreviation()
{
	Lua_Safe_Call_String();
	return GetPlayerClassAbbreviation(self->GetBaseClass());
}

std::string Lua_Client::GetRaceAbbreviation()
{
	Lua_Safe_Call_String();
	return GetPlayerRaceAbbreviation(self->GetBaseRace());
}

void Lua_Client::SetLDoNPoints(uint32 theme_id, uint32 points)
{
	Lua_Safe_Call_Void();
	self->SetLDoNPoints(theme_id, points);
}

void Lua_Client::DeleteBucket(std::string bucket_name)
{
	Lua_Safe_Call_Void();
	self->DeleteBucket(bucket_name);
}

std::string Lua_Client::GetBucket(std::string bucket_name)
{
	Lua_Safe_Call_String();
	return self->GetBucket(bucket_name);
}

std::string Lua_Client::GetBucketExpires(std::string bucket_name)
{
	Lua_Safe_Call_String();
	return self->GetBucketExpires(bucket_name);
}

std::string Lua_Client::GetBucketRemaining(std::string bucket_name)
{
	Lua_Safe_Call_String();
	return self->GetBucketRemaining(bucket_name);
}

void Lua_Client::SetBucket(std::string bucket_name, std::string bucket_value)
{
	Lua_Safe_Call_Void();
	self->SetBucket(bucket_name, bucket_value);
}

void Lua_Client::SetBucket(std::string bucket_name, std::string bucket_value, std::string expiration)
{
	Lua_Safe_Call_Void();
	self->SetBucket(bucket_name, bucket_value, expiration);
}

void Lua_Client::GrantAllAAPoints()
{
	Lua_Safe_Call_Void();
	self->GrantAllAAPoints();
}

void Lua_Client::GrantAllAAPoints(uint8 unlock_level)
{
	Lua_Safe_Call_Void();
	self->GrantAllAAPoints(unlock_level);
}

void Lua_Client::GrantAllAAPoints(uint8 unlock_level, bool skip_grant_only)
{
	Lua_Safe_Call_Void();
	self->GrantAllAAPoints(unlock_level, skip_grant_only);
}

void Lua_Client::AddEbonCrystals(uint32 amount)
{
	Lua_Safe_Call_Void();
	self->AddEbonCrystals(amount);
}

void Lua_Client::AddRadiantCrystals(uint32 amount)
{
	Lua_Safe_Call_Void();
	self->AddRadiantCrystals(amount);
}

void Lua_Client::RemoveEbonCrystals(uint32 amount)
{
	Lua_Safe_Call_Void();
	self->RemoveEbonCrystals(amount);
}

void Lua_Client::RemoveRadiantCrystals(uint32 amount)
{
	Lua_Safe_Call_Void();
	self->RemoveRadiantCrystals(amount);
}

void Lua_Client::SummonItemIntoInventory(luabind::object item_table) {
	Lua_Safe_Call_Void();
	if (luabind::type(item_table) != LUA_TTABLE) {
		return;
	}

	const uint32 item_id       = luabind::object_cast<uint32>(item_table["item_id"]);
	const int16 charges        = luabind::object_cast<uint32>(item_table["charges"]);
	const uint32 augment_one   = luabind::type(item_table["augment_one"]) != LUA_TNIL ? luabind::object_cast<uint32>(item_table["augment_one"]) : 0;
	const uint32 augment_two   = luabind::type(item_table["augment_two"]) != LUA_TNIL ? luabind::object_cast<uint32>(item_table["augment_two"]) : 0;
	const uint32 augment_three = luabind::type(item_table["augment_three"]) != LUA_TNIL ? luabind::object_cast<uint32>(item_table["augment_three"]) : 0;
	const uint32 augment_four  = luabind::type(item_table["augment_four"]) != LUA_TNIL ? luabind::object_cast<uint32>(item_table["augment_four"]) : 0;
	const uint32 augment_five  = luabind::type(item_table["augment_five"]) != LUA_TNIL ? luabind::object_cast<uint32>(item_table["augment_five"]) : 0;
	const uint32 augment_six   = luabind::type(item_table["augment_six"]) != LUA_TNIL ? luabind::object_cast<uint32>(item_table["augment_six"]) : 0;
	const bool attuned         = luabind::type(item_table["attuned"]) != LUA_TNIL ? luabind::object_cast<bool>(item_table["attuned"]) : false;

	self->SummonItemIntoInventory(
		item_id,
		charges,
		augment_one,
		augment_two,
		augment_three,
		augment_four,
		augment_five,
		augment_six,
		attuned
	);
}

bool Lua_Client::HasItemOnCorpse(uint32 item_id)
{
	Lua_Safe_Call_Bool();
	return self->HasItemOnCorpse(item_id);
}

void Lua_Client::ClearXTargets()
{
	Lua_Safe_Call_Void();
	self->ClearXTargets();
}

int Lua_Client::GetAAEXPPercentage()
{
	Lua_Safe_Call_Int();
	return self->GetAAEXPPercentage();
}

int Lua_Client::GetEXPPercentage()
{
	Lua_Safe_Call_Int();
	return self->GetEXPPercentage();
}

bool Lua_Client::IsInAGuild()
{
	Lua_Safe_Call_Bool();
	return self->IsInAGuild();
}

bool Lua_Client::RemoveAAPoints(uint32 points)
{
	Lua_Safe_Call_Bool();
	return self->RemoveAAPoints(points);
}

bool Lua_Client::RemoveAlternateCurrencyValue(uint32 currency_id, uint32 amount)
{
	Lua_Safe_Call_Bool();
	return self->RemoveAlternateCurrencyValue(currency_id, amount);
}

luabind::object Lua_Client::GetRaidOrGroupOrSelf(lua_State* L)
{
	auto t = luabind::newtable(L);
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto l = self->GetRaidOrGroupOrSelf();
		int i = 1;
		for (const auto& e : l) {
			t[i] = Lua_Mob(e);
			i++;
		}
	}

	return t;
}

luabind::object Lua_Client::GetRaidOrGroupOrSelf(lua_State* L, bool clients_only)
{
	auto t = luabind::newtable(L);
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto l = self->GetRaidOrGroupOrSelf(clients_only);
		int i = 1;
		for (const auto& e : l) {
			t[i] = Lua_Mob(e);
			i++;
		}
	}

	return t;
}

std::string Lua_Client::GetAutoLoginCharacterName()
{
	Lua_Safe_Call_String();
	return quest_manager.GetAutoLoginCharacterNameByAccountID(self->AccountID());
}

bool Lua_Client::SetAutoLoginCharacterName()
{
	Lua_Safe_Call_Bool();
	return quest_manager.SetAutoLoginCharacterNameByAccountID(self->AccountID(), self->GetCleanName());
}

bool Lua_Client::SetAutoLoginCharacterName(std::string character_name)
{
	Lua_Safe_Call_Bool();
	return quest_manager.SetAutoLoginCharacterNameByAccountID(self->AccountID(), character_name);
}

void Lua_Client::DescribeSpecialAbilities(Lua_NPC n)
{
	Lua_Safe_Call_Void();
	n.DescribeSpecialAbilities(self);
}

void Lua_Client::ResetLeadershipAA()
{
	Lua_Safe_Call_Void();
	self->ResetLeadershipAA();
}

uint8 Lua_Client::GetSkillTrainLevel(int skill_id)
{
	Lua_Safe_Call_Int();
	return self->GetSkillTrainLevel(static_cast<EQ::skills::SkillType>(skill_id), self->GetClass());
}

bool Lua_Client::AreTasksCompleted(luabind::object task_ids)
{
	Lua_Safe_Call_Int();

	if (luabind::type(task_ids) != LUA_TTABLE) {
		return false;
	}

	std::vector<int> v;
	int index = 1;
	while (luabind::type(task_ids[index]) != LUA_TNIL) {
		auto current_id = task_ids[index];
		int task_id = 0;
		if (luabind::type(current_id) != LUA_TNIL) {
			try {
				task_id = luabind::object_cast<int>(current_id);
			} catch(luabind::cast_failed &) {
			}
		} else {
			break;
		}

		v.push_back(task_id);
		++index;
	}

	if (v.empty()) {
		return false;
	}

	return self->AreTasksCompleted(v);
}

void Lua_Client::AreaTaunt()
{
	Lua_Safe_Call_Void();
	entity_list.AETaunt(self);
}

void Lua_Client::AreaTaunt(float range)
{
	Lua_Safe_Call_Void();
	entity_list.AETaunt(self, range);
}

void Lua_Client::AreaTaunt(float range, int bonus_hate)
{
	Lua_Safe_Call_Void();
	entity_list.AETaunt(self, range, bonus_hate);
}

luabind::object Lua_Client::GetInventorySlots(lua_State* L)
{
	auto lua_table = luabind::newtable(L);

	if (d_) {
		auto self  = reinterpret_cast<NativeType *>(d_);
		int  index = 1;
		for (const int16 &slot_id: self->GetInventorySlots()) {
			lua_table[index] = slot_id;
			index++;
		}
	}

	return lua_table;
}

void Lua_Client::ShowZoneShardMenu()
{
	Lua_Safe_Call_Void();
	self->ShowZoneShardMenu();
}

void Lua_Client::GrantPetNameChange()
{
	Lua_Safe_Call_Void();
	self->GrantPetNameChange();
}

void Lua_Client::SetAAEXPPercentage(uint8 percentage)
{
	Lua_Safe_Call_Void();
	self->SetAAEXPPercentage(percentage);
}

void Lua_Client::SetAccountBucket(std::string bucket_name, std::string bucket_value)
{
	Lua_Safe_Call_Void();
	self->SetAccountBucket(bucket_name, bucket_value);
}

void Lua_Client::SetAccountBucket(std::string bucket_name, std::string bucket_value, std::string expiration)
{
	Lua_Safe_Call_Void();
	self->SetAccountBucket(bucket_name, bucket_value, expiration);
}

void Lua_Client::DeleteAccountBucket(std::string bucket_name)
{
	Lua_Safe_Call_Void();
	self->DeleteAccountBucket(bucket_name);
}

std::string Lua_Client::GetAccountBucket(std::string bucket_name)
{
	Lua_Safe_Call_String();
	return self->GetAccountBucket(bucket_name);
}

std::string Lua_Client::GetAccountBucketExpires(std::string bucket_name)
{
	Lua_Safe_Call_String();
	return self->GetAccountBucketExpires(bucket_name);
}

std::string Lua_Client::GetAccountBucketRemaining(std::string bucket_name)
{
	Lua_Safe_Call_String();
	return self->GetAccountBucketRemaining(bucket_name);
}

void Lua_Client::GrantNameChange()
{
	Lua_Safe_Call_Void();
	self->GrantNameChange();
}

bool Lua_Client::IsNameChangeAllowed()
{
	Lua_Safe_Call_Bool();
	return self->IsNameChangeAllowed();
}

bool Lua_Client::ClearNameChange()
{
	Lua_Safe_Call_Bool();
	return self->ClearNameChange();
}

std::string Lua_Client::GetBandolierName(uint8 bandolier_slot)
{
	Lua_Safe_Call_String();
	return self->GetBandolierName(bandolier_slot);
}

uint32 Lua_Client::GetBandolierItemIcon(uint8 bandolier_slot, uint8 slot_id)
{
	Lua_Safe_Call_Int();
	return self->GetBandolierItemIcon(bandolier_slot, slot_id);
}

uint32 Lua_Client::GetBandolierItemID(uint8 bandolier_slot, uint8 slot_id)
{
	Lua_Safe_Call_Int();
	return self->GetBandolierItemID(bandolier_slot, slot_id);
}

std::string Lua_Client::GetBandolierItemName(uint8 bandolier_slot, uint8 slot_id)
{
	Lua_Safe_Call_String();
	return self->GetBandolierItemName(bandolier_slot, slot_id);
}

uint32 Lua_Client::GetPotionBeltItemIcon(uint8 slot_id)
{
	Lua_Safe_Call_Int();
	return self->GetPotionBeltItemIcon(slot_id);
}

uint32 Lua_Client::GetPotionBeltItemID(uint8 slot_id)
{
	Lua_Safe_Call_Int();
	return self->GetPotionBeltItemID(slot_id);
}

std::string Lua_Client::GetPotionBeltItemName(uint8 slot_id)
{
	Lua_Safe_Call_String();
	return self->GetPotionBeltItemName(slot_id);
}

bool Lua_Client::KeyRingAdd(uint32 item) {
	Lua_Safe_Call_Bool();
	return self->KeyRingAdd(item);
}

bool Lua_Client::KeyRingCheck(uint32 item) {
	Lua_Safe_Call_Bool();
	return self->KeyRingCheck(item);
}

bool Lua_Client::KeyRingClear()
{
	Lua_Safe_Call_Bool();
	return self->KeyRingClear();
}

void Lua_Client::KeyRingList()
{
	Lua_Safe_Call_Void();
	self->KeyRingList(self);
}

void Lua_Client::KeyRingList(Lua_Client c)
{
	Lua_Safe_Call_Void();
	self->KeyRingList(self);
}

bool Lua_Client::KeyRingRemove(uint32 item_id)
{
	Lua_Safe_Call_Bool();
	return self->KeyRingRemove(item_id);
}


bool Lua_Client::CompleteTask(int task_id)
{
	Lua_Safe_Call_Bool();
	return self->CompleteTask(task_id);
}

bool Lua_Client::UncompleteTask(int task_id)
{
	Lua_Safe_Call_Bool();
	return self->UncompleteTask(task_id);
}

void Lua_Client::EnableTitleSet(uint32 title_set) {
	Lua_Safe_Call_Void();
	self->EnableTitle(title_set);
}

luabind::object Lua_Client::GetKeyRing(lua_State* L)
{
	auto lua_table = luabind::newtable(L);

	if (d_) {
		auto self  = reinterpret_cast<NativeType *>(d_);
		int  index = 1;
		for (const uint32& item_id: self->GetKeyRing()) {
			lua_table[index] = item_id;
			index++;
		}
	}

	return lua_table;
}

luabind::scope lua_register_client() {
	return luabind::class_<Lua_Client, Lua_Mob>("Client")
	.def(luabind::constructor<>())
	.def("AccountID", (uint32(Lua_Client::*)(void))&Lua_Client::AccountID)
	.def("AccountName", (const char *(Lua_Client::*)(void))&Lua_Client::AccountName)
	.def("AddAAPoints", (void(Lua_Client::*)(int))&Lua_Client::AddAAPoints)
	.def("AddAlternateCurrencyValue", (void(Lua_Client::*)(uint32,int))&Lua_Client::AddAlternateCurrencyValue)
	.def("AddCrystals", (void(Lua_Client::*)(uint32,uint32))&Lua_Client::AddCrystals)
	.def("AddEXP", (void(Lua_Client::*)(uint32))&Lua_Client::AddEXP)
	.def("AddEXP", (void(Lua_Client::*)(uint32,int))&Lua_Client::AddEXP)
	.def("AddEXP", (void(Lua_Client::*)(uint32,int,bool))&Lua_Client::AddEXP)
	.def("AddEbonCrystals", (void(Lua_Client::*)(uint32))&Lua_Client::AddEbonCrystals)
	.def("AddExpeditionLockout", (void(Lua_Client::*)(std::string, std::string, uint32))&Lua_Client::AddExpeditionLockout)
	.def("AddExpeditionLockout", (void(Lua_Client::*)(std::string, std::string, uint32, std::string))&Lua_Client::AddExpeditionLockout)
	.def("AddExpeditionLockoutDuration", (void(Lua_Client::*)(std::string, std::string, int))&Lua_Client::AddExpeditionLockoutDuration)
	.def("AddExpeditionLockoutDuration", (void(Lua_Client::*)(std::string, std::string, int, std::string))&Lua_Client::AddExpeditionLockoutDuration)
	.def("AddItem", (void(Lua_Client::*)(luabind::adl::object))&Lua_Client::AddItem)
	.def("AddLDoNLoss", (void(Lua_Client::*)(uint32))&Lua_Client::AddLDoNLoss)
	.def("AddLDoNWin", (void(Lua_Client::*)(uint32))&Lua_Client::AddLDoNWin)
	.def("AddLevelBasedExp", (void(Lua_Client::*)(int))&Lua_Client::AddLevelBasedExp)
	.def("AddLevelBasedExp", (void(Lua_Client::*)(int,int))&Lua_Client::AddLevelBasedExp)
	.def("AddLevelBasedExp", (void(Lua_Client::*)(int,int,bool))&Lua_Client::AddLevelBasedExp)
	.def("AddMoneyToPP", (void(Lua_Client::*)(uint32,uint32,uint32,uint32))&Lua_Client::AddMoneyToPP)
	.def("AddMoneyToPP", (void(Lua_Client::*)(uint32,uint32,uint32,uint32,bool))&Lua_Client::AddMoneyToPP)
	.def("AddPlatinum", (void(Lua_Client::*)(uint32))&Lua_Client::AddPlatinum)
	.def("AddPlatinum", (void(Lua_Client::*)(uint32,bool))&Lua_Client::AddPlatinum)
	.def("AddPVPPoints", (void(Lua_Client::*)(uint32))&Lua_Client::AddPVPPoints)
	.def("AddRadiantCrystals", (void(Lua_Client::*)(uint32))&Lua_Client::AddRadiantCrystals)
	.def("AddSkill", (void(Lua_Client::*)(int,int))&Lua_Client::AddSkill)
	.def("Admin", (int16(Lua_Client::*)(void))&Lua_Client::Admin)
	.def("ApplySpell", (void(Lua_Client::*)(int))&Lua_Client::ApplySpell)
	.def("ApplySpell", (void(Lua_Client::*)(int,int))&Lua_Client::ApplySpell)
	.def("ApplySpell", (void(Lua_Client::*)(int,int,int))&Lua_Client::ApplySpell)
	.def("ApplySpell", (void(Lua_Client::*)(int,int,int,bool))&Lua_Client::ApplySpell)
	.def("ApplySpell", (void(Lua_Client::*)(int,int,int,bool,bool))&Lua_Client::ApplySpell)
	.def("ApplySpellGroup", (void(Lua_Client::*)(int))&Lua_Client::ApplySpellGroup)
	.def("ApplySpellGroup", (void(Lua_Client::*)(int,int))&Lua_Client::ApplySpellGroup)
	.def("ApplySpellGroup", (void(Lua_Client::*)(int,int,int))&Lua_Client::ApplySpellGroup)
	.def("ApplySpellGroup", (void(Lua_Client::*)(int,int,int,bool))&Lua_Client::ApplySpellGroup)
	.def("ApplySpellGroup", (void(Lua_Client::*)(int,int,int,bool,bool))&Lua_Client::ApplySpellGroup)
	.def("ApplySpellRaid", (void(Lua_Client::*)(int))&Lua_Client::ApplySpellRaid)
	.def("ApplySpellRaid", (void(Lua_Client::*)(int,int))&Lua_Client::ApplySpellRaid)
	.def("ApplySpellRaid", (void(Lua_Client::*)(int,int,int))&Lua_Client::ApplySpellRaid)
	.def("ApplySpellRaid", (void(Lua_Client::*)(int,int,int,bool))&Lua_Client::ApplySpellRaid)
	.def("ApplySpellRaid", (void(Lua_Client::*)(int,int,int,bool,bool))&Lua_Client::ApplySpellRaid)
	.def("ApplySpellRaid", (void(Lua_Client::*)(int,int,int,bool,bool,bool))&Lua_Client::ApplySpellRaid)
	.def("AreTasksCompleted", (bool(Lua_Client::*)(luabind::object))&Lua_Client::AreTasksCompleted)
	.def("AreaTaunt", (void(Lua_Client::*)(void))&Lua_Client::AreaTaunt)
	.def("AreaTaunt", (void(Lua_Client::*)(float))&Lua_Client::AreaTaunt)
	.def("AreaTaunt", (void(Lua_Client::*)(float, int))&Lua_Client::AreaTaunt)
	.def("AssignTask", (void(Lua_Client::*)(int))&Lua_Client::AssignTask)
	.def("AssignTask", (void(Lua_Client::*)(int,int))&Lua_Client::AssignTask)
	.def("AssignTask", (void(Lua_Client::*)(int,int,bool))&Lua_Client::AssignTask)
	.def("AssignToInstance", (void(Lua_Client::*)(int))&Lua_Client::AssignToInstance)
	.def("AutoSplitEnabled", (bool(Lua_Client::*)(void))&Lua_Client::AutoSplitEnabled)
	.def("BreakInvis", (void(Lua_Client::*)(void))&Lua_Client::BreakInvis)
	.def("CalcATK", &Lua_Client::CalcATK)
	.def("CalcCurrentWeight", &Lua_Client::CalcCurrentWeight)
	.def("CalcEXP", (uint64(Lua_Client::*)(uint8))&Lua_Client::CalcEXP)
	.def("CalcEXP", (uint64(Lua_Client::*)(uint8,bool))&Lua_Client::CalcEXP)
	.def("CalcPriceMod", (float(Lua_Client::*)(Lua_Mob,bool))&Lua_Client::CalcPriceMod)
	.def("CampAllBots", (void(Lua_Client::*)(void))&Lua_Client::CampAllBots)
	.def("CampAllBots", (void(Lua_Client::*)(uint8))&Lua_Client::CampAllBots)
	.def("CanEnterZone", (bool(Lua_Client::*)(std::string))&Lua_Client::CanEnterZone)
	.def("CanEnterZone", (bool(Lua_Client::*)(std::string,int16))&Lua_Client::CanEnterZone)
	.def("CanHaveSkill", (bool(Lua_Client::*)(int))&Lua_Client::CanHaveSkill)
	.def("CashReward", &Lua_Client::CashReward)
	.def("ChangeLastName", (void(Lua_Client::*)(std::string))&Lua_Client::ChangeLastName)
	.def("GrantPetNameChange", &Lua_Client::GrantPetNameChange)
	.def("ClearNameChange", &Lua_Client::ClearNameChange)
	.def("CharacterID", (uint32(Lua_Client::*)(void))&Lua_Client::CharacterID)
	.def("CheckIncreaseSkill", (void(Lua_Client::*)(int,Lua_Mob))&Lua_Client::CheckIncreaseSkill)
	.def("CheckIncreaseSkill", (void(Lua_Client::*)(int,Lua_Mob,int))&Lua_Client::CheckIncreaseSkill)
	.def("CheckSpecializeIncrease", (void(Lua_Client::*)(int))&Lua_Client::CheckSpecializeIncrease)
	.def("ClearCompassMark",(void(Lua_Client::*)(void))&Lua_Client::ClearCompassMark)
	.def("ClearAccountFlag", (void(Lua_Client::*)(const std::string&))&Lua_Client::ClearAccountFlag)
	.def("ClearPEQZoneFlag", (void(Lua_Client::*)(uint32))&Lua_Client::ClearPEQZoneFlag)
	.def("ClearXTargets", (void(Lua_Client::*)(void))&Lua_Client::ClearXTargets)
	.def("ClearZoneFlag", (void(Lua_Client::*)(uint32))&Lua_Client::ClearZoneFlag)
	.def("CompleteTask", (bool(Lua_Client::*)(int))&Lua_Client::CompleteTask)
	.def("Connected", (bool(Lua_Client::*)(void))&Lua_Client::Connected)
	.def("CountAugmentEquippedByID", (uint32(Lua_Client::*)(uint32))&Lua_Client::CountAugmentEquippedByID)
	.def("CountItem", (uint32(Lua_Client::*)(uint32))&Lua_Client::CountItem)
	.def("CountItemEquippedByID", (uint32(Lua_Client::*)(uint32))&Lua_Client::CountItemEquippedByID)
	.def("CreateExpedition", (Lua_Expedition(Lua_Client::*)(luabind::object))&Lua_Client::CreateExpedition)
	.def("CreateExpedition", (Lua_Expedition(Lua_Client::*)(std::string, uint32, uint32, std::string, uint32, uint32))&Lua_Client::CreateExpedition)
	.def("CreateExpedition", (Lua_Expedition(Lua_Client::*)(std::string, uint32, uint32, std::string, uint32, uint32, bool))&Lua_Client::CreateExpedition)
	.def("CreateExpeditionFromTemplate", &Lua_Client::CreateExpeditionFromTemplate)
	.def("CreateTaskDynamicZone", &Lua_Client::CreateTaskDynamicZone)
	.def("DecreaseByID", (bool(Lua_Client::*)(uint32,int))&Lua_Client::DecreaseByID)
	.def("DescribeSpecialAbilities", (void(Lua_Client::*)(Lua_NPC))&Lua_Client::DescribeSpecialAbilities)
	.def("DeleteAccountBucket", (void(Lua_Client::*)(std::string))&Lua_Client::DeleteAccountBucket)
	.def("DeleteBucket", (void(Lua_Client::*)(std::string))&Lua_Client::DeleteBucket)
	.def("DeleteItemInInventory", (void(Lua_Client::*)(int,int))&Lua_Client::DeleteItemInInventory)
	.def("DeleteItemInInventory", (void(Lua_Client::*)(int,int,bool))&Lua_Client::DeleteItemInInventory)
	.def("DiaWind", (void(Lua_Client::*)(std::string))&Lua_Client::DialogueWindow)
	.def("DialogueWindow", (void(Lua_Client::*)(std::string))&Lua_Client::DialogueWindow)
	.def("DisableAreaEndRegen", &Lua_Client::DisableAreaEndRegen)
	.def("DisableAreaHPRegen", &Lua_Client::DisableAreaHPRegen)
	.def("DisableAreaManaRegen", &Lua_Client::DisableAreaManaRegen)
	.def("DisableAreaRegens", &Lua_Client::DisableAreaRegens)
	.def("Disconnect", (void(Lua_Client::*)(void))&Lua_Client::Disconnect)
	.def("DropItem", (void(Lua_Client::*)(int))&Lua_Client::DropItem)
	.def("Duck", (void(Lua_Client::*)(void))&Lua_Client::Duck)
	.def("DyeArmorBySlot", (void(Lua_Client::*)(uint8,uint8,uint8,uint8))&Lua_Client::DyeArmorBySlot)
	.def("DyeArmorBySlot", (void(Lua_Client::*)(uint8,uint8,uint8,uint8,uint8))&Lua_Client::DyeArmorBySlot)
	.def("EnableAreaEndRegen", &Lua_Client::EnableAreaEndRegen)
	.def("EnableAreaHPRegen", &Lua_Client::EnableAreaHPRegen)
	.def("EnableAreaManaRegen", &Lua_Client::EnableAreaManaRegen)
	.def("EnableAreaRegens", &Lua_Client::EnableAreaRegens)
	.def("EnableTitleSet", &Lua_Client::EnableTitleSet)
	.def("EndSharedTask", (void(Lua_Client::*)(void))&Lua_Client::EndSharedTask)
	.def("EndSharedTask", (void(Lua_Client::*)(bool))&Lua_Client::EndSharedTask)
	.def("Escape", (void(Lua_Client::*)(void))&Lua_Client::Escape)
	.def("FailTask", (void(Lua_Client::*)(int))&Lua_Client::FailTask)
	.def("FilteredMessage", &Lua_Client::FilteredMessage)
	.def("FindEmptyMemSlot", (int(Lua_Client::*)(void))&Lua_Client::FindEmptyMemSlot)
	.def("FindMemmedSpellBySlot", (uint16(Lua_Client::*)(int))&Lua_Client::FindMemmedSpellBySlot)
	.def("FindMemmedSpellBySpellID", (int(Lua_Client::*)(uint16))&Lua_Client::FindMemmedSpellBySpellID)
	.def("FindSpellBookSlotBySpellID", (int(Lua_Client::*)(int))&Lua_Client::FindSpellBookSlotBySpellID)
	.def("Fling", (void(Lua_Client::*)(float,float,float))&Lua_Client::Fling)
	.def("Fling", (void(Lua_Client::*)(float,float,float,bool))&Lua_Client::Fling)
	.def("Fling", (void(Lua_Client::*)(float,float,float,bool,bool))&Lua_Client::Fling)
	.def("Fling", (void(Lua_Client::*)(float,float,float,float))&Lua_Client::Fling)
	.def("Fling", (void(Lua_Client::*)(float,float,float,float,bool))&Lua_Client::Fling)
	.def("Fling", (void(Lua_Client::*)(float,float,float,float,bool,bool))&Lua_Client::Fling)
	.def("ForageItem", (void(Lua_Client::*)(bool))&Lua_Client::ForageItem)
	.def("ForageItem", (void(Lua_Client::*)(void))&Lua_Client::ForageItem)
	.def("Freeze", (void(Lua_Client::*)(void))&Lua_Client::Freeze)
	.def("GetAAEXPModifier", (float(Lua_Client::*)(void))&Lua_Client::GetAAEXPModifier)
	.def("GetAAEXPModifier", (float(Lua_Client::*)(uint32))&Lua_Client::GetAAEXPModifier)
	.def("GetAAEXPModifier", (float(Lua_Client::*)(uint32,int16))&Lua_Client::GetAAEXPModifier)
	.def("GetAAEXPPercentage", (int(Lua_Client::*)(void))&Lua_Client::GetAAEXPPercentage)
	.def("GetAAExp", (uint32(Lua_Client::*)(void))&Lua_Client::GetAAExp)
	.def("GetAAPercent", (uint32(Lua_Client::*)(void))&Lua_Client::GetAAPercent)
	.def("GetAAPoints", (int(Lua_Client::*)(void))&Lua_Client::GetAAPoints)
	.def("GetAFK", (int(Lua_Client::*)(void))&Lua_Client::GetAFK)
	.def("GetAccountAge", (int(Lua_Client::*)(void))&Lua_Client::GetAccountAge)
	.def("GetAccountFlag", (std::string(Lua_Client::*)(const std::string&))&Lua_Client::GetAccountFlag)
	.def("GetAccountFlags", (luabind::object(Lua_Client::*)(lua_State*))&Lua_Client::GetAccountFlags)
	.def("GetAggroCount", (uint32(Lua_Client::*)(void))&Lua_Client::GetAggroCount)
	.def("GetAllMoney", (uint64(Lua_Client::*)(void))&Lua_Client::GetAllMoney)
	.def("GetAlternateCurrencyValue", (uint32(Lua_Client::*)(uint32))&Lua_Client::GetAlternateCurrencyValue)
	.def("GetAnon", (int(Lua_Client::*)(void))&Lua_Client::GetAnon)
	.def("GetAugmentIDAt", (int(Lua_Client::*)(int,int))&Lua_Client::GetAugmentIDAt)
	.def("GetAugmentIDsBySlotID", (luabind::object(Lua_Client::*)(lua_State* L,int16))&Lua_Client::GetAugmentIDsBySlotID)
	.def("GetAutoLoginCharacterName", (std::string(Lua_Client::*)(void))&Lua_Client::GetAutoLoginCharacterName)
	.def("GetBandolierItemIcon", (uint32(Lua_Client::*)(uint8,uint8))&Lua_Client::GetBandolierItemIcon)
	.def("GetBandolierItemID", (uint32(Lua_Client::*)(uint8,uint8))&Lua_Client::GetBandolierItemID)
	.def("GetBandolierItemName", (std::string(Lua_Client::*)(uint8,uint8))&Lua_Client::GetBandolierItemName)
	.def("GetBandolierName", (std::string(Lua_Client::*)(uint8))&Lua_Client::GetBandolierName)
	.def("GetBaseAGI", (int(Lua_Client::*)(void))&Lua_Client::GetBaseAGI)
	.def("GetBaseCHA", (int(Lua_Client::*)(void))&Lua_Client::GetBaseCHA)
	.def("GetBaseDEX", (int(Lua_Client::*)(void))&Lua_Client::GetBaseDEX)
	.def("GetBaseFace", (int(Lua_Client::*)(void))&Lua_Client::GetBaseFace)
	.def("GetBaseINT", (int(Lua_Client::*)(void))&Lua_Client::GetBaseINT)
	.def("GetBaseSTA", (int(Lua_Client::*)(void))&Lua_Client::GetBaseSTA)
	.def("GetBaseSTR", (int(Lua_Client::*)(void))&Lua_Client::GetBaseSTR)
	.def("GetBaseWIS", (int(Lua_Client::*)(void))&Lua_Client::GetBaseWIS)
	.def("GetBindHeading", (float(Lua_Client::*)(int))&Lua_Client::GetBindHeading)
	.def("GetBindHeading", (float(Lua_Client::*)(void))&Lua_Client::GetBindHeading)
	.def("GetBindX", (float(Lua_Client::*)(int))&Lua_Client::GetBindX)
	.def("GetBindX", (float(Lua_Client::*)(void))&Lua_Client::GetBindX)
	.def("GetBindY", (float(Lua_Client::*)(int))&Lua_Client::GetBindY)
	.def("GetBindY", (float(Lua_Client::*)(void))&Lua_Client::GetBindY)
	.def("GetBindZ", (float(Lua_Client::*)(int))&Lua_Client::GetBindZ)
	.def("GetBindZ", (float(Lua_Client::*)(void))&Lua_Client::GetBindZ)
	.def("GetBindZoneID", (uint32(Lua_Client::*)(int))&Lua_Client::GetBindZoneID)
	.def("GetBindZoneID", (uint32(Lua_Client::*)(void))&Lua_Client::GetBindZoneID)
	.def("GetBotCreationLimit", (uint32(Lua_Client::*)(void))&Lua_Client::GetBotCreationLimit)
	.def("GetBotCreationLimit", (uint32(Lua_Client::*)(uint8))&Lua_Client::GetBotCreationLimit)
	.def("GetBotRequiredLevel", (int(Lua_Client::*)(void))&Lua_Client::GetBotRequiredLevel)
	.def("GetBotRequiredLevel", (int(Lua_Client::*)(uint8))&Lua_Client::GetBotRequiredLevel)
	.def("GetBotSpawnLimit", (int(Lua_Client::*)(void))&Lua_Client::GetBotSpawnLimit)
	.def("GetBotSpawnLimit", (int(Lua_Client::*)(uint8))&Lua_Client::GetBotSpawnLimit)
	.def("GetAccountBucket", (std::string(Lua_Client::*)(std::string))&Lua_Client::GetAccountBucket)
	.def("GetAccountBucketExpires", (std::string(Lua_Client::*)(std::string))&Lua_Client::GetAccountBucketExpires)
	.def("GetAccountBucketRemaining", (std::string(Lua_Client::*)(std::string))&Lua_Client::GetAccountBucketRemaining)
	.def("GetBucket", (std::string(Lua_Client::*)(std::string))&Lua_Client::GetBucket)
	.def("GetBucketExpires", (std::string(Lua_Client::*)(std::string))&Lua_Client::GetBucketExpires)
	.def("GetBucketRemaining", (std::string(Lua_Client::*)(std::string))&Lua_Client::GetBucketRemaining)
	.def("GetCarriedMoney", (uint64(Lua_Client::*)(void))&Lua_Client::GetCarriedMoney)
	.def("GetCarriedPlatinum", (uint32(Lua_Client::*)(void))&Lua_Client::GetCarriedPlatinum)
	.def("GetCharacterFactionLevel", (int(Lua_Client::*)(int))&Lua_Client::GetCharacterFactionLevel)
	.def("GetClassAbbreviation", (std::string(Lua_Client::*)(void))&Lua_Client::GetClassAbbreviation)
	.def("GetClassBitmask", (uint16(Lua_Client::*)(void))&Lua_Client::GetClassBitmask)
	.def("GetClientMaxLevel", (int(Lua_Client::*)(void))&Lua_Client::GetClientMaxLevel)
	.def("GetClientVersion", (int(Lua_Client::*)(void))&Lua_Client::GetClientVersion)
	.def("GetClientVersionBit", (uint32(Lua_Client::*)(void))&Lua_Client::GetClientVersionBit)
	.def("GetCorpseCount", (int64(Lua_Client::*)(void))&Lua_Client::GetCorpseCount)
	.def("GetCorpseID", (int(Lua_Client::*)(int))&Lua_Client::GetCorpseID)
	.def("GetCorpseItemAt", (int(Lua_Client::*)(int,int))&Lua_Client::GetCorpseItemAt)
	.def("GetDeityBitmask", (uint32(Lua_Client::*)(void))&Lua_Client::GetDeityBitmask)
	.def("GetDiscSlotBySpellID", (int(Lua_Client::*)(int32))&Lua_Client::GetDiscSlotBySpellID)
	.def("GetDisciplineTimer", (uint32(Lua_Client::*)(uint32))&Lua_Client::GetDisciplineTimer)
	.def("GetDuelTarget", (int(Lua_Client::*)(void))&Lua_Client::GetDuelTarget)
	.def("GetEXP", (uint32(Lua_Client::*)(void))&Lua_Client::GetEXP)
	.def("GetEXPForLevel", (uint32(Lua_Client::*)(uint16))&Lua_Client::GetEXPForLevel)
	.def("GetEXPModifier", (float(Lua_Client::*)(void))&Lua_Client::GetEXPModifier)
	.def("GetEXPModifier", (float(Lua_Client::*)(uint32))&Lua_Client::GetEXPModifier)
	.def("GetEXPModifier", (float(Lua_Client::*)(uint32,int16))&Lua_Client::GetEXPModifier)
	.def("GetEXPPercentage", (int(Lua_Client::*)(void))&Lua_Client::GetEXPPercentage)
	.def("GetEbonCrystals", (uint32(Lua_Client::*)(void))&Lua_Client::GetEbonCrystals)
	.def("GetEndurance", (int(Lua_Client::*)(void))&Lua_Client::GetEndurance)
	.def("GetEndurancePercent", (int(Lua_Client::*)(void))&Lua_Client::GetEndurancePercent)
	.def("GetEnvironmentDamageModifier", (int(Lua_Client::*)(void))&Lua_Client::GetEnvironmentDamageModifier)
	.def("GetExpedition", (Lua_Expedition(Lua_Client::*)(void))&Lua_Client::GetExpedition)
	.def("GetExpeditionLockouts", (luabind::object(Lua_Client::*)(lua_State* L))&Lua_Client::GetExpeditionLockouts)
	.def("GetExpeditionLockouts", (luabind::object(Lua_Client::*)(lua_State* L, std::string))&Lua_Client::GetExpeditionLockouts)
	.def("GetFace", (int(Lua_Client::*)(void))&Lua_Client::GetFace)
	.def("GetFactionLevel", (int(Lua_Client::*)(uint32,uint32,uint32,uint32,uint32,uint32,Lua_NPC))&Lua_Client::GetFactionLevel)
	.def("GetFeigned", (bool(Lua_Client::*)(void))&Lua_Client::GetFeigned)
	.def("GetGM", (bool(Lua_Client::*)(void))&Lua_Client::GetGM)
	.def("GetGMStatus", (int16(Lua_Client::*)(void))&Lua_Client::GetGMStatus)
	.def("GetGroup", (Lua_Group(Lua_Client::*)(void))&Lua_Client::GetGroup)
	.def("GetGroupPoints", (uint32(Lua_Client::*)(void))&Lua_Client::GetGroupPoints)
	.def("GetGuildPublicNote", (std::string(Lua_Client::*)(void))&Lua_Client::GetGuildPublicNote)
	.def("GetHorseId", (int(Lua_Client::*)(void))&Lua_Client::GetHorseId)
	.def("GetHealAmount", (int(Lua_Client::*)(void))&Lua_Client::GetHealAmount)
	.def("GetHunger", (int(Lua_Client::*)(void))&Lua_Client::GetHunger)
	.def("GetIP", (uint32(Lua_Client::*)(void))&Lua_Client::GetIP)
	.def("GetIPExemption", (int(Lua_Client::*)(void))&Lua_Client::GetIPExemption)
	.def("GetIPString", (std::string(Lua_Client::*)(void))&Lua_Client::GetIPString)
	.def("GetInstrumentMod", (int(Lua_Client::*)(int))&Lua_Client::GetInstrumentMod)
	.def("GetIntoxication", (int(Lua_Client::*)(void))&Lua_Client::GetIntoxication)
	.def("GetInventory", (Lua_Inventory(Lua_Client::*)(void))&Lua_Client::GetInventory)
	.def("GetInventorySlots", (luabind::object(Lua_Client::*)(lua_State* L))&Lua_Client::GetInventorySlots)
	.def("GetInvulnerableEnvironmentDamage", (bool(Lua_Client::*)(void))&Lua_Client::GetInvulnerableEnvironmentDamage)
	.def("GetItemIDAt", (int(Lua_Client::*)(int))&Lua_Client::GetItemIDAt)
	.def("GetItemCooldown", (uint32(Lua_Client::*)(uint32))&Lua_Client::GetItemCooldown)
	.def("GetKeyRing", (luabind::object(Lua_Client::*)(lua_State* L))&Lua_Client::GetKeyRing)
	.def("GetLDoNLosses", (int(Lua_Client::*)(void))&Lua_Client::GetLDoNLosses)
	.def("GetLDoNLossesTheme", (int(Lua_Client::*)(int))&Lua_Client::GetLDoNLossesTheme)
	.def("GetLDoNPointsTheme", (int(Lua_Client::*)(int))&Lua_Client::GetLDoNPointsTheme)
	.def("GetLDoNWins", (int(Lua_Client::*)(void))&Lua_Client::GetLDoNWins)
	.def("GetLDoNWinsTheme", (int(Lua_Client::*)(int))&Lua_Client::GetLDoNWinsTheme)
	.def("GetLanguageSkill", (int(Lua_Client::*)(int))&Lua_Client::GetLanguageSkill)
	.def("GetLearnableDisciplines", (luabind::object(Lua_Client::*)(lua_State* L))&Lua_Client::GetLearnableDisciplines)
	.def("GetLearnableDisciplines", (luabind::object(Lua_Client::*)(lua_State* L,uint8))&Lua_Client::GetLearnableDisciplines)
	.def("GetLearnableDisciplines", (luabind::object(Lua_Client::*)(lua_State* L,uint8,uint8))&Lua_Client::GetLearnableDisciplines)
	.def("GetLearnedDisciplines", (luabind::object(Lua_Client::*)(lua_State* L))&Lua_Client::GetLearnedDisciplines)
	.def("GetLockoutExpeditionUUID", (std::string(Lua_Client::*)(std::string, std::string))&Lua_Client::GetLockoutExpeditionUUID)
	.def("GetMaxEndurance", (int(Lua_Client::*)(void))&Lua_Client::GetMaxEndurance)
	.def("GetMemmedSpells", (luabind::object(Lua_Client::*)(lua_State* L))&Lua_Client::GetMemmedSpells)
	.def("GetModCharacterFactionLevel", (int(Lua_Client::*)(int))&Lua_Client::GetModCharacterFactionLevel)
	.def("GetMoney", (uint32(Lua_Client::*)(uint8, uint8))&Lua_Client::GetMoney)
	.def("GetNextAvailableDisciplineSlot", (int(Lua_Client::*)(int))&Lua_Client::GetNextAvailableDisciplineSlot)
	.def("GetNextAvailableDisciplineSlot", (int(Lua_Client::*)(void))&Lua_Client::GetNextAvailableDisciplineSlot)
	.def("GetNextAvailableSpellBookSlot", (int(Lua_Client::*)(int))&Lua_Client::GetNextAvailableSpellBookSlot)
	.def("GetNextAvailableSpellBookSlot", (int(Lua_Client::*)(void))&Lua_Client::GetNextAvailableSpellBookSlot)
	.def("GetPotionBeltItemIcon", (uint32(Lua_Client::*)(uint8))&Lua_Client::GetPotionBeltItemIcon)
	.def("GetPotionBeltItemID", (uint32(Lua_Client::*)(uint8))&Lua_Client::GetPotionBeltItemID)
	.def("GetPotionBeltItemName", (std::string(Lua_Client::*)(uint8))&Lua_Client::GetPotionBeltItemName)
	.def("GetPVP", (bool(Lua_Client::*)(void))&Lua_Client::GetPVP)
	.def("GetPVPPoints", (uint32(Lua_Client::*)(void))&Lua_Client::GetPVPPoints)
	.def("GetRaceBitmask", (uint16(Lua_Client::*)(void))&Lua_Client::GetRaceBitmask)
	.def("GetRadiantCrystals", (uint32(Lua_Client::*)(void))&Lua_Client::GetRadiantCrystals)
	.def("GetRaid", (Lua_Raid(Lua_Client::*)(void))&Lua_Client::GetRaid)
	.def("GetRaidOrGroupOrSelf", (luabind::object(Lua_Client::*)(lua_State*))&Lua_Client::GetRaidOrGroupOrSelf)
	.def("GetRaidOrGroupOrSelf", (luabind::object(Lua_Client::*)(lua_State*,bool))&Lua_Client::GetRaidOrGroupOrSelf)
	.def("GetRaidPoints", (uint32(Lua_Client::*)(void))&Lua_Client::GetRaidPoints)
	.def("GetRaceAbbreviation", (std::string(Lua_Client::*)(void))&Lua_Client::GetRaceAbbreviation)
	.def("GetRawItemAC", (int(Lua_Client::*)(void))&Lua_Client::GetRawItemAC)
	.def("GetRawSkill", (int(Lua_Client::*)(int))&Lua_Client::GetRawSkill)
	.def("GetRecipeMadeCount", (int(Lua_Client::*)(uint32))&Lua_Client::GetRecipeMadeCount)
	.def("GetScribeableSpells", (luabind::object(Lua_Client::*)(lua_State* L))&Lua_Client::GetScribeableSpells)
	.def("GetScribeableSpells", (luabind::object(Lua_Client::*)(lua_State* L,uint8))&Lua_Client::GetScribeableSpells)
	.def("GetScribeableSpells", (luabind::object(Lua_Client::*)(lua_State* L,uint8,uint8))&Lua_Client::GetScribeableSpells)
	.def("GetScribedSpells", (luabind::object(Lua_Client::*)(lua_State* L))&Lua_Client::GetScribedSpells)
	.def("GetSkillPoints", (int(Lua_Client::*)(void))&Lua_Client::GetSkillPoints)
	.def("GetSkillTrainLevel", (uint8(Lua_Client::*)(int))&Lua_Client::GetSkillTrainLevel)
	.def("GetSpellDamage", (int(Lua_Client::*)(void))&Lua_Client::GetSpellDamage)
	.def("GetSpellIDByBookSlot", (uint32(Lua_Client::*)(int))&Lua_Client::GetSpellIDByBookSlot)
	.def("GetSpentAA", (int(Lua_Client::*)(void))&Lua_Client::GetSpentAA)
	.def("GetStartZone", (int(Lua_Client::*)(void))&Lua_Client::GetStartZone)
	.def("GetTargetRingX", (float(Lua_Client::*)(void))&Lua_Client::GetTargetRingX)
	.def("GetTargetRingY", (float(Lua_Client::*)(void))&Lua_Client::GetTargetRingY)
	.def("GetTargetRingZ", (float(Lua_Client::*)(void))&Lua_Client::GetTargetRingZ)
	.def("GetThirst", (int(Lua_Client::*)(void))&Lua_Client::GetThirst)
	.def("GetTotalSecondsPlayed", (uint32(Lua_Client::*)(void))&Lua_Client::GetTotalSecondsPlayed)
	.def("GetWeight", (int(Lua_Client::*)(void))&Lua_Client::GetWeight)
	.def("GetPEQZoneFlags", (luabind::object(Lua_Client::*)(lua_State*))&Lua_Client::GetPEQZoneFlags)
	.def("GetZoneFlags", (luabind::object(Lua_Client::*)(lua_State*))&Lua_Client::GetZoneFlags)
	.def("GoFish", (void(Lua_Client::*)(void))&Lua_Client::GoFish)
	.def("GrantAllAAPoints", (void(Lua_Client::*)(void))&Lua_Client::GrantAllAAPoints)
	.def("GrantAllAAPoints", (void(Lua_Client::*)(uint8))&Lua_Client::GrantAllAAPoints)
	.def("GrantAllAAPoints", (void(Lua_Client::*)(uint8,bool))&Lua_Client::GrantAllAAPoints)
	.def("GrantAlternateAdvancementAbility", (bool(Lua_Client::*)(int, int))&Lua_Client::GrantAlternateAdvancementAbility)
	.def("GrantAlternateAdvancementAbility", (bool(Lua_Client::*)(int, int, bool))&Lua_Client::GrantAlternateAdvancementAbility)
	.def("GrantNameChange", &Lua_Client::GrantNameChange)
	.def("GuildID", (uint32(Lua_Client::*)(void))&Lua_Client::GuildID)
	.def("GuildRank", (int(Lua_Client::*)(void))&Lua_Client::GuildRank)
	.def("HasAugmentEquippedByID", (bool(Lua_Client::*)(uint32))&Lua_Client::HasAugmentEquippedByID)
	.def("HasDisciplineLearned", (bool(Lua_Client::*)(uint16))&Lua_Client::HasDisciplineLearned)
	.def("HasExpeditionLockout", (bool(Lua_Client::*)(std::string, std::string))&Lua_Client::HasExpeditionLockout)
	.def("HasItemEquippedByID", (bool(Lua_Client::*)(uint32))&Lua_Client::HasItemEquippedByID)
	.def("HasItemOnCorpse", (bool(Lua_Client::*)(uint32))&Lua_Client::HasItemOnCorpse)
	.def("HasPEQZoneFlag", (bool(Lua_Client::*)(uint32))&Lua_Client::HasPEQZoneFlag)
	.def("HasRecipeLearned", (bool(Lua_Client::*)(uint32))&Lua_Client::HasRecipeLearned)
	.def("HasSkill", (bool(Lua_Client::*)(int))&Lua_Client::HasSkill)
	.def("HasSpellScribed", (bool(Lua_Client::*)(int))&Lua_Client::HasSpellScribed)
	.def("HasZoneFlag", (bool(Lua_Client::*)(uint32))&Lua_Client::HasZoneFlag)
	.def("Hungry", (bool(Lua_Client::*)(void))&Lua_Client::Hungry)
	.def("InZone", (bool(Lua_Client::*)(void))&Lua_Client::InZone)
	.def("IncStats", (void(Lua_Client::*)(int,int))&Lua_Client::IncStats)
	.def("IncreaseLanguageSkill", (void(Lua_Client::*)(uint8))&Lua_Client::IncreaseLanguageSkill)
	.def("IncreaseLanguageSkill", (void(Lua_Client::*)(uint8,uint8))&Lua_Client::IncreaseLanguageSkill)
	.def("IncreaseSkill", (void(Lua_Client::*)(int))&Lua_Client::IncreaseSkill)
	.def("IncreaseSkill", (void(Lua_Client::*)(int,int))&Lua_Client::IncreaseSkill)
	.def("IncrementAA", (void(Lua_Client::*)(int))&Lua_Client::IncrementAA)
	.def("IsAutoAttackEnabled", (bool(Lua_Client::*)(void))&Lua_Client::IsAutoAttackEnabled)
	.def("IsAutoFireEnabled", (bool(Lua_Client::*)(void))&Lua_Client::IsAutoFireEnabled)
	.def("IsCrouching", (bool(Lua_Client::*)(void))&Lua_Client::IsCrouching)
	.def("IsDead", &Lua_Client::IsDead)
	.def("IsDueling", (bool(Lua_Client::*)(void))&Lua_Client::IsDueling)
	.def("IsEXPEnabled", (bool(Lua_Client::*)(void))&Lua_Client::IsEXPEnabled)
	.def("IsGrouped", (bool(Lua_Client::*)(void))&Lua_Client::IsGrouped)
	.def("IsInAGuild", (bool(Lua_Client::*)(void))&Lua_Client::IsInAGuild)
	.def("IsLD", (bool(Lua_Client::*)(void))&Lua_Client::IsLD)
	.def("IsMedding", (bool(Lua_Client::*)(void))&Lua_Client::IsMedding)
	.def("IsNameChangeAllowed", &Lua_Client::IsNameChangeAllowed)
	.def("IsRaidGrouped", (bool(Lua_Client::*)(void))&Lua_Client::IsRaidGrouped)
	.def("IsSitting", (bool(Lua_Client::*)(void))&Lua_Client::IsSitting)
	.def("IsStanding", (bool(Lua_Client::*)(void))&Lua_Client::IsStanding)
	.def("IsTaskActive", (bool(Lua_Client::*)(int))&Lua_Client::IsTaskActive)
	.def("IsTaskActivityActive", (bool(Lua_Client::*)(int,int))&Lua_Client::IsTaskActivityActive)
	.def("IsTaskCompleted", (bool(Lua_Client::*)(int))&Lua_Client::IsTaskCompleted)
	.def("KeyRingAdd", (bool(Lua_Client::*)(uint32))&Lua_Client::KeyRingAdd)
	.def("KeyRingCheck", (bool(Lua_Client::*)(uint32))&Lua_Client::KeyRingCheck)
	.def("KeyRingClear", (bool(Lua_Client::*)(void))&Lua_Client::KeyRingClear)
	.def("KeyRingList", (void(Lua_Client::*)(void))&Lua_Client::KeyRingList)
	.def("KeyRingList", (void(Lua_Client::*)(Lua_Client))&Lua_Client::KeyRingList)
	.def("KeyRingRemove", (bool(Lua_Client::*)(uint32))&Lua_Client::KeyRingRemove)
	.def("Kick", (void(Lua_Client::*)(void))&Lua_Client::Kick)
	.def("LearnDisciplines", (uint16(Lua_Client::*)(uint8,uint8))&Lua_Client::LearnDisciplines)
	.def("LearnRecipe", (void(Lua_Client::*)(uint32))&Lua_Client::LearnRecipe)
	.def("LeaveGroup", (void(Lua_Client::*)(void))&Lua_Client::LeaveGroup)
	.def("LoadPEQZoneFlags", (void(Lua_Client::*)(void))&Lua_Client::LoadPEQZoneFlags)
	.def("LoadZoneFlags", (void(Lua_Client::*)(void))&Lua_Client::LoadZoneFlags)
	.def("LockSharedTask", &Lua_Client::LockSharedTask)
	.def("MarkSingleCompassLoc", (void(Lua_Client::*)(float,float,float))&Lua_Client::MarkSingleCompassLoc)
	.def("MarkSingleCompassLoc", (void(Lua_Client::*)(float,float,float,int))&Lua_Client::MarkSingleCompassLoc)
	.def("Marquee", (void(Lua_Client::*)(uint32, std::string))&Lua_Client::SendMarqueeMessage)
	.def("Marquee", (void(Lua_Client::*)(uint32, std::string, uint32))&Lua_Client::SendMarqueeMessage)
	.def("Marquee", (void(Lua_Client::*)(uint32, uint32, uint32, uint32, uint32, std::string))&Lua_Client::SendMarqueeMessage)
	.def("MaxSkill", (int(Lua_Client::*)(int))&Lua_Client::MaxSkill)
	.def("MaxSkills", (void(Lua_Client::*)(void))&Lua_Client::MaxSkills)
	.def("MemSpell", (void(Lua_Client::*)(int,int))&Lua_Client::MemSpell)
	.def("MemSpell", (void(Lua_Client::*)(int,int,bool))&Lua_Client::MemSpell)
	.def("MemmedCount", (int(Lua_Client::*)(void))&Lua_Client::MemmedCount)
	.def("MovePC", (void(Lua_Client::*)(int,float,float,float,float))&Lua_Client::MovePC)
	.def("MovePCDynamicZone", (void(Lua_Client::*)(std::string))&Lua_Client::MovePCDynamicZone)
	.def("MovePCDynamicZone", (void(Lua_Client::*)(std::string, int))&Lua_Client::MovePCDynamicZone)
	.def("MovePCDynamicZone", (void(Lua_Client::*)(std::string, int, bool))&Lua_Client::MovePCDynamicZone)
	.def("MovePCDynamicZone", (void(Lua_Client::*)(uint32))&Lua_Client::MovePCDynamicZone)
	.def("MovePCDynamicZone", (void(Lua_Client::*)(uint32, int))&Lua_Client::MovePCDynamicZone)
	.def("MovePCDynamicZone", (void(Lua_Client::*)(uint32, int, bool))&Lua_Client::MovePCDynamicZone)
	.def("MovePCInstance", (void(Lua_Client::*)(int,int,float,float,float,float))&Lua_Client::MovePCInstance)
	.def("MoveZone", (void(Lua_Client::*)(const char*))&Lua_Client::MoveZone)
	.def("MoveZone", (void(Lua_Client::*)(const char*,float,float,float))&Lua_Client::MoveZone)
	.def("MoveZone", (void(Lua_Client::*)(const char*,float,float,float,float))&Lua_Client::MoveZone)
	.def("MoveZoneGroup", (void(Lua_Client::*)(const char*))&Lua_Client::MoveZoneGroup)
	.def("MoveZoneGroup", (void(Lua_Client::*)(const char*,float,float,float))&Lua_Client::MoveZoneGroup)
	.def("MoveZoneGroup", (void(Lua_Client::*)(const char*,float,float,float,float))&Lua_Client::MoveZoneGroup)
	.def("MoveZoneInstance", (void(Lua_Client::*)(uint16))&Lua_Client::MoveZoneInstance)
	.def("MoveZoneInstance", (void(Lua_Client::*)(uint16,float,float,float))&Lua_Client::MoveZoneInstance)
	.def("MoveZoneInstance", (void(Lua_Client::*)(uint16,float,float,float,float))&Lua_Client::MoveZoneInstance)
	.def("MoveZoneInstanceGroup", (void(Lua_Client::*)(uint16))&Lua_Client::MoveZoneInstanceGroup)
	.def("MoveZoneInstanceGroup", (void(Lua_Client::*)(uint16,float,float,float))&Lua_Client::MoveZoneInstanceGroup)
	.def("MoveZoneInstanceGroup", (void(Lua_Client::*)(uint16,float,float,float,float))&Lua_Client::MoveZoneInstanceGroup)
	.def("MoveZoneInstanceRaid", (void(Lua_Client::*)(uint16))&Lua_Client::MoveZoneInstanceRaid)
	.def("MoveZoneInstanceRaid", (void(Lua_Client::*)(uint16,float,float,float))&Lua_Client::MoveZoneInstanceRaid)
	.def("MoveZoneInstanceRaid", (void(Lua_Client::*)(uint16,float,float,float,float))&Lua_Client::MoveZoneInstanceRaid)
	.def("MoveZoneRaid", (void(Lua_Client::*)(const char*))&Lua_Client::MoveZoneRaid)
	.def("MoveZoneRaid", (void(Lua_Client::*)(const char*,float,float,float))&Lua_Client::MoveZoneRaid)
	.def("MoveZoneRaid", (void(Lua_Client::*)(const char*,float,float,float,float))&Lua_Client::MoveZoneRaid)
	.def("NotifyNewTitlesAvailable", (void(Lua_Client::*)(void))&Lua_Client::NotifyNewTitlesAvailable)
	.def("NukeItem", (void(Lua_Client::*)(uint32))&Lua_Client::NukeItem)
	.def("NukeItem", (void(Lua_Client::*)(uint32,int))&Lua_Client::NukeItem)
	.def("OpenLFGuildWindow", (void(Lua_Client::*)(void))&Lua_Client::OpenLFGuildWindow)
	.def("PlayMP3", (void(Lua_Client::*)(std::string))&Lua_Client::PlayMP3)
	.def("Popup", (void(Lua_Client::*)(const char*,const char*))&Lua_Client::Popup)
	.def("Popup", (void(Lua_Client::*)(const char*,const char*,uint32))&Lua_Client::Popup)
	.def("Popup", (void(Lua_Client::*)(const char*,const char*,uint32,uint32))&Lua_Client::Popup)
	.def("Popup", (void(Lua_Client::*)(const char*,const char*,uint32,uint32,uint32))&Lua_Client::Popup)
	.def("Popup", (void(Lua_Client::*)(const char*,const char*,uint32,uint32,uint32,uint32))&Lua_Client::Popup)
	.def("Popup", (void(Lua_Client::*)(const char*,const char*,uint32,uint32,uint32,uint32,const char*,const char*))&Lua_Client::Popup)
	.def("Popup", (void(Lua_Client::*)(const char*,const char*,uint32,uint32,uint32,uint32,const char*,const char*,uint32))&Lua_Client::Popup)
	.def("PushItemOnCursor", (bool(Lua_Client::*)(Lua_ItemInst))&Lua_Client::PushItemOnCursor)
	.def("PutItemInInventory", (bool(Lua_Client::*)(int,Lua_ItemInst))&Lua_Client::PutItemInInventory)
	.def("QuestReadBook", (void(Lua_Client::*)(const char *,int))&Lua_Client::QuestReadBook)
	.def("QuestReward", (void(Lua_Client::*)(Lua_Mob))&Lua_Client::QuestReward)
	.def("QuestReward", (void(Lua_Client::*)(Lua_Mob, luabind::adl::object))&Lua_Client::QuestReward)
	.def("QuestReward", (void(Lua_Client::*)(Lua_Mob, uint32))&Lua_Client::QuestReward)
	.def("QuestReward", (void(Lua_Client::*)(Lua_Mob, uint32, uint32))&Lua_Client::QuestReward)
	.def("QuestReward", (void(Lua_Client::*)(Lua_Mob, uint32, uint32, uint32))&Lua_Client::QuestReward)
	.def("QuestReward", (void(Lua_Client::*)(Lua_Mob, uint32, uint32, uint32, uint32))&Lua_Client::QuestReward)
	.def("QuestReward", (void(Lua_Client::*)(Lua_Mob, uint32, uint32, uint32, uint32, uint32))&Lua_Client::QuestReward)
	.def("QuestReward", (void(Lua_Client::*)(Lua_Mob, uint32, uint32, uint32, uint32, uint32, uint32))&Lua_Client::QuestReward)
	.def("QuestReward", (void(Lua_Client::*)(Lua_Mob, uint32, uint32, uint32, uint32, uint32, uint32, bool))&Lua_Client::QuestReward)
	.def("QueuePacket", (void(Lua_Client::*)(Lua_Packet))&Lua_Client::QueuePacket)
	.def("QueuePacket", (void(Lua_Client::*)(Lua_Packet,bool))&Lua_Client::QueuePacket)
	.def("QueuePacket", (void(Lua_Client::*)(Lua_Packet,bool,int))&Lua_Client::QueuePacket)
	.def("QueuePacket", (void(Lua_Client::*)(Lua_Packet,bool,int,int))&Lua_Client::QueuePacket)
	.def("ReadBookByName", (void(Lua_Client::*)(std::string,uint8))&Lua_Client::ReadBookByName)
	.def("RefundAA", (void(Lua_Client::*)(void))&Lua_Client::RefundAA)
	.def("ReloadDataBuckets", (bool(Lua_Client::*)(void))&Lua_Client::ReloadDataBuckets)
	.def("RemoveAAPoints", (bool(Lua_Client::*)(uint32))&Lua_Client::RemoveAAPoints)
	.def("RemoveAllExpeditionLockouts", (void(Lua_Client::*)(std::string))&Lua_Client::RemoveAllExpeditionLockouts)
	.def("RemoveAllExpeditionLockouts", (void(Lua_Client::*)(void))&Lua_Client::RemoveAllExpeditionLockouts)
	.def("RemoveAlternateCurrencyValue", (bool(Lua_Client::*)(uint32,uint32))&Lua_Client::RemoveAlternateCurrencyValue)
	.def("RemoveEbonCrystals", (void(Lua_Client::*)(uint32))&Lua_Client::RemoveEbonCrystals)
	.def("RemoveExpeditionLockout", (void(Lua_Client::*)(std::string, std::string))&Lua_Client::RemoveExpeditionLockout)
	.def("RemoveItem", (void(Lua_Client::*)(uint32))&Lua_Client::RemoveItem)
	.def("RemoveItem", (void(Lua_Client::*)(uint32,uint32))&Lua_Client::RemoveItem)
	.def("RemoveLDoNLoss", (void(Lua_Client::*)(uint32))&Lua_Client::RemoveLDoNLoss)
	.def("RemoveLDoNWin", (void(Lua_Client::*)(uint32))&Lua_Client::RemoveLDoNWin)
	.def("RemoveRadiantCrystals", (void(Lua_Client::*)(uint32))&Lua_Client::RemoveRadiantCrystals)
	.def("ResetAA", (void(Lua_Client::*)(void))&Lua_Client::ResetAA)
	.def("ResetAllDisciplineTimers", (void(Lua_Client::*)(void))&Lua_Client::ResetAllDisciplineTimers)
	.def("ResetAllCastbarCooldowns", (void(Lua_Client::*)(void))&Lua_Client::ResetAllCastbarCooldowns)
	.def("ResetAlternateAdvancementRank", &Lua_Client::ResetAlternateAdvancementRank)
	.def("ResetCastbarCooldownBySlot", (void(Lua_Client::*)(int))&Lua_Client::ResetCastbarCooldownBySlot)
	.def("ResetCastbarCooldownBySpellID", (void(Lua_Client::*)(uint32))&Lua_Client::ResetCastbarCooldownBySpellID)
	.def("ResetDisciplineTimer", (void(Lua_Client::*)(uint32))&Lua_Client::ResetDisciplineTimer)
	.def("ResetItemCooldown", (void(Lua_Client::*)(uint32))&Lua_Client::ResetItemCooldown)
	.def("ResetLeadershipAA", (void(Lua_Client::*)(void))&Lua_Client::ResetLeadershipAA)
	.def("ResetTrade", (void(Lua_Client::*)(void))&Lua_Client::ResetTrade)
	.def("RewardFaction", (void(Lua_Client::*)(int,int))&Lua_Client::RewardFaction)
	.def("Save", (void(Lua_Client::*)(int))&Lua_Client::Save)
	.def("Save", (void(Lua_Client::*)(void))&Lua_Client::Save)
	.def("ScribeSpell", (void(Lua_Client::*)(int,int))&Lua_Client::ScribeSpell)
	.def("ScribeSpell", (void(Lua_Client::*)(int,int,bool))&Lua_Client::ScribeSpell)
	.def("ScribeSpells", (uint16(Lua_Client::*)(uint8,uint8))&Lua_Client::ScribeSpells)
	.def("SendColoredText", (void(Lua_Client::*)(uint32, std::string))&Lua_Client::SendColoredText)
	.def("SendItemScale", (void(Lua_Client::*)(Lua_ItemInst))&Lua_Client::SendItemScale)
	.def("SendGMCommand", (bool(Lua_Client::*)(std::string))&Lua_Client::SendGMCommand)
	.def("SendGMCommand", (bool(Lua_Client::*)(std::string,bool))&Lua_Client::SendGMCommand)
	.def("SendMarqueeMessage", (void(Lua_Client::*)(uint32, std::string))&Lua_Client::SendMarqueeMessage)
	.def("SendMarqueeMessage", (void(Lua_Client::*)(uint32, std::string, uint32))&Lua_Client::SendMarqueeMessage)
	.def("SendMarqueeMessage", (void(Lua_Client::*)(uint32, uint32, uint32, uint32, uint32, std::string))&Lua_Client::SendMarqueeMessage)
	.def("SendOPTranslocateConfirm", (void(Lua_Client::*)(Lua_Mob,int))&Lua_Client::SendOPTranslocateConfirm)
	.def("SendPath", (void(Lua_Client::*)(Lua_Mob))&Lua_Client::SendPath)
	.def("SendPEQZoneFlagInfo", (void(Lua_Client::*)(Lua_Client))&Lua_Client::SendPEQZoneFlagInfo)
	.def("SendSound", (void(Lua_Client::*)(void))&Lua_Client::SendSound)
	.def("SendToGuildHall", (void(Lua_Client::*)(void))&Lua_Client::SendToGuildHall)
	.def("SendToInstance", (void(Lua_Client::*)(std::string,std::string,uint32,float,float,float,float,std::string,uint32))&Lua_Client::SendToInstance)
	.def("SendPayload", (void(Lua_Client::*)(int))&Lua_Client::SendPayload)
	.def("SendPayload", (void(Lua_Client::*)(int,std::string))&Lua_Client::SendPayload)
	.def("SendWebLink", (void(Lua_Client::*)(const char *))&Lua_Client::SendWebLink)
	.def("SendZoneFlagInfo", (void(Lua_Client::*)(Lua_Client))&Lua_Client::SendZoneFlagInfo)
	.def("SetAAEXPModifier", (void(Lua_Client::*)(float))&Lua_Client::SetAAEXPModifier)
	.def("SetAAEXPModifier", (void(Lua_Client::*)(uint32,float))&Lua_Client::SetAAEXPModifier)
	.def("SetAAEXPModifier", (void(Lua_Client::*)(uint32,float,int16))&Lua_Client::SetAAEXPModifier)
	.def("SetAAEXPPercentage", (void(Lua_Client::*)(uint8))&Lua_Client::SetAAEXPPercentage)
	.def("SetAAPoints", (void(Lua_Client::*)(int))&Lua_Client::SetAAPoints)
	.def("SetAATitle", (void(Lua_Client::*)(std::string))&Lua_Client::SetAATitle)
	.def("SetAATitle", (void(Lua_Client::*)(std::string,bool))&Lua_Client::SetAATitle)
	.def("SetAFK", (void(Lua_Client::*)(uint8))&Lua_Client::SetAFK)
	.def("SetAccountFlag", (void(Lua_Client::*)(const std::string&,const std::string&))&Lua_Client::SetAccountFlag)
	.def("SetAlternateCurrencyValue", (void(Lua_Client::*)(uint32,uint32))&Lua_Client::SetAlternateCurrencyValue)
	.def("SetAnon", (void(Lua_Client::*)(uint8))&Lua_Client::SetAnon)
	.def("SetAutoLoginCharacterName", (bool(Lua_Client::*)(void))&Lua_Client::SetAutoLoginCharacterName)
	.def("SetAutoLoginCharacterName", (bool(Lua_Client::*)(std::string))&Lua_Client::SetAutoLoginCharacterName)
	.def("SetBaseClass", (void(Lua_Client::*)(int))&Lua_Client::SetBaseClass)
	.def("SetBaseGender", (void(Lua_Client::*)(int))&Lua_Client::SetBaseGender)
	.def("SetBaseRace", (void(Lua_Client::*)(int))&Lua_Client::SetBaseRace)
	.def("SetBindPoint", (void(Lua_Client::*)(int))&Lua_Client::SetBindPoint)
	.def("SetBindPoint", (void(Lua_Client::*)(int,int))&Lua_Client::SetBindPoint)
	.def("SetBindPoint", (void(Lua_Client::*)(int,int,float))&Lua_Client::SetBindPoint)
	.def("SetBindPoint", (void(Lua_Client::*)(int,int,float,float))&Lua_Client::SetBindPoint)
	.def("SetBindPoint", (void(Lua_Client::*)(int,int,float,float,float))&Lua_Client::SetBindPoint)
	.def("SetBindPoint", (void(Lua_Client::*)(int,int,float,float,float,float))&Lua_Client::SetBindPoint)
	.def("SetBindPoint", (void(Lua_Client::*)(void))&Lua_Client::SetBindPoint)
	.def("SetBotCreationLimit", (void(Lua_Client::*)(uint32))&Lua_Client::SetBotCreationLimit)
	.def("SetBotCreationLimit", (void(Lua_Client::*)(uint32,uint8))&Lua_Client::SetBotCreationLimit)
	.def("SetBotRequiredLevel", (void(Lua_Client::*)(int))&Lua_Client::SetBotRequiredLevel)
	.def("SetBotRequiredLevel", (void(Lua_Client::*)(int,uint8))&Lua_Client::SetBotRequiredLevel)
	.def("SetBotSpawnLimit", (void(Lua_Client::*)(int))&Lua_Client::SetBotSpawnLimit)
	.def("SetBotSpawnLimit", (void(Lua_Client::*)(int,uint8))&Lua_Client::SetBotSpawnLimit)
	.def("SetAccountBucket", (void(Lua_Client::*)(std::string,std::string))&Lua_Client::SetAccountBucket)
	.def("SetAccountBucket", (void(Lua_Client::*)(std::string,std::string,std::string))&Lua_Client::SetAccountBucket)
	.def("SetBucket", (void(Lua_Client::*)(std::string,std::string))&Lua_Client::SetBucket)
	.def("SetBucket", (void(Lua_Client::*)(std::string,std::string,std::string))&Lua_Client::SetBucket)
	.def("SetClientMaxLevel", (void(Lua_Client::*)(int))&Lua_Client::SetClientMaxLevel)
	.def("SetConsumption", (void(Lua_Client::*)(int, int))&Lua_Client::SetConsumption)
	.def("SetDeity", (void(Lua_Client::*)(int))&Lua_Client::SetDeity)
	.def("SetDuelTarget", (void(Lua_Client::*)(int))&Lua_Client::SetDuelTarget)
	.def("SetDueling", (void(Lua_Client::*)(bool))&Lua_Client::SetDueling)
	.def("SetEXP", (void(Lua_Client::*)(uint64,uint64))&Lua_Client::SetEXP)
	.def("SetEXP", (void(Lua_Client::*)(uint64,uint64,bool))&Lua_Client::SetEXP)
	.def("SetEXPEnabled", (void(Lua_Client::*)(bool))&Lua_Client::SetEXPEnabled)
	.def("SetEXPModifier", (void(Lua_Client::*)(float))&Lua_Client::SetEXPModifier)
	.def("SetEXPModifier", (void(Lua_Client::*)(uint32,float))&Lua_Client::SetEXPModifier)
	.def("SetEXPModifier", (void(Lua_Client::*)(uint32,float,int16))&Lua_Client::SetEXPModifier)
	.def("SetEbonCrystals", (void(Lua_Client::*)(uint32))&Lua_Client::SetEbonCrystals)
	.def("SetEndurance", (void(Lua_Client::*)(int))&Lua_Client::SetEndurance)
	.def("SetEnvironmentDamageModifier", (void(Lua_Client::*)(int))&Lua_Client::SetEnvironmentDamageModifier)
	.def("SetFactionLevel", (void(Lua_Client::*)(uint32,uint32,int,int,int))&Lua_Client::SetFactionLevel)
	.def("SetFactionLevel2", (void(Lua_Client::*)(uint32,int,int,int,int,int,int))&Lua_Client::SetFactionLevel2)
	.def("SetFeigned", (void(Lua_Client::*)(bool))&Lua_Client::SetFeigned)
	.def("SetGM", (void(Lua_Client::*)(bool))&Lua_Client::SetGM)
	.def("SetGMStatus", (void(Lua_Client::*)(int))&Lua_Client::SetGMStatus)
	.def("SetHideMe", (void(Lua_Client::*)(bool))&Lua_Client::SetHideMe)
	.def("SetHorseId", (void(Lua_Client::*)(int))&Lua_Client::SetHorseId)
	.def("SetHunger", (void(Lua_Client::*)(int))&Lua_Client::SetHunger)
	.def("SetInvulnerableEnvironmentDamage", (void(Lua_Client::*)(int))&Lua_Client::SetInvulnerableEnvironmentDamage)
	.def("SetIPExemption", (void(Lua_Client::*)(int))&Lua_Client::SetIPExemption)
	.def("SetItemCooldown", (void(Lua_Client::*)(uint32,uint32))&Lua_Client::SetItemCooldown)
	.def("SetLanguageSkill", (void(Lua_Client::*)(int,int))&Lua_Client::SetLanguageSkill)
	.def("SetLDoNPoints", (void(Lua_Client::*)(uint32,uint32))&Lua_Client::SetLDoNPoints)
	.def("SetMaterial", (void(Lua_Client::*)(int,uint32))&Lua_Client::SetMaterial)
	.def("SetPEQZoneFlag", (void(Lua_Client::*)(uint32))&Lua_Client::SetPEQZoneFlag)
	.def("SetPVP", (void(Lua_Client::*)(bool))&Lua_Client::SetPVP)
	.def("SetPrimaryWeaponOrnamentation", (void(Lua_Client::*)(uint32))&Lua_Client::SetPrimaryWeaponOrnamentation)
	.def("SetRadiantCrystals", (void(Lua_Client::*)(uint32))&Lua_Client::SetRadiantCrystals)
	.def("SetSecondaryWeaponOrnamentation", (void(Lua_Client::*)(uint32))&Lua_Client::SetSecondaryWeaponOrnamentation)
	.def("SetSkill", (void(Lua_Client::*)(int,int))&Lua_Client::SetSkill)
	.def("SetSkillPoints", (void(Lua_Client::*)(int))&Lua_Client::SetSkillPoints)
	.def("SetSpellDuration", (void(Lua_Client::*)(int))&Lua_Client::SetSpellDuration)
	.def("SetSpellDuration", (void(Lua_Client::*)(int,int))&Lua_Client::SetSpellDuration)
	.def("SetSpellDuration", (void(Lua_Client::*)(int,int,int))&Lua_Client::SetSpellDuration)
	.def("SetSpellDuration", (void(Lua_Client::*)(int,int,int,bool))&Lua_Client::SetSpellDuration)
	.def("SetSpellDuration", (void(Lua_Client::*)(int,int,int,bool,bool))&Lua_Client::SetSpellDuration)
	.def("SetSpellDurationGroup", (void(Lua_Client::*)(int))&Lua_Client::SetSpellDurationGroup)
	.def("SetSpellDurationGroup", (void(Lua_Client::*)(int,int))&Lua_Client::SetSpellDurationGroup)
	.def("SetSpellDurationGroup", (void(Lua_Client::*)(int,int,int))&Lua_Client::SetSpellDurationGroup)
	.def("SetSpellDurationGroup", (void(Lua_Client::*)(int,int,int,bool))&Lua_Client::SetSpellDurationGroup)
	.def("SetSpellDurationGroup", (void(Lua_Client::*)(int,int,int,bool,bool))&Lua_Client::SetSpellDurationGroup)
	.def("SetSpellDurationRaid", (void(Lua_Client::*)(int))&Lua_Client::SetSpellDurationRaid)
	.def("SetSpellDurationRaid", (void(Lua_Client::*)(int,int))&Lua_Client::SetSpellDurationRaid)
	.def("SetSpellDurationRaid", (void(Lua_Client::*)(int,int,int))&Lua_Client::SetSpellDurationRaid)
	.def("SetSpellDurationRaid", (void(Lua_Client::*)(int,int,int,bool))&Lua_Client::SetSpellDurationRaid)
	.def("SetSpellDurationRaid", (void(Lua_Client::*)(int,int,int,bool,bool))&Lua_Client::SetSpellDurationRaid)
	.def("SetSpellDurationRaid", (void(Lua_Client::*)(int,int,int,bool,bool,bool))&Lua_Client::SetSpellDurationRaid)
	.def("SetStartZone", (void(Lua_Client::*)(int))&Lua_Client::SetStartZone)
	.def("SetStartZone", (void(Lua_Client::*)(int,float))&Lua_Client::SetStartZone)
	.def("SetStartZone", (void(Lua_Client::*)(int,float,float))&Lua_Client::SetStartZone)
	.def("SetStartZone", (void(Lua_Client::*)(int,float,float,float))&Lua_Client::SetStartZone)
	.def("SetStats", (void(Lua_Client::*)(int,int))&Lua_Client::SetStats)
	.def("SetThirst", (void(Lua_Client::*)(int))&Lua_Client::SetThirst)
	.def("SetTint", (void(Lua_Client::*)(int,uint32))&Lua_Client::SetTint)
	.def("SetTitleSuffix", (void(Lua_Client::*)(const char *))&Lua_Client::SetTitleSuffix)
	.def("SetZoneFlag", (void(Lua_Client::*)(uint32))&Lua_Client::SetZoneFlag)
	.def("ShowZoneShardMenu", (void(Lua_Client::*)(void))&Lua_Client::ShowZoneShardMenu)
	.def("Signal", (void(Lua_Client::*)(int))&Lua_Client::Signal)
	.def("Sit", (void(Lua_Client::*)(void))&Lua_Client::Sit)
	.def("Stand", (void(Lua_Client::*)(void))&Lua_Client::Stand)
	.def("SummonBaggedItems", (void(Lua_Client::*)(uint32,luabind::adl::object))&Lua_Client::SummonBaggedItems)
	.def("SummonItem", (void(Lua_Client::*)(uint32))&Lua_Client::SummonItem)
	.def("SummonItem", (void(Lua_Client::*)(uint32,int))&Lua_Client::SummonItem)
	.def("SummonItem", (void(Lua_Client::*)(uint32,int,uint32))&Lua_Client::SummonItem)
	.def("SummonItem", (void(Lua_Client::*)(uint32,int,uint32,uint32))&Lua_Client::SummonItem)
	.def("SummonItem", (void(Lua_Client::*)(uint32,int,uint32,uint32,uint32))&Lua_Client::SummonItem)
	.def("SummonItem", (void(Lua_Client::*)(uint32,int,uint32,uint32,uint32,uint32))&Lua_Client::SummonItem)
	.def("SummonItem", (void(Lua_Client::*)(uint32,int,uint32,uint32,uint32,uint32,uint32))&Lua_Client::SummonItem)
	.def("SummonItem", (void(Lua_Client::*)(uint32,int,uint32,uint32,uint32,uint32,uint32,bool))&Lua_Client::SummonItem)
	.def("SummonItem", (void(Lua_Client::*)(uint32,int,uint32,uint32,uint32,uint32,uint32,bool,int))&Lua_Client::SummonItem)
	.def("SummonItemIntoInventory", (void(Lua_Client::*)(luabind::adl::object))&Lua_Client::SummonItemIntoInventory)
	.def("TGB", (bool(Lua_Client::*)(void))&Lua_Client::TGB)
	.def("TakeMoneyFromPP", (bool(Lua_Client::*)(uint64))&Lua_Client::TakeMoneyFromPP)
	.def("TakeMoneyFromPP", (bool(Lua_Client::*)(uint64,bool))&Lua_Client::TakeMoneyFromPP)
	.def("TakePlatinum", (bool(Lua_Client::*)(uint32))&Lua_Client::TakePlatinum)
	.def("TakePlatinum", (bool(Lua_Client::*)(uint32,bool))&Lua_Client::TakePlatinum)
	.def("TaskSelector", (void(Lua_Client::*)(luabind::adl::object))&Lua_Client::TaskSelector)
	.def("TaskSelector", (void(Lua_Client::*)(luabind::adl::object, bool))&Lua_Client::TaskSelector)
	.def("TeleportToPlayerByCharID", (bool(Lua_Client::*)(uint32))&Lua_Client::TeleportToPlayerByCharID)
	.def("TeleportToPlayerByName", (bool(Lua_Client::*)(std::string))&Lua_Client::TeleportToPlayerByName)
	.def("TeleportGroupToPlayerByCharID", (bool(Lua_Client::*)(uint32))&Lua_Client::TeleportGroupToPlayerByCharID)
	.def("TeleportGroupToPlayerByName", (bool(Lua_Client::*)(std::string))&Lua_Client::TeleportGroupToPlayerByName)
	.def("TeleportRaidToPlayerByCharID", (bool(Lua_Client::*)(uint32))&Lua_Client::TeleportRaidToPlayerByCharID)
	.def("TeleportRaidToPlayerByName", (bool(Lua_Client::*)(std::string))&Lua_Client::TeleportRaidToPlayerByName)
	.def("Thirsty", (bool(Lua_Client::*)(void))&Lua_Client::Thirsty)
	.def("TrainDisc", (void(Lua_Client::*)(int))&Lua_Client::TrainDisc)
	.def("TrainDiscBySpellID", (void(Lua_Client::*)(int32))&Lua_Client::TrainDiscBySpellID)
	.def("UnFreeze", (void(Lua_Client::*)(void))&Lua_Client::UnFreeze)
	.def("UncompleteTask", (bool(Lua_Client::*)(int))&Lua_Client::UncompleteTask)
	.def("Undye", (void(Lua_Client::*)(void))&Lua_Client::Undye)
	.def("UnmemSpell", (void(Lua_Client::*)(int))&Lua_Client::UnmemSpell)
	.def("UnmemSpell", (void(Lua_Client::*)(int,bool))&Lua_Client::UnmemSpell)
	.def("UnmemSpellAll", (void(Lua_Client::*)(bool))&Lua_Client::UnmemSpellAll)
	.def("UnmemSpellAll", (void(Lua_Client::*)(void))&Lua_Client::UnmemSpellAll)
	.def("UnmemSpellBySpellID", (void(Lua_Client::*)(int32))&Lua_Client::UnmemSpellBySpellID)
	.def("UnscribeSpell", (void(Lua_Client::*)(int))&Lua_Client::UnscribeSpell)
	.def("UnscribeSpell", (void(Lua_Client::*)(int,bool))&Lua_Client::UnscribeSpell)
	.def("UnscribeSpellAll", (void(Lua_Client::*)(bool))&Lua_Client::UnscribeSpellAll)
	.def("UnscribeSpellAll", (void(Lua_Client::*)(void))&Lua_Client::UnscribeSpellAll)
	.def("UnscribeSpellBySpellID", (void(Lua_Client::*)(uint16))&Lua_Client::UnscribeSpellBySpellID)
	.def("UnscribeSpellBySpellID", (void(Lua_Client::*)(uint16,bool))&Lua_Client::UnscribeSpellBySpellID)
	.def("UntrainDisc", (void(Lua_Client::*)(int))&Lua_Client::UntrainDisc)
	.def("UntrainDisc", (void(Lua_Client::*)(int,bool))&Lua_Client::UntrainDisc)
	.def("UntrainDiscAll", (void(Lua_Client::*)(bool))&Lua_Client::UntrainDiscAll)
	.def("UntrainDiscAll", (void(Lua_Client::*)(void))&Lua_Client::UntrainDiscAll)
	.def("UntrainDiscBySpellID", (void(Lua_Client::*)(uint16))&Lua_Client::UntrainDiscBySpellID)
	.def("UntrainDiscBySpellID", (void(Lua_Client::*)(uint16,bool))&Lua_Client::UntrainDiscBySpellID)
	.def("UpdateAdmin", (void(Lua_Client::*)(void))&Lua_Client::UpdateAdmin)
	.def("UpdateAdmin", (void(Lua_Client::*)(bool))&Lua_Client::UpdateAdmin)
	.def("UpdateGroupAAs", (void(Lua_Client::*)(int,uint32))&Lua_Client::UpdateGroupAAs)
	.def("UpdateLDoNPoints", (void(Lua_Client::*)(uint32,int))&Lua_Client::UpdateLDoNPoints)
	.def("UpdateTaskActivity", (void(Lua_Client::*)(int,int,int))&Lua_Client::UpdateTaskActivity)
	.def("UseDiscipline", (bool(Lua_Client::*)(int,int))&Lua_Client::UseDiscipline)
	.def("UseAugmentContainer", (void(Lua_Client::*)(int))&Lua_Client::UseAugmentContainer)
	.def("WorldKick", (void(Lua_Client::*)(void))&Lua_Client::WorldKick);
}

luabind::scope lua_register_inventory_where() {
	return luabind::class_<InventoryWhere>("InventoryWhere")
		.enum_("constants")
		[(
			luabind::value("Personal", static_cast<int>(invWherePersonal)),
			luabind::value("Bank", static_cast<int>(invWhereBank)),
			luabind::value("SharedBank", static_cast<int>(invWhereSharedBank)),
			luabind::value("Trading", static_cast<int>(invWhereTrading)),
			luabind::value("Cursor", static_cast<int>(invWhereCursor))
		)];
}

#endif // LUA_EQEMU
