#include "common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "zone/client.h"
#include "zone/dialogue_window.h"
#include "zone/dynamic_zone.h"
#include "zone/embperl.h"
#include "zone/titles.h"

void Perl_Client_SendSound(Client* self) // @categories Script Utility
{
	self->SendSound();
}

bool Perl_Client_Save(Client* self, uint8 commit_now) // @categories Script Utility
{
	return self->Save(commit_now);
}

bool Perl_Client_Connected(Client* self) // @categories Script Utility
{
	return self->Connected();
}

bool Perl_Client_InZone(Client* self) // @categories Script Utility
{
	return self->InZone();
}

void Perl_Client_Kick(Client* self) // @categories Script Utility
{
	self->Kick("Perl Quest");
}

void Perl_Client_Disconnect(Client* self) // @categories Script Utility
{
	self->Disconnect();
}

bool Perl_Client_IsLD(Client* self) // @categories Account and Character
{
	return self->IsLD();
}

void Perl_Client_WorldKick(Client* self) // @categories Script Utility
{
	self->WorldKick();
}

void Perl_Client_SendToGuildHall(Client* self) // @categories Script Utility, Guild
{
	self->SendToGuildHall();
}

int Perl_Client_GetAnon(Client* self) // @categories Account and Character, Stats and Attributes
{
	return self->GetAnon();
}

void Perl_Client_SetAnon(Client* self, uint8 anon_flag) // @categories Account and Character, Stats and Attributes
{
	self->SetAnon(anon_flag);
}

int Perl_Client_GetAFK(Client* self) // @categories Account and Character, Stats and Attributes
{
	return self->GetAFK();
}

void Perl_Client_SetAFK(Client* self, uint8 afk_flag) // @categories Account and Character, Stats and Attributes
{
	self->SetAFK(afk_flag);
}

void Perl_Client_Duck(Client* self) // @categories Account and Character
{
	self->Duck();
}

void Perl_Client_DyeArmorBySlot(Client* self, uint8 slot, uint8 red, uint8 green, uint8 blue) // @categories Account and Character, Inventory and Items
{
	self->DyeArmorBySlot(slot, red, green, blue);
}

void Perl_Client_DyeArmorBySlot(Client* self, uint8 slot, uint8 red, uint8 green, uint8 blue, uint8 use_tint) // @categories Account and Character, Inventory and Items
{
	self->DyeArmorBySlot(slot, red, green, blue, use_tint);
}

void Perl_Client_Stand(Client* self) // @categories Script Utility
{
	self->Stand();
}

void Perl_Client_SetGM(Client* self, bool on) // @categories Account and Character
{
	self->SetGM(on);
}

void Perl_Client_SetPVP(Client* self, bool on) // @categories Account and Character
{
	self->SetPVP(on);
}

bool Perl_Client_GetPVP(Client* self) // @categories Account and Character, Stats and Attributes
{
	return self->GetPVP();
}

bool Perl_Client_GetGM(Client* self) // @categories Account and Character
{
	return self->GetGM();
}

void Perl_Client_SetBaseClass(Client* self, uint32 class_id) // @categories Account and Character, Stats and Attributes
{
	self->SetBaseClass(class_id);
}

void Perl_Client_SetBaseRace(Client* self, uint32 race_id) // @categories Account and Character, Stats and Attributes
{
	self->SetBaseRace(race_id);
}

void Perl_Client_SetBaseGender(Client* self, uint32 gender_id) // @categories Account and Character, Stats and Attributes
{
	self->SetBaseGender(gender_id);
}

int Perl_Client_GetBaseFace(Client* self) // @categories Stats and Attributes
{
	return self->GetBaseFace();
}

int Perl_Client_GetLanguageSkill(Client* self, uint16 lanuage_id) // @categories Account and Character, Stats and Attributes
{
	return self->GetLanguageSkill(lanuage_id);
}

uint32_t Perl_Client_GetLDoNPointsTheme(Client* self, int theme) // @categories Currency and Points
{
	return self->GetLDoNPointsTheme(theme);
}

int Perl_Client_GetBaseSTR(Client* self) // @categories Stats and Attributes
{
	return self->GetBaseSTR();
}

int Perl_Client_GetBaseSTA(Client* self) // @categories Stats and Attributes
{
	return self->GetBaseSTA();
}

int Perl_Client_GetBaseCHA(Client* self) // @categories Stats and Attributes
{
	return self->GetBaseCHA();
}

int Perl_Client_GetBaseDEX(Client* self) // @categories Stats and Attributes
{
	return self->GetBaseDEX();
}

int Perl_Client_GetBaseINT(Client* self) // @categories Stats and Attributes
{
	return self->GetBaseINT();
}

int Perl_Client_GetBaseAGI(Client* self) // @categories Stats and Attributes
{
	return self->GetBaseAGI();
}

int Perl_Client_GetBaseWIS(Client* self) // @categories Stats and Attributes
{
	return self->GetBaseWIS();
}

int Perl_Client_GetWeight(Client* self) // @categories Account and Character, Stats and Attributes
{
	return self->GetWeight();
}

uint32_t Perl_Client_GetEXP(Client* self) // @categories Experience and Level
{
	return self->GetEXP();
}

uint32_t Perl_Client_GetAAExp(Client* self) // @categories Alternative Advancement, Experience and Level
{
	return self->GetAAXP();
}

uint32_t Perl_Client_GetAAPercent(Client* self) // @categories Alternative Advancement, Experience and Level
{
	return self->GetAAPercent();
}

uint32_t Perl_Client_GetTotalSecondsPlayed(Client* self) // @categories Account and Character, Stats and Attributes
{
	return self->GetTotalSecondsPlayed();
}

bool Perl_Client_UpdateLDoNPoints(Client* self, uint32 theme_id, int points) // @categories Currency and Points
{
	return self->UpdateLDoNPoints(theme_id, points);
}

void Perl_Client_SetDeity(Client* self, uint32 deity_id) // @categories Account and Character, Stats and Attributes
{
	self->SetDeity(deity_id);
}

void Perl_Client_AddEXP(Client* self, uint32 add_exp) // @categories Experience and Level
{
	self->AddEXP(ExpSource::Quest, add_exp);
}

void Perl_Client_AddEXP(Client* self, uint32 add_exp, uint8 conlevel) // @categories Experience and Level
{
	self->AddEXP(ExpSource::Quest, add_exp, conlevel);
}

void Perl_Client_AddEXP(Client* self, uint32 add_exp, uint8 conlevel, bool resexp) // @categories Experience and Level
{
	self->AddEXP(ExpSource::Quest, add_exp, conlevel, resexp);
}

void Perl_Client_SetEXP(Client* self, uint64 set_exp, uint64 set_aaxp) // @categories Experience and Level
{
	self->SetEXP(ExpSource::Quest, set_exp, set_aaxp);
}

void Perl_Client_SetEXP(Client* self, uint64 set_exp, uint64 set_aaxp, bool resexp) // @categories Experience and Level
{
	self->SetEXP(ExpSource::Quest, set_exp, set_aaxp, resexp);
}

void Perl_Client_SetBindPoint(Client* self) // @categories Account and Character, Stats and Attributes
{
	self->SetBindPoint2(0);
}

void Perl_Client_SetBindPoint(Client* self, int to_zone) // @categories Account and Character, Stats and Attributes
{
	self->SetBindPoint2(0, to_zone);
}

void Perl_Client_SetBindPoint(Client* self, int to_zone, int to_instance) // @categories Account and Character, Stats and Attributes
{
	self->SetBindPoint2(0, to_zone, to_instance);
}

void Perl_Client_SetBindPoint(Client* self, int to_zone, int to_instance, float new_x) // @categories Account and Character, Stats and Attributes
{
	self->SetBindPoint2(0, to_zone, to_instance, glm::vec4(new_x, 0.0f, 0.0f, 0.0f));
}

void Perl_Client_SetBindPoint(Client* self, int to_zone, int to_instance, float new_x, float new_y) // @categories Account and Character, Stats and Attributes
{
	self->SetBindPoint2(0, to_zone, to_instance, glm::vec4(new_x, new_y, 0.0f, 0.0f));
}

void Perl_Client_SetBindPoint(Client* self, int to_zone, int to_instance, float new_x, float new_y, float new_z) // @categories Account and Character, Stats and Attributes
{
	self->SetBindPoint2(0, to_zone, to_instance, glm::vec4(new_x, new_y, new_z, 0.0f));
}

void Perl_Client_SetBindPoint(Client* self, int to_zone, int to_instance, float new_x, float new_y, float new_z, float new_heading) // @categories Account and Character, Stats and Attributes
{
	self->SetBindPoint2(0, to_zone, to_instance, glm::vec4(new_x, new_y, new_z, new_heading));
}

float Perl_Client_GetBindX(Client* self) // @categories Account and Character
{
	return self->GetBindX();
}

float Perl_Client_GetBindX(Client* self, int index) // @categories Account and Character
{
	return self->GetBindX(index);
}

float Perl_Client_GetBindY(Client* self) // @categories Account and Character
{
	return self->GetBindY();
}

float Perl_Client_GetBindY(Client* self, int index) // @categories Account and Character
{
	return self->GetBindY(index);
}

float Perl_Client_GetBindZ(Client* self) // @categories Account and Character
{
	return self->GetBindZ();
}

float Perl_Client_GetBindZ(Client* self, int index) // @categories Account and Character
{
	return self->GetBindZ(index);
}

float Perl_Client_GetBindHeading(Client* self) // @categories Account and Character
{
	return self->GetBindHeading();
}

float Perl_Client_GetBindHeading(Client* self, int index) // @categories Account and Character
{
	return self->GetBindHeading(index);
}

uint32_t Perl_Client_GetBindZoneID(Client* self) // @categories Account and Character
{
	return self->GetBindZoneID();
}

uint32_t Perl_Client_GetBindZoneID(Client* self, int index) // @categories Account and Character
{
	return self->GetBindZoneID(index);
}

void Perl_Client_MovePC(Client* self, uint32 zone_id, float x, float y, float z, float heading) // @categories Script Utility
{
	self->MovePC(zone_id, x, y, z, heading);
}

void Perl_Client_MovePCInstance(Client* self, uint32 zone_id, uint32 instance_id, float x, float y, float z, float heading) // @categories Adventures and Expeditions, Script Utility
{
	self->MovePC(zone_id, instance_id, x, y, z, heading);
}

void Perl_Client_ChangeLastName(Client* self, std::string last_name) // @categories Account and Character
{
	self->ChangeLastName(last_name);
}

int Perl_Client_GetFactionLevel(Client* self, uint32 char_id, uint32 npc_id, uint32 race_id, uint32 class_id, uint32 deity_id, uint32 faction_id, Mob* tnpc) // @categories Faction
{
	return self->GetFactionLevel(char_id, npc_id, race_id, class_id, deity_id, faction_id, tnpc);
}

void Perl_Client_SetFactionLevel(Client* self, uint32 char_id, uint32 npc_faction_id, uint8 char_class, uint8 char_race, uint8 char_deity) // @categories Faction
{
	self->SetFactionLevel(char_id, npc_faction_id, char_class, char_race, char_deity);
}

void Perl_Client_SetFactionLevel2(Client* self, uint32 char_id, int32 faction_id, uint8 char_class, uint8 char_race, uint8 char_deity, int32 value) // @categories Faction
{
	self->SetFactionLevel2(char_id, faction_id, char_class, char_race, char_deity, value, 0);
}

void Perl_Client_SetFactionLevel2(Client* self, uint32 char_id, int32 faction_id, uint8 char_class, uint8 char_race, uint8 char_deity, int32 value, uint8 temp) // @categories Faction
{
	self->SetFactionLevel2(char_id, faction_id, char_class, char_race, char_deity, value, temp);
}

int Perl_Client_GetRawItemAC(Client* self) // @categories Inventory and Items
{
	return self->GetRawItemAC();
}

uint32_t Perl_Client_AccountID(Client* self) // @categories Account and Character
{
	return self->AccountID();
}

std::string Perl_Client_AccountName(Client* self) // @categories Account and Character
{
	return self->AccountName();
}

int16 Perl_Client_Admin(Client* self) // @categories Account and Character
{
	return self->Admin();
}

uint32_t Perl_Client_CharacterID(Client* self) // @categories Account and Character
{
	return self->CharacterID();
}

void Perl_Client_UpdateAdmin(Client* self) // @categories Account and Character
{
	self->UpdateAdmin();
}

void Perl_Client_UpdateAdmin(Client* self, bool from_database) // @categories Account and Character
{
	self->UpdateAdmin(from_database);
}

void Perl_Client_UpdateWho(Client* self) // @categories Script Utility
{
	self->UpdateWho();
}

void Perl_Client_UpdateWho(Client* self, uint8 remove) // @categories Script Utility
{
	self->UpdateWho(remove);
}

int Perl_Client_GuildRank(Client* self) // @categories Account and Character, Guild
{
	return self->GuildRank();
}

uint32_t Perl_Client_GuildID(Client* self) // @categories Account and Character, Guild
{
	return self->GuildID();
}

int Perl_Client_GetFace(Client* self) // @categories Account and Character, Stats and Attributes
{
	return self->GetFace();
}

bool Perl_Client_TakeMoneyFromPP(Client* self, uint64 copper) // @categories Currency and Points
{
	return self->TakeMoneyFromPP(copper);
}

bool Perl_Client_TakeMoneyFromPP(Client* self, uint64 copper, bool update_client) // @categories Currency and Points
{
	return self->TakeMoneyFromPP(copper, update_client);
}

void Perl_Client_AddMoneyToPP(Client* self, uint32 copper, uint32 silver, uint32 gold, uint32 platinum) // @categories Currency and Points
{
	self->AddMoneyToPP(copper, silver, gold, platinum);
}

void Perl_Client_AddMoneyToPP(Client* self, uint32 copper, uint32 silver, uint32 gold, uint32 platinum, bool update_client) // @categories Currency and Points
{
	self->AddMoneyToPP(copper, silver, gold, platinum, update_client);
}

bool Perl_Client_TGB(Client* self) // @categories Spells and Disciplines
{
	return self->TGB();
}

int Perl_Client_GetSkillPoints(Client* self) // @categories Skills and Recipes
{
	return self->GetSkillPoints();
}

void Perl_Client_SetSkillPoints(Client* self, int points) // @categories Skills and Recipes
{
	self->SetSkillPoints(points);
}

void Perl_Client_IncreaseSkill(Client* self, int skill_id) // @categories Skills and Recipes
{
	self->IncreaseSkill(skill_id);
}

void Perl_Client_IncreaseSkill(Client* self, int skill_id, int value) // @categories Skills and Recipes
{
	self->IncreaseSkill(skill_id, value);
}

void Perl_Client_IncreaseLanguageSkill(Client* self, uint8 language_id) // @categories Skills and Recipes
{
	self->IncreaseLanguageSkill(language_id);
}

void Perl_Client_IncreaseLanguageSkill(Client* self, uint8 language_id, uint8 increase) // @categories Skills and Recipes
{
	self->IncreaseLanguageSkill(language_id, increase);
}

uint32_t Perl_Client_GetRawSkill(Client* self, int skill_id) // @categories Skills and Recipes
{
	return self->GetRawSkill(static_cast<EQ::skills::SkillType>(skill_id));
}

bool Perl_Client_HasSkill(Client* self, int skill_id) // @categories Skills and Recipes
{
	return self->HasSkill(static_cast<EQ::skills::SkillType>(skill_id));
}

bool Perl_Client_CanHaveSkill(Client* self, int skill_id) // @categories Skills and Recipes
{
	return self->CanHaveSkill(static_cast<EQ::skills::SkillType>(skill_id));
}

void Perl_Client_SetSkill(Client* self, int skill_id, uint16 value) // @categories Skills and Recipes
{
	self->SetSkill(static_cast<EQ::skills::SkillType>(skill_id), value);
}

void Perl_Client_AddSkill(Client* self, int skill_id, uint16 value) // @categories Skills and Recipes
{
	self->AddSkill(static_cast<EQ::skills::SkillType>(skill_id), value);
}

void Perl_Client_CheckSpecializeIncrease(Client* self, uint16 spell_id) // @categories Spells and Disciplines
{
	self->CheckSpecializeIncrease(spell_id);
}

bool Perl_Client_CheckIncreaseSkill(Client* self, int skill_id) // @categories Skills and Recipes
{
	return self->CheckIncreaseSkill(static_cast<EQ::skills::SkillType>(skill_id), nullptr);
}

bool Perl_Client_CheckIncreaseSkill(Client* self, int skill_id, int chance_modifier) // @categories Skills and Recipes
{
	return self->CheckIncreaseSkill(static_cast<EQ::skills::SkillType>(skill_id), nullptr, chance_modifier);
}

void Perl_Client_SetLanguageSkill(Client* self, uint8 language_id, uint8 language_skill) // @categories Account and Character, Skills and Recipes, Stats and Attributes
{
	self->SetLanguageSkill(language_id, language_skill);
}

int Perl_Client_MaxSkill(Client* self, uint16 skill_id) // @categories Skills and Recipes
{
	return self->MaxSkill(static_cast<EQ::skills::SkillType>(skill_id), self->GetClass(), self->GetLevel());
}

int Perl_Client_MaxSkill(Client* self, uint16 skill_id, uint16 class_id) // @categories Skills and Recipes
{
	return self->MaxSkill(static_cast<EQ::skills::SkillType>(skill_id), class_id, self->GetLevel());
}

int Perl_Client_MaxSkill(Client* self, uint16 skill_id, uint16 class_id, uint16 level) // @categories Skills and Recipes
{
	return self->MaxSkill(static_cast<EQ::skills::SkillType>(skill_id), class_id, level);
}

void Perl_Client_GMKill(Client* self) // @categories Script Utility
{
	self->GMKill();
}

bool Perl_Client_IsMedding(Client* self) // @categories Account and Character
{
	return self->IsMedding();
}

uint32_t Perl_Client_GetDuelTarget(Client* self) // @categories Account and Character, Script Utility
{
	return self->GetDuelTarget();
}

bool Perl_Client_IsDueling(Client* self) // @categories Account and Character
{
	return self->IsDueling();
}

void Perl_Client_SetDuelTarget(Client* self, uint32_t set_id) // @categories Account and Character
{
	self->SetDuelTarget(set_id);
}

void Perl_Client_SetDueling(Client* self, bool duel) // @categories Account and Character, Script Utility
{
	self->SetDueling(duel);
}

void Perl_Client_ResetAA(Client* self) // @categories Alternative Advancement
{
	self->ResetAA();
}

void Perl_Client_MemSpell(Client* self, uint16 spell_id, int slot) // @categories Spells and Disciplines
{
	self->MemSpell(spell_id, slot);
}

void Perl_Client_MemSpell(Client* self, uint16 spell_id, int slot, bool update_client) // @categories Spells and Disciplines
{
	self->MemSpell(spell_id, slot, update_client);
}

void Perl_Client_UnmemSpell(Client* self, int slot) // @categories Spells and Disciplines
{
	self->UnmemSpell(slot);
}

void Perl_Client_UnmemSpell(Client* self, int slot, bool update_client) // @categories Spells and Disciplines
{
	self->UnmemSpell(slot, update_client);
}

void Perl_Client_UnmemSpellBySpellID(Client* self, int spell_id) // @categories Spells and Disciplines
{
	self->UnmemSpellBySpellID(spell_id);
}

void Perl_Client_UnmemSpellAll(Client* self) // @categories Spells and Disciplines
{
	self->UnmemSpellAll();
}

void Perl_Client_UnmemSpellAll(Client* self, bool update_client) // @categories Spells and Disciplines
{
	self->UnmemSpellAll(update_client);
}

int Perl_Client_FindEmptyMemSlot(Client* self) // @categories Account and Character, Spells and Disciplines
{
	return self->FindEmptyMemSlot();
}

int Perl_Client_FindMemmedSpellBySlot(Client* self, int slot) // @categories Account and Character, Spells and Disciplines
{
	return self->FindMemmedSpellBySlot(slot);
}

int Perl_Client_FindMemmedSpellBySpellID(Client* self, uint16 spell_id) // @categories Account and Character, Spells and Disciplines
{
	return self->FindMemmedSpellBySpellID(spell_id);
}

int Perl_Client_MemmedCount(Client* self) // @categories Spells and Disciplines
{
	return self->MemmedCount();
}

void Perl_Client_ScribeSpell(Client* self, uint16 spell_id, int slot) // @categories Spells and Disciplines
{
	self->ScribeSpell(spell_id, slot);
}

void Perl_Client_ScribeSpell(Client* self, uint16 spell_id, int slot, bool update_client) // @categories Spells and Disciplines
{
	self->ScribeSpell(spell_id, slot, update_client);
}

void Perl_Client_UnscribeSpell(Client* self, int slot) // @categories Spells and Disciplines
{
	self->UnscribeSpell(slot);
}

void Perl_Client_UnscribeSpell(Client* self, int slot, bool update_client) // @categories Spells and Disciplines
{
	self->UnscribeSpell(slot, update_client);
}

void Perl_Client_UnscribeSpellAll(Client* self)
{
	self->UnscribeSpellAll();
}

void Perl_Client_UnscribeSpellAll(Client* self, bool update_client)
{
	self->UnscribeSpellAll(update_client);
}

void Perl_Client_TrainDiscBySpellID(Client* self, int spell_id) // @categories Spells and Disciplines
{
	self->TrainDiscBySpellID(spell_id);
}

int Perl_Client_GetDiscSlotBySpellID(Client* self, int spell_id) // @categories Spells and Disciplines
{
	return self->GetDiscSlotBySpellID(spell_id);
}

void Perl_Client_UntrainDisc(Client* self, int slot) // @categories Spells and Disciplines
{
	self->UntrainDisc(slot);
}

void Perl_Client_UntrainDisc(Client* self, int slot, bool update_client) // @categories Spells and Disciplines
{
	self->UntrainDisc(slot, update_client);
}

void Perl_Client_UntrainDiscAll(Client* self) // @categories Spells and Disciplines
{
	self->UntrainDiscAll();
}

void Perl_Client_UntrainDiscAll(Client* self, bool update_client) // @categories Spells and Disciplines
{
	self->UntrainDiscAll(update_client);
}

bool Perl_Client_IsStanding(Client* self) // @categories Account and Character
{
	return self->IsStanding();
}

void Perl_Client_Sit(Client* self)
{
	self->Sit();
}

bool Perl_Client_IsSitting(Client* self) // @categories Account and Character
{
	return self->IsSitting();
}

bool Perl_Client_IsCrouching(Client* self) // @categories Account and Character
{
	return self->IsCrouching();
}

bool Perl_Client_IsBecomeNPC(Client* self) // @categories Account and Character
{
	return self->IsBecomeNPC();
}

int Perl_Client_GetBecomeNPCLevel(Client* self) // @categories Experience and Level
{
	return self->GetBecomeNPCLevel();
}

void Perl_Client_SetBecomeNPC(Client* self, bool flag) // @categories Account and Character, Stats and Attributes
{
	self->SetBecomeNPC(flag);
}

void Perl_Client_SetBecomeNPCLevel(Client* self, uint8 level) // @categories Account and Character, Stats and Attributes
{
	self->SetBecomeNPCLevel(level);
}

void Perl_Client_SetFeigned(Client* self, bool feigned) // @categories Script Utility
{
	self->SetFeigned(feigned);
}

bool Perl_Client_GetFeigned(Client* self) // @categories Script Utility
{
	return self->GetFeigned();
}

bool Perl_Client_AutoSplitEnabled(Client* self) // @categories Currency and Points
{
	return self->AutoSplitEnabled();
}

void Perl_Client_SetHorseId(Client* self, uint16_t horseid) // @categories Script Utility
{
	self->SetHorseId(horseid);
}

int Perl_Client_GetHorseId(Client* self) // @categories Account and Character, Script Utility
{
	return self->GetHorseId();
}

uint32 Perl_Client_NukeItem(Client* self, uint32 item_id) // @categories Inventory and Items
{
	return self->NukeItem(item_id, 0xFF);
}

uint32 Perl_Client_NukeItem(Client* self, uint32 item_id, uint8 slot_to_check) // @categories Inventory and Items
{
	return self->NukeItem(item_id, slot_to_check);
}

void Perl_Client_SetTint(Client* self, int16 slot_id, uint32 color) // @categories Inventory and Items
{
	self->SetTint(slot_id, color);
}

void Perl_Client_SetMaterial(Client* self, int16 slot_id, uint32 item_id) // @categories Inventory and Items
{
	self->SetMaterial(slot_id, item_id);
}

void Perl_Client_Undye(Client* self) // @categories Script Utility
{
	self->Undye();
}

int Perl_Client_GetItemIDAt(Client* self, int16 slot_id) // @categories Inventory and Items
{
	return self->GetItemIDAt(slot_id);
}

int Perl_Client_GetAugmentIDAt(Client* self, int16 slot_id, uint8 aug_slot) // @categories Inventory and Items
{
	return self->GetAugmentIDAt(slot_id, aug_slot);
}

void Perl_Client_DeleteItemInInventory(Client* self, int16 slot_id) // @categories Inventory and Items
{
	self->DeleteItemInInventory(slot_id);
}

void Perl_Client_DeleteItemInInventory(Client* self, int16 slot_id, int16 quantity) // @categories Inventory and Items
{
	self->DeleteItemInInventory(slot_id, quantity);
}

void Perl_Client_DeleteItemInInventory(Client* self, int16 slot_id, int16 quantity, bool client_update) // @categories Inventory and Items
{
	self->DeleteItemInInventory(slot_id, quantity, client_update);
}

void Perl_Client_SummonItem(Client* self, uint32 item_id) // @categories Inventory and Items, Script Utility
{
	self->SummonItem(item_id);
}

void Perl_Client_SummonItem(Client* self, uint32 item_id, int16 charges) // @categories Inventory and Items, Script Utility
{
	self->SummonItem(item_id, charges);
}

void Perl_Client_SummonItem(Client* self, uint32 item_id, int16 charges, bool attune) // @categories Inventory and Items, Script Utility
{
	self->SummonItem(item_id, charges, 0, 0, 0, 0, 0, 0, attune);
}

void Perl_Client_SummonItem(Client* self, uint32 item_id, int16 charges, bool attune, uint32 aug1) // @categories Inventory and Items, Script Utility
{
	self->SummonItem(item_id, charges, aug1, 0, 0, 0, 0, 0, attune);
}

void Perl_Client_SummonItem(Client* self, uint32 item_id, int16 charges, bool attune, uint32 aug1, uint32 aug2) // @categories Inventory and Items, Script Utility
{
	self->SummonItem(item_id, charges, aug1, aug2, 0, 0, 0, 0, attune);
}

void Perl_Client_SummonItem(Client* self, uint32 item_id, int16 charges, bool attune, uint32 aug1, uint32 aug2, uint32 aug3) // @categories Inventory and Items, Script Utility
{
	self->SummonItem(item_id, charges, aug1, aug2, aug3, 0, 0, 0, attune);
}

void Perl_Client_SummonItem(Client* self, uint32 item_id, int16 charges, bool attune, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4) // @categories Inventory and Items, Script Utility
{
	self->SummonItem(item_id, charges, aug1, aug2, aug3, aug4, 0, 0, attune);
}

void Perl_Client_SummonItem(Client* self, uint32 item_id, int16 charges, bool attune, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5) // @categories Inventory and Items, Script Utility
{
	self->SummonItem(item_id, charges, aug1, aug2, aug3, aug4, aug5, 0, attune);
}

void Perl_Client_SummonItem(Client* self, uint32 item_id, int16 charges, bool attune, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5, uint16 slot_id) // @categories Inventory and Items, Script Utility
{
	self->SummonItem(item_id, charges, aug1, aug2, aug3, aug4, aug5, 0, attune, slot_id);
}

void Perl_Client_SetStats(Client* self, uint8 type, uint16 increase_val) // @categories Account and Character, Stats and Attributes
{
	self->SetStats(type, increase_val);
}

void Perl_Client_IncStats(Client* self, uint8 type, uint16 increase_val) // @categories Account and Character, Stats and Attributes
{
	self->IncStats(type, increase_val);
}

void Perl_Client_DropItem(Client* self, int16 slot_id) // @categories Inventory and Items
{
	self->DropItem(slot_id);
}

void Perl_Client_BreakInvis(Client* self) // @categories Spells and Disciplines, Script Utility
{
	self->BreakInvis();
}

Group* Perl_Client_GetGroup(Client* self) // @categories Account and Character, Group
{
	return self->GetGroup();
}

void Perl_Client_LeaveGroup(Client* self) // @categories Account and Character, Group
{
	self->LeaveGroup();
}

Raid* Perl_Client_GetRaid(Client* self) // @categories Account and Character, Raid
{
	return self->GetRaid();
}

bool Perl_Client_IsGrouped(Client* self) // @categories Account and Character, Group
{
	return self->IsGrouped();
}

bool Perl_Client_IsRaidGrouped(Client* self) // @categories Account and Character, Group, Raid
{
	return self->IsRaidGrouped();
}

bool Perl_Client_Hungry(Client* self) // @categories Account and Character, Stats and Attributes
{
	return self->Hungry();
}

bool Perl_Client_Thirsty(Client* self) // @categories Script Utility
{
	return self->Thirsty();
}

int Perl_Client_GetInstrumentMod(Client* self, uint16 spell_id) // @categories Spells and Disciplines
{
	return self->GetInstrumentMod(spell_id);
}

bool Perl_Client_DecreaseByID(Client* self, uint32 type, int16 quantity) // @categories Script Utility
{
	return self->DecreaseByID(type, quantity);
}

int Perl_Client_SlotConvert2(Client* self, uint8 slot) // @categories Inventory and Items
{
	return self->SlotConvert2(slot);
}

void Perl_Client_Escape(Client* self) // @categories Account and Character, Skills and Recipes
{
	self->Escape();
}

void Perl_Client_RemoveNoRent(Client* self) // @categories Inventory and Items
{
	self->RemoveNoRent();
}

void Perl_Client_GoFish(Client* self) // @categories Skills and Recipes
{
	self->GoFish();
}

void Perl_Client_ForageItem(Client* self) // @categories Skills and Recipes
{
	self->ForageItem();
}

float Perl_Client_CalcPriceMod(Client* self) // @categories Currency and Points
{
	return self->CalcPriceMod();
}

float Perl_Client_CalcPriceMod(Client* self, Mob* other) // @categories Currency and Points
{
	return self->CalcPriceMod(other);
}

float Perl_Client_CalcPriceMod(Client* self, Mob* other, bool reverse) // @categories Currency and Points
{
	return self->CalcPriceMod(other, reverse);
}

void Perl_Client_ResetTrade(Client* self) // @categories Script Utility
{
	self->ResetTrade();
}

bool Perl_Client_UseDiscipline(Client* self, uint32 spell_id, uint32 target) // @categories Spells and Disciplines
{
	return self->UseDiscipline(spell_id, target);
}

uint32_t Perl_Client_GetDisciplineTimer(Client* self, uint32 timer_id) // @categories Spells and Disciplines
{
	return self->GetDisciplineTimer(timer_id);
}

void Perl_Client_ResetDisciplineTimer(Client* self, uint32_t timer_id) // @categories Spells and Disciplines
{
	self->ResetDisciplineTimer(timer_id);
}

int Perl_Client_GetCharacterFactionLevel(Client* self, int faction_id) // @categories Faction
{
	return self->GetCharacterFactionLevel(faction_id);
}

void Perl_Client_SetZoneFlag(Client* self, uint32 zone_id) // @categories Account and Character, Zones
{
	self->SetZoneFlag(zone_id);
}

void Perl_Client_ClearZoneFlag(Client* self, uint32 zone_id) // @categories Script Utility
{
	self->ClearZoneFlag(zone_id);
}

bool Perl_Client_HasZoneFlag(Client* self, uint32 zone_id) // @categories Account and Character
{
	return self->HasZoneFlag(zone_id);
}

void Perl_Client_SendZoneFlagInfo(Client* self, Client* to) // @categories Account and Character, Zones
{
	self->SendZoneFlagInfo(to);
}

void Perl_Client_LoadZoneFlags(Client* self) // @categories Zones
{
	self->LoadZoneFlags();
}

void Perl_Client_SetAATitle(Client* self, std::string title) // @categories Alternative Advancement
{
	if (title.size() > 31)
	{
		throw std::runtime_error("Title must be 31 characters or less.");
	}

	self->SetAATitle(title);
}

void Perl_Client_SetAATitle(Client* self, std::string title, bool save) // @categories Alternative Advancement
{
	if (title.size() > 31)
	{
		throw std::runtime_error("Title must be 31 characters or less.");
	}

	if (!save) {
		self->SetAATitle(title);
	} else {
		title_manager.CreateNewPlayerTitle(self, title);
	}
}

uint32_t Perl_Client_GetClientVersion(Client* self) // @categories Script Utility
{
	return static_cast<uint32_t>(self->ClientVersion());
}

uint32_t Perl_Client_GetClientVersionBit(Client* self) // @categories Script Utility
{
	return self->ClientVersionBit();
}

void Perl_Client_SetTitleSuffix(Client* self, std::string suffix) // @categories Account and Character
{
	if (suffix.size() > 31)
	{
		throw std::runtime_error("Suffix must be 31 characters or less.");
	}

	self->SetTitleSuffix(suffix);
}

void Perl_Client_SetTitleSuffix(Client* self, std::string suffix, bool save) // @categories Account and Character
{
	if (suffix.size() > 31)
	{
		throw std::runtime_error("Suffix must be 31 characters or less.");
	}

	if (!save) {
		self->SetTitleSuffix(suffix);
	} else {
		title_manager.CreateNewPlayerSuffix(self, suffix);
	}
}

void Perl_Client_SetAAPoints(Client* self, uint32 points) // @categories Alternative Advancement
{
	return self->SetAAPoints(points);
}

uint32_t Perl_Client_GetAAPoints(Client* self) // @categories Alternative Advancement, Experience and Level
{
	return self->GetAAPoints();
}

uint32_t Perl_Client_GetSpentAA(Client* self) // @categories Alternative Advancement
{
	return self->GetSpentAA();
}

void Perl_Client_AddAAPoints(Client* self, uint32 points) // @categories Alternative Advancement
{
	self->AddAAPoints(points);
}

void Perl_Client_RefundAA(Client* self) // @categories Alternative Advancement
{
	self->RefundAA();
}

int Perl_Client_GetModCharacterFactionLevel(Client* self, int faction_id) // @categories Faction
{
	return self->GetModCharacterFactionLevel(faction_id);
}

uint32_t Perl_Client_GetLDoNWins(Client* self) // @categories Currency and Points
{
	return self->GetLDoNWins();
}

uint32_t Perl_Client_GetLDoNLosses(Client* self) // @categories Currency and Points
{
	return self->GetLDoNLosses();
}

uint32_t Perl_Client_GetLDoNWinsTheme(Client* self, uint32_t theme) // @categories Currency and Points
{
	return self->GetLDoNWinsTheme(theme);
}

uint32_t Perl_Client_GetLDoNLossesTheme(Client* self, uint32_t theme) // @categories Currency and Points
{
	return self->GetLDoNLossesTheme(theme);
}

EQ::ItemInstance* Perl_Client_GetItemAt(Client* self, uint32 slot) // @categories Inventory and Items
{
	return self->GetInv().GetItem(slot);
}

EQ::ItemInstance* Perl_Client_GetAugmentAt(Client* self, uint32 slot, uint32 aug_slot) // @categories Inventory and Items
{
	EQ::ItemInstance* inst = self->GetInv().GetItem(slot);
	if (inst)
	{
		return inst->GetAugment(aug_slot);
	}
	return nullptr;
}

uint32_t Perl_Client_GetStartZone(Client* self) // @categories Account and Character
{
	return self->GetStartZone();
}

void Perl_Client_SetStartZone(Client* self, uint32 zone_id)
{
	self->SetStartZone(zone_id);
}

void Perl_Client_SetStartZone(Client* self, uint32 zone_id, float x, float y, float z)
{
	self->SetStartZone(zone_id, x, y, z);
}

void Perl_Client_SetStartZone(Client* self, uint32 zone_id, float x, float y, float z, float heading)
{
	self->SetStartZone(zone_id, x, y, z, heading);
}

void Perl_Client_AddPVPPoints(Client* self, uint32 points) // @categories Currency and Points
{
	self->AddPVPPoints(points);
}

void Perl_Client_AddCrystals(Client* self, uint32 radiant_count, uint32 ebon_count) // @categories Currency and Points
{
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

void Perl_Client_SetEbonCrystals(Client* self, uint32 value)
{
	self->SetEbonCrystals(value);
}

void Perl_Client_SetRadiantCrystals(Client* self, uint32 value)
{
	self->SetRadiantCrystals(value);
}

uint32_t Perl_Client_GetPVPPoints(Client* self) // @categories Currency and Points
{
	return self->GetPVPPoints();
}

uint32_t Perl_Client_GetRadiantCrystals(Client* self) // @categories Currency and Points
{
	return self->GetRadiantCrystals();
}

uint32_t Perl_Client_GetEbonCrystals(Client* self) // @categories Currency and Points
{
	return self->GetEbonCrystals();
}

void Perl_Client_ReadBook(Client* self, const char* book_text, uint8 type) // @categories Script Utility
{
	self->QuestReadBook(book_text, type);
}

void Perl_Client_SetGMStatus(Client* self, int new_status) // @categories Script Utility
{
	self->SetGMStatus(new_status);
}

int16 Perl_Client_GetGMStatus(Client* self) // @categories Account and Character
{
	return self->Admin();
}

void Perl_Client_UpdateGroupAAs(Client* self, int points, uint32 type) // @categories Alternative Advancement, Group
{
	self->UpdateGroupAAs(points, type);
}

uint32_t Perl_Client_GetGroupPoints(Client* self) // @categories Account and Character, Group
{
	return self->GetGroupPoints();
}

uint32_t Perl_Client_GetRaidPoints(Client* self) // @categories Account and Character, Raid
{
	return self->GetRaidPoints();
}

void Perl_Client_LearnRecipe(Client* self, uint32 recipe_id) // @categories Skills and Recipes
{
	self->LearnRecipe(recipe_id);
}

int64_t Perl_Client_GetEndurance(Client* self) // @categories Stats and Attributes
{
	return self->GetEndurance();
}

int64_t Perl_Client_GetMaxEndurance(Client* self) // @categories Account and Character, Stats and Attributes
{
	return self->GetMaxEndurance();
}

int Perl_Client_GetEnduranceRatio(Client* self) // @categories Stats and Attributes
{
	return self->GetEndurancePercent();
}

void Perl_Client_SetEndurance(Client* self, int endurance) // @categories Account and Character, Stats and Attributes
{
	self->SetEndurance(endurance);
}

void Perl_Client_SendOPTranslocateConfirm(Client* self, Mob* caster, uint16 spell_id) // @categories Script Utility
{
	self->SendOPTranslocateConfirm(caster, spell_id);
}

void Perl_Client_NPCSpawn(Client* self, NPC* target_npc, const char* option) // @categories Script Utility, Spawns
{
	return self->NPCSpawn(target_npc, option, 1200);
}

void Perl_Client_NPCSpawn(Client* self, NPC* target_npc, const char* option, uint32 respawn_time) // @categories Script Utility, Spawns
{
	return self->NPCSpawn(target_npc, option, respawn_time);
}

uint32_t Perl_Client_GetIP(Client* self) // @categories Script Utility
{
	return self->GetIP();
}

void Perl_Client_AddLevelBasedExp(Client* self, uint8 exp_percentage) // @categories Experience and Level
{
	self->AddLevelBasedExp(ExpSource::Quest, exp_percentage);
}

void Perl_Client_AddLevelBasedExp(Client* self, uint8 exp_percentage, uint8 max_level) // @categories Experience and Level
{
	self->AddLevelBasedExp(ExpSource::Quest, exp_percentage, max_level);
}

void Perl_Client_AddLevelBasedExp(Client* self, uint8 exp_percentage, uint8 max_level, bool ignore_mods) // @categories Experience and Level
{
	self->AddLevelBasedExp(ExpSource::Quest, exp_percentage, max_level, ignore_mods);
}

void Perl_Client_IncrementAA(Client* self, uint32 aa_skill_id) // @categories Alternative Advancement
{
	self->IncrementAlternateAdvancementRank(aa_skill_id);
}

bool Perl_Client_GrantAlternateAdvancementAbility(Client* self, int aa_id, int points) // @categories Alternative Advancement
{
	return self->GrantAlternateAdvancementAbility(aa_id, points);
}

bool Perl_Client_GrantAlternateAdvancementAbility(Client* self, int aa_id, int points, bool ignore_cost) // @categories Alternative Advancement
{
	return self->GrantAlternateAdvancementAbility(aa_id, points, ignore_cost);
}

void Perl_Client_ResetAlternateAdvancementRank(Client* self, int aa_id) // @categories Alternative Advancement
{
	return self->ResetAlternateAdvancementRank(aa_id);
}

uint32_t Perl_Client_GetAALevel(Client* self, uint32 aa_skill_id) // @categories Alternative Advancement, Experience and Level
{
	return self->GetAA(aa_skill_id);
}

void Perl_Client_MarkCompassLoc(Client* self, float x, float y, float z) // @categories Adventures and Expeditions
{
	self->MarkSingleCompassLoc(x, y, z);
}

void Perl_Client_ClearCompassMark(Client* self) // @categories Adventures and Expeditions
{
	self->MarkSingleCompassLoc(0, 0, 0, 0);
}

int Perl_Client_GetFreeSpellBookSlot(Client* self) // @categories Spells and Disciplines
{
	return self->GetNextAvailableSpellBookSlot();
}

int Perl_Client_GetFreeSpellBookSlot(Client* self, uint32 start_slot) // @categories Spells and Disciplines
{
	return self->GetNextAvailableSpellBookSlot(start_slot);
}

int Perl_Client_GetSpellBookSlotBySpellID(Client* self, uint32 spell_id) // @categories Spells and Disciplines
{
	return self->FindSpellBookSlotBySpellID(spell_id);
}

uint32_t Perl_Client_GetSpellIDByBookSlot(Client* self, int slot_id)
{
	return self->GetSpellIDByBookSlot(slot_id);
}

void Perl_Client_UpdateTaskActivity(Client* self, int task_id, int activity_id, int count) // @categories Tasks and Activities
{
	self->UpdateTaskActivity(task_id, activity_id, count);
}

void Perl_Client_UpdateTaskActivity(Client* self, int task_id, int activity_id, int count, bool ignore_quest_update) // @categories Tasks and Activities
{
	self->UpdateTaskActivity(task_id, activity_id, count, ignore_quest_update);
}

int Perl_Client_GetTaskActivityDoneCount(Client* self, int task_id, int activity_id) // @categories Tasks and Activities
{
	return self->GetTaskActivityDoneCountFromTaskID(task_id, activity_id);
}

void Perl_Client_AssignTask(Client* self, int task_id) // @categories Tasks and Activities
{
	self->AssignTask(task_id);
}

void Perl_Client_AssignTask(Client* self, int task_id, int npc_id) // @categories Tasks and Activities
{
	self->AssignTask(task_id, npc_id);
}

void Perl_Client_AssignTask(Client* self, int task_id, int npc_id, bool enforce_level_requirement) // @categories Tasks and Activities
{
	self->AssignTask(task_id, npc_id, enforce_level_requirement);
}

void Perl_Client_FailTask(Client* self, int task_id) // @categories Tasks and Activities
{
	self->FailTask(task_id);
}

bool Perl_Client_IsTaskCompleted(Client* self, int task_id) // @categories Tasks and Activities
{
	return self->IsTaskCompleted(task_id);
}

bool Perl_Client_IsTaskActive(Client* self, int task_id) // @categories Tasks and Activities
{
	return self->IsTaskActive(task_id);
}

bool Perl_Client_IsTaskActivityActive(Client* self, int task_id, int activity_id) // @categories Tasks and Activities
{
	return self->IsTaskActivityActive(task_id, activity_id);
}

void Perl_Client_LockSharedTask(Client* self, bool lock)
{
	return self->LockSharedTask(lock);
}

void Perl_Client_EndSharedTask(Client* self)
{
	return self->EndSharedTask();
}

void Perl_Client_EndSharedTask(Client* self, bool send_fail)
{
	return self->EndSharedTask(send_fail);
}

int64_t Perl_Client_GetCorpseCount(Client* self) // @categories Account and Character, Corpse
{
	return self->GetCorpseCount();
}

uint32_t Perl_Client_GetCorpseID(Client* self, uint8 corpse) // @categories Account and Character, Corpse
{
	return self->GetCorpseID(corpse);
}

uint32_t Perl_Client_GetCorpseItemAt(Client* self, uint32 corpse_id, uint16 slot_id) // @categories Inventory and Items, Corpse
{
	return self->GetCorpseItemAt(corpse_id, slot_id);
}

void Perl_Client_AssignToInstance(Client* self, uint16 instance_id) // @categories Adventures and Expeditions
{
	self->AssignToInstance(instance_id);
}

void Perl_Client_RemoveFromInstance(Client* self, uint16 instance_id) // @categories Adventures and Expeditions
{
	self->RemoveFromInstance(instance_id);
}

void Perl_Client_Freeze(Client* self)
{
	self->SendAppearancePacket(AppearanceType::Animation, Animation::Freeze);
}

void Perl_Client_UnFreeze(Client* self)
{
	self->SendAppearancePacket(AppearanceType::Animation, Animation::Standing);
}

uint32 Perl_Client_GetAggroCount(Client* self) // @categories Script Utility, Hate and Aggro
{
	return self->GetAggroCount();
}

uint64_t Perl_Client_GetCarriedMoney(Client* self) // @categories Currency and Points
{
	return self->GetCarriedMoney();
}

uint64_t Perl_Client_GetAllMoney(Client* self) // @categories Currency and Points
{
	return self->GetAllMoney();
}

EQ::ItemInstance* Perl_Client_GetItemInInventory(Client* self, int16 slot_id) // @categories Inventory and Items
{
	return self->GetInv().GetItem(slot_id);
}

void Perl_Client_SetCustomItemData(Client* self, int16 slot_id, std::string identifier, std::string value) // @categories Inventory and Items
{
	self->GetInv().SetCustomItemData(self->CharacterID(), slot_id, identifier, value);
}

std::string Perl_Client_GetCustomItemData(Client* self, int16 slot_id, std::string identifier) // @categories Inventory and Items, Corpse
{
	return self->GetInv().GetCustomItemData(slot_id, identifier);
}

void Perl_Client_OpenLFGuildWindow(Client* self) // @categories Script Utility, Guild
{
	self->OpenLFGuildWindow();
}

void Perl_Client_NotifyNewTitlesAvailable(Client* self) // @categories Account and Character
{
	self->NotifyNewTitlesAvailable();
}

void Perl_Client_AddAlternateCurrencyValue(Client* self, uint32 currency_id, int amount) // @categories Currency and Points
{
	self->AddAlternateCurrencyValue(currency_id, amount, true);
}

void Perl_Client_SetAlternateCurrencyValue(Client* self, uint32 currency_id, uint32 amount) // @categories Currency and Points
{
	self->SetAlternateCurrencyValue(currency_id, amount);
}

uint32 Perl_Client_GetAlternateCurrencyValue(Client* self, uint32_t currency_id) // @categories Currency and Points
{
	return self->GetAlternateCurrencyValue(currency_id);
}

void Perl_Client_SendWebLink(Client* self, const char* url) // @categories Script Utility
{
	self->SendWebLink(url);
}

int Perl_Client_GetInstanceID(Client* self) // @categories Adventures and Expeditions
{
	return self->GetInstanceID();
}

bool Perl_Client_HasSpellScribed(Client* self, int spell_id) // @categories Spells and Disciplines
{
	return self->HasSpellScribed(spell_id);
}

void Perl_Client_ClearAccountFlag(Client* self, std::string flag) // @categories Account and Character
{
	self->ClearAccountFlag(flag);
}

void Perl_Client_SetAccountFlag(Client* self, std::string flag, std::string value) // @categories Account and Character
{
	self->SetAccountFlag(flag, value);
}

std::string Perl_Client_GetAccountFlag(Client* self, std::string flag) // @categories Account and Character
{
	return self->GetAccountFlag(flag);
}

perl::array Perl_Client_GetAccountFlags(Client* self)
{
	perl::array result;

	const auto& l = self->GetAccountFlags();

	result.reserve(l.size());

	for (const auto& e : l) {
		result.push_back(e);
	}

	return result;
}

int Perl_Client_GetHunger(Client* self) // @categories Account and Character, Stats and Attributes
{
	return self->GetHunger();
}

int Perl_Client_GetThirst(Client* self) // @categories Account and Character, Stats and Attributes
{
	return self->GetThirst();
}

int Perl_Client_GetIntoxication(Client* self) // @categories Account and Character, Stats and Attributes
{
	return self->GetIntoxication();
}

void Perl_Client_SetHunger(Client* self, int in_hunger) // @categories Script Utility, Stats and Attributes
{
	self->SetHunger(in_hunger);
}

void Perl_Client_SetThirst(Client* self, int in_thirst) // @categories Account and Character, Stats and Attributes
{
	self->SetThirst(in_thirst);
}

void Perl_Client_SendTargetCommand(Client* self, uint32 entity_id) // @categories Script Utility
{
	self->SendTargetCommand(entity_id);
}

void Perl_Client_SetConsumption(Client* self, int hunger_amount, int thirst_amount) // @categories Script Utility, Stats and Attributes
{
	self->SetConsumption(hunger_amount, thirst_amount);
}

void Perl_Client_SilentMessage(Client* self, const char* message) // @categories Script Utility
{
	if (self->GetTarget() != NULL) {
		if (self->GetTarget()->IsNPC()) {
			if (DistanceSquaredNoZ(self->GetPosition(), self->GetTarget()->GetPosition()) <= 200) {
				if (self->GetTarget()->CastToNPC()->IsMoving() &&
					  !self->GetTarget()->CastToNPC()->IsOnHatelist(self->GetTarget()))
					self->GetTarget()->CastToNPC()->PauseWandering(RuleI(NPC, SayPauseTimeInSec));
				self->ChannelMessageReceived(ChatChannel_Say, Language::CommonTongue, Language::MaxValue, message, nullptr, true);
			}
		}
	}
}

void Perl_Client_PlayMP3(Client* self, const char* file) // @categories Script Utility
{
	self->PlayMP3(file);
}

// todo: this is some legacy api for 'cust_inst_players' and can possibly be removed (not related to current expeditions)
void Perl_Client_ExpeditionMessage(Client* self, int expedition_id, const char* message) // @categories Adventures and Expeditions
{
	self->ExpeditionSay(message, expedition_id);
}

void Perl_Client_SendColoredText(Client* self, uint32 color, std::string msg) // @categories Script Utility
{
	self->SendColoredText(color, std::move(msg));
}

void Perl_Client_SendSpellAnim(Client* self, uint16 targetid, uint16 spell_id)
{
	self->SendSpellAnim(targetid, spell_id);
}

float Perl_Client_GetTargetRingX(Client* self) // @categories Script Utility
{
	return self->GetTargetRingX();
}

float Perl_Client_GetTargetRingY(Client* self) // @categories Script Utility
{
	return self->GetTargetRingY();
}

float Perl_Client_GetTargetRingZ(Client* self) // @categories Script Utility
{
	return self->GetTargetRingZ();
}

uint64_t Perl_Client_CalcEXP(Client* self, uint8 consider_level)
{
	return self->CalcEXP(consider_level);
}

uint64_t Perl_Client_CalcEXP(Client* self, uint8 consider_level, bool ignore_modifiers)
{
	return self->CalcEXP(consider_level, ignore_modifiers);
}

void Perl_Client_QuestReward(Client* self, Mob* mob) // @categories Currency and Points, Experience and Level, Inventory and Items, Faction
{
	self->QuestReward(mob);
}

void Perl_Client_QuestReward(Client* self, Mob* mob, uint32 copper) // @categories Currency and Points, Experience and Level, Inventory and Items, Faction
{
	self->QuestReward(mob, copper);
}

void Perl_Client_QuestReward(Client* self, Mob* mob, uint32 copper, uint32 silver) // @categories Currency and Points, Experience and Level, Inventory and Items, Faction
{
	self->QuestReward(mob, copper, silver);
}

void Perl_Client_QuestReward(Client* self, Mob* mob, uint32 copper, uint32 silver, uint32 gold) // @categories Currency and Points, Experience and Level, Inventory and Items, Faction
{
	self->QuestReward(mob, copper, silver, gold);
}

void Perl_Client_QuestReward(Client* self, Mob* mob, uint32 copper, uint32 silver, uint32 gold, uint32 platinum) // @categories Currency and Points, Experience and Level, Inventory and Items, Faction
{
	self->QuestReward(mob, copper, silver, gold, platinum);
}

void Perl_Client_QuestReward(Client* self, Mob* mob, uint32 copper, uint32 silver, uint32 gold, uint32 platinum, uint32 item_id) // @categories Currency and Points, Experience and Level, Inventory and Items, Faction
{
	self->QuestReward(mob, copper, silver, gold, platinum, item_id);
}

void Perl_Client_QuestReward(Client* self, Mob* mob, uint32 copper, uint32 silver, uint32 gold, uint32 platinum, uint32 item_id, uint32 exp) // @categories Currency and Points, Experience and Level, Inventory and Items, Faction
{
	self->QuestReward(mob, copper, silver, gold, platinum, item_id, exp);
}

void Perl_Client_QuestReward(Client* self, Mob* mob, uint32 copper, uint32 silver, uint32 gold, uint32 platinum, uint32 item_id, uint32 exp, bool faction) // @categories Currency and Points, Experience and Level, Inventory and Items, Faction
{
	self->QuestReward(mob, copper, silver, gold, platinum, item_id, exp, faction);
}

void Perl_Client_CashReward(Client* self, uint32 copper, uint32 silver, uint32 gold, uint32 platinum)
{
	self->CashReward(copper, silver, gold, platinum);
}

uint32_t Perl_Client_GetMoney(Client* self, int8 type, int8 subtype)
{
	return self->GetMoney(type, subtype);
}

int Perl_Client_GetAccountAge(Client* self)
{
	return self->GetAccountAge();
}

void Perl_Client_Popup2(Client* self, const char* title, const char* text) // @categories Script Utility
{
	self->SendFullPopup(title, text);
}

void Perl_Client_Popup2(Client* self, const char* title, const char* text, uint32 popup_id) // @categories Script Utility
{
	self->SendFullPopup(title, text, popup_id);
}

void Perl_Client_Popup2(Client* self, const char* title, const char* text, uint32 popup_id, uint32 negative_id) // @categories Script Utility
{
	self->SendFullPopup(title, text, popup_id, negative_id);
}

void Perl_Client_Popup2(Client* self, const char* title, const char* text, uint32 popup_id, uint32 negative_id, uint32 buttons) // @categories Script Utility
{
	self->SendFullPopup(title, text, popup_id, negative_id, buttons);
}

void Perl_Client_Popup2(Client* self, const char* title, const char* text, uint32 popup_id, uint32 negative_id, uint32 buttons, uint32 duration) // @categories Script Utility
{
	self->SendFullPopup(title, text, popup_id, negative_id, buttons, duration);
}

void Perl_Client_Popup2(Client* self, const char* title, const char* text, uint32 popup_id, uint32 negative_id, uint32 buttons, uint32 duration, const char* button_name_0) // @categories Script Utility
{
	self->SendFullPopup(title, text, popup_id, negative_id, buttons, duration, button_name_0);
}

void Perl_Client_Popup2(Client* self, const char* title, const char* text, uint32 popup_id, uint32 negative_id, uint32 buttons, uint32 duration, const char* button_name_0, const char* button_name_1) // @categories Script Utility
{
	self->SendFullPopup(title, text, popup_id, negative_id, buttons, duration, button_name_0, button_name_1);
}

void Perl_Client_Popup2(Client* self, const char* title, const char* text, uint32 popup_id, uint32 negative_id, uint32 buttons, uint32 duration, const char* button_name_0, const char* button_name_1, uint32 sound_controls) // @categories Script Utility
{
	self->SendFullPopup(title, text, popup_id, negative_id, buttons, duration, button_name_0, button_name_1, sound_controls);
}

void Perl_Client_SetPrimaryWeaponOrnamentation(Client* self, int model_id) // @categories Account and Character, Inventory and Items
{
	self->SetPrimaryWeaponOrnamentation(model_id);
}

void Perl_Client_SetSecondaryWeaponOrnamentation(Client* self, int model_id) // @categories Account and Character, Inventory and Items
{
	self->SetSecondaryWeaponOrnamentation(model_id);
}

void Perl_Client_SetClientMaxLevel(Client* self, uint8 max_level)
{
	self->SetClientMaxLevel(max_level);
}

int Perl_Client_GetClientMaxLevel(Client* self)
{
	return self->GetClientMaxLevel();
}

void Perl_Client_ShowZoneShardMenu(Client* self) // @categories Script Utility
{
	self->ShowZoneShardMenu();
}

DynamicZoneLocation GetDynamicZoneLocationFromHash(perl::hash table)
{
	// dynamic zone helper method, defaults invalid/missing keys to 0
	perl::scalar zone = table["zone"];
	uint32_t zone_id = zone.is_string() ? ZoneID(zone.c_str()) : zone.as<uint32_t>();

	float x = table.exists("x") ? table["x"] : 0.0f;
	float y = table.exists("y") ? table["y"] : 0.0f;
	float z = table.exists("z") ? table["z"] : 0.0f;
	float h = table.exists("h") ? table["h"] : 0.0f;

	return { zone_id, x, y, z, h };
}

DynamicZone* Perl_Client_CreateExpedition(Client* self, perl::reference table_ref)
{
	perl::hash table      = table_ref;
	perl::hash expedition = table["expedition"];
	perl::hash instance   = table["instance"];

	perl::scalar zone = instance["zone"];
	uint32_t version  = instance["version"];
	uint32_t duration = instance["duration"];
	uint32_t zone_id  = zone.is_string() ? ZoneID(zone.c_str()) : zone.as<uint32_t>();

	DynamicZone dz{ zone_id, version, duration, DynamicZoneType::Expedition };
	dz.SetName(expedition["name"]);
	dz.SetMinPlayers(expedition["min_players"]);
	dz.SetMaxPlayers(expedition["max_players"]);

	if (table.exists("compass"))
	{
		auto compass = GetDynamicZoneLocationFromHash(table["compass"]);
		dz.SetCompass(compass);
	}

	if (table.exists("safereturn"))
	{
		auto safereturn = GetDynamicZoneLocationFromHash(table["safereturn"]);
		dz.SetSafeReturn(safereturn);
	}

	if (table.exists("zonein"))
	{
		auto zonein = GetDynamicZoneLocationFromHash(table["zonein"]);
		dz.SetZoneInLocation(zonein);
	}

	if (table.exists("switchid"))
	{
		dz.SetSwitchID(table["switchid"].as<int>());
	}

	if (expedition.exists("disable_messages"))
	{
		return self->CreateExpedition(dz, expedition["disable_messages"].as<bool>());
	}

	return self->CreateExpedition(dz);
}

DynamicZone* Perl_Client_CreateExpedition(Client* self, std::string zone_name, uint32 version, uint32 duration, std::string expedition_name, uint32 min_players, uint32 max_players)
{
	return self->CreateExpedition(ZoneID(zone_name), version, duration, expedition_name, min_players, max_players);
}

DynamicZone* Perl_Client_CreateExpedition(Client* self, std::string zone_name, uint32 version, uint32 duration, std::string expedition_name, uint32 min_players, uint32 max_players, bool disable_messages)
{
	return self->CreateExpedition(ZoneID(zone_name), version, duration, expedition_name, min_players, max_players, disable_messages);
}

DynamicZone* Perl_Client_CreateExpeditionFromTemplate(Client* self, uint32_t dz_template_id)
{
	return self->CreateExpeditionFromTemplate(dz_template_id);
}

void Perl_Client_CreateTaskDynamicZone(Client* self, int task_id, perl::reference table_ref)
{
	perl::hash table = table_ref;
	perl::hash instance = table["instance"];

	perl::scalar zone = instance["zone"];
	uint32_t version  = instance["version"];
	uint32_t zone_id  = zone.is_string() ? ZoneID(zone.c_str()) : zone.as<uint32_t>();

	// tasks override dz duration so duration is ignored here
	DynamicZone dz{ zone_id, version, 0, DynamicZoneType::None };

	if (table.exists("compass"))
	{
		auto compass = GetDynamicZoneLocationFromHash(table["compass"]);
		dz.SetCompass(compass);
	}

	if (table.exists("safereturn"))
	{
		auto safereturn = GetDynamicZoneLocationFromHash(table["safereturn"]);
		dz.SetSafeReturn(safereturn);
	}

	if (table.exists("zonein"))
	{
		auto zonein = GetDynamicZoneLocationFromHash(table["zonein"]);
		dz.SetZoneInLocation(zonein);
	}

	if (table.exists("switchid"))
	{
		dz.SetSwitchID(table["switchid"].as<int>());
	}

	self->CreateTaskDynamicZone(task_id, dz);
}

DynamicZone* Perl_Client_GetExpedition(Client* self)
{
	return self->GetExpedition();
}

perl::reference Perl_Client_GetExpeditionLockouts(Client* self)
{
	perl::hash lockout_hash;

	const auto& lockouts = self->GetDzLockouts();
	for (const auto& lockout : lockouts)
	{
		if (!lockout_hash.exists(lockout.DzName()))
		{
			lockout_hash[lockout.DzName()] = perl::reference(perl::hash());
		}
		perl::hash events = lockout_hash[lockout.DzName()]; // nested
		events[lockout.Event()] = lockout.GetSecondsRemaining();
	}

	return perl::reference(lockout_hash);
}

perl::reference Perl_Client_GetExpeditionLockouts(Client* self, std::string expedition_name)
{
	perl::hash event_hash;

	auto lockouts = self->GetDzLockouts(expedition_name);
	for (const auto& lockout : lockouts)
	{
		event_hash[lockout.Event()] = lockout.GetSecondsRemaining();
	}

	return perl::reference(event_hash);
}

std::string Perl_Client_GetLockoutExpeditionUUID(Client* self, std::string expedition_name, std::string event_name)
{
	auto lockout = self->GetDzLockout(expedition_name, event_name);
	return lockout ? lockout->UUID() : std::string{};
}

void Perl_Client_AddExpeditionLockout(Client* self, std::string expedition_name, std::string event_name, uint32 seconds)
{
	self->AddDzLockout(expedition_name, event_name, seconds);
}

void Perl_Client_AddExpeditionLockout(Client* self, std::string expedition_name, std::string event_name, uint32 seconds, std::string uuid)
{
	self->AddDzLockout(expedition_name, event_name, seconds, uuid);
}

void Perl_Client_AddExpeditionLockoutDuration(Client* self, std::string expedition_name, std::string event_name, int seconds)
{
	auto lockout = DzLockout::Create(expedition_name, event_name, seconds);
	self->AddDzLockoutDuration(lockout, seconds, {}, true);
}

void Perl_Client_AddExpeditionLockoutDuration(Client* self, std::string expedition_name, std::string event_name, int seconds, std::string uuid)
{
	auto lockout = DzLockout::Create(expedition_name, event_name, seconds, uuid);
	self->AddDzLockoutDuration(lockout, seconds, uuid, true);
}

void Perl_Client_RemoveAllExpeditionLockouts(Client* self)
{
	self->RemoveDzLockouts({}, true);
}

void Perl_Client_RemoveAllExpeditionLockouts(Client* self, std::string expedition_name)
{
	self->RemoveDzLockouts(expedition_name, true);
}

void Perl_Client_RemoveExpeditionLockout(Client* self, std::string expedition_name, std::string event_name)
{
	self->RemoveDzLockout(expedition_name, event_name, true);
}

bool Perl_Client_HasExpeditionLockout(Client* self, std::string expedition_name, std::string event_name)
{
	return self->HasDzLockout(expedition_name, event_name);
}

void Perl_Client_MovePCDynamicZone(Client* self, perl::scalar zone)
{
	uint32_t zone_id = zone.is_string() ? ZoneID(zone.c_str()) : zone.as<uint32_t>();
	self->MovePCDynamicZone(zone_id);
}

void Perl_Client_MovePCDynamicZone(Client* self, perl::scalar zone, int zone_version)
{
	uint32_t zone_id = zone.is_string() ? ZoneID(zone.c_str()) : zone.as<uint32_t>();
	self->MovePCDynamicZone(zone_id, zone_version);
}

void Perl_Client_MovePCDynamicZone(Client* self, perl::scalar zone, int zone_version, bool msg_if_invalid)
{
	uint32_t zone_id = zone.is_string() ? ZoneID(zone.c_str()) : zone.as<uint32_t>();
	self->MovePCDynamicZone(zone_id, zone_version, msg_if_invalid);
}

void Perl_Client_Fling(Client* self, float target_x, float target_y, float target_z)
{
	self->Fling(0, target_x, target_y, target_z, false, false, true);
}

void Perl_Client_Fling(Client* self, float target_x, float target_y, float target_z, bool ignore_los)
{
	self->Fling(0, target_x, target_y, target_z, ignore_los, false, true);
}

void Perl_Client_Fling(Client* self, float target_x, float target_y, float target_z, bool ignore_los, bool clip_through_walls)
{
	self->Fling(0, target_x, target_y, target_z, ignore_los, clip_through_walls, true);
}

void Perl_Client_Fling(Client* self, float value, float target_x, float target_y, float target_z)
{
	self->Fling(value, target_x, target_y, target_z);
}

void Perl_Client_Fling(Client* self, float value, float target_x, float target_y, float target_z, bool ignore_los)
{
	self->Fling(value, target_x, target_y, target_z, ignore_los);
}

void Perl_Client_Fling(Client* self, float value, float target_x, float target_y, float target_z, bool ignore_los, bool clip_through_walls)
{
	self->Fling(value, target_x, target_y, target_z, ignore_los, clip_through_walls);
}

bool Perl_Client_HasDisciplineLearned(Client* self, uint16 spell_id)
{
	return self->HasDisciplineLearned(spell_id);
}

uint16_t Perl_Client_GetClassBitmask(Client* self)
{
	return GetPlayerClassBit(self->GetClass());
}

uint32_t Perl_Client_GetDeityBitmask(Client* self)
{
	return Deity::GetBitmask(self->GetDeity());
}

uint16_t Perl_Client_GetRaceBitmask(Client* self) // @categories Stats and Attributes
{
	return GetPlayerRaceBit(self->GetBaseRace());
}

perl::array Perl_Client_GetLearnableDisciplines(Client* self)
{
	perl::array result;
	auto learnable_disciplines = self->GetLearnableDisciplines();
	for (int i = 0; i < learnable_disciplines.size(); ++i)
	{
		result.push_back(learnable_disciplines[i]);
	}
	return result;
}

perl::array Perl_Client_GetLearnableDisciplines(Client* self, uint8 min_level)
{
	perl::array result;
	auto learnable_disciplines = self->GetLearnableDisciplines(min_level);
	for (int i = 0; i < learnable_disciplines.size(); ++i)
	{
		result.push_back(learnable_disciplines[i]);
	}
	return result;
}

perl::array Perl_Client_GetLearnableDisciplines(Client* self, uint8 min_level, uint8 max_level)
{
	perl::array result;
	auto learnable_disciplines = self->GetLearnableDisciplines(min_level, max_level);
	for (int i = 0; i < learnable_disciplines.size(); ++i)
	{
		result.push_back(learnable_disciplines[i]);
	}
	return result;
}

perl::array Perl_Client_GetLearnedDisciplines(Client* self)
{
	perl::array result;
	auto learned_disciplines = self->GetLearnedDisciplines();
	for (int i = 0; i < learned_disciplines.size(); ++i)
	{
		result.push_back(learned_disciplines[i]);
	}
	return result;
}

perl::array Perl_Client_GetMemmedSpells(Client* self)
{
	perl::array result;
	auto memmed_spells = self->GetMemmedSpells();
	for (int i = 0; i < memmed_spells.size(); ++i)
	{
		result.push_back(memmed_spells[i]);
	}
	return result;
}

perl::array Perl_Client_GetScribeableSpells(Client* self)
{
	perl::array result;
	auto scribeable_spells = self->GetScribeableSpells();
	for (int i = 0; i < scribeable_spells.size(); ++i)
	{
		result.push_back(scribeable_spells[i]);
	}
	return result;
}

perl::array Perl_Client_GetScribeableSpells(Client* self, uint8 min_level)
{
	perl::array result;
	auto scribeable_spells = self->GetScribeableSpells(min_level);
	for (int i = 0; i < scribeable_spells.size(); ++i)
	{
		result.push_back(scribeable_spells[i]);
	}
	return result;
}

perl::array Perl_Client_GetScribeableSpells(Client* self, uint8 min_level, uint8 max_level)
{
	perl::array result;
	auto scribeable_spells = self->GetScribeableSpells(min_level, max_level);
	for (int i = 0; i < scribeable_spells.size(); ++i)
	{
		result.push_back(scribeable_spells[i]);
	}
	return result;
}

perl::array Perl_Client_GetScribedSpells(Client* self)
{
	perl::array result;
	auto scribed_spells = self->GetScribedSpells();
	for (int i = 0; i < scribed_spells.size(); ++i)
	{
		result.push_back(scribed_spells[i]);
	}
	return result;
}

EQ::InventoryProfile* Perl_Client_GetInventory(Client* self)
{
	return &self->GetInv();
}

float Perl_Client_GetAAEXPModifier(Client* self)
{
	return zone->GetAAEXPModifier(self);
}

float Perl_Client_GetAAEXPModifier(Client* self, uint32 zone_id)
{
	return self->GetAAEXPModifier(zone_id);
}

float Perl_Client_GetAAEXPModifier(Client* self, uint32 zone_id, int16 instance_version)
{
	return self->GetAAEXPModifier(zone_id, instance_version);
}

float Perl_Client_GetEXPModifier(Client* self)
{
	return zone->GetEXPModifier(self);
}

float Perl_Client_GetEXPModifier(Client* self, uint32 zone_id)
{
	return self->GetEXPModifier(zone_id);
}

float Perl_Client_GetEXPModifier(Client* self, uint32 zone_id, int16 instance_version)
{
	return self->GetEXPModifier(zone_id, instance_version);
}

void Perl_Client_SetAAEXPModifier(Client* self, float aa_modifier)
{
	zone->SetAAEXPModifier(self, aa_modifier);
}

void Perl_Client_SetAAEXPModifier(Client* self, uint32 zone_id, float aa_modifier)
{
	self->SetAAEXPModifier(zone_id, aa_modifier);
}

void Perl_Client_SetAAEXPModifier(Client* self, uint32 zone_id, float aa_modifier, int16 instance_version)
{
	self->SetAAEXPModifier(zone_id, aa_modifier, instance_version);
}

void Perl_Client_SetEXPModifier(Client* self, float exp_modifier)
{
	zone->SetEXPModifier(self, exp_modifier);
}

void Perl_Client_SetEXPModifier(Client* self, uint32 zone_id, float exp_modifier)
{
	self->SetEXPModifier(zone_id, exp_modifier);
}

void Perl_Client_SetEXPModifier(Client* self, uint32 zone_id, float exp_modifier, int16 instance_version)
{
	self->SetEXPModifier(zone_id, exp_modifier, instance_version);
}

void Perl_Client_AddLDoNLoss(Client* self, uint32 theme_id)
{
	self->UpdateLDoNWinLoss(theme_id);
}

void Perl_Client_AddLDoNWin(Client* self, uint32 theme_id)
{
	self->UpdateLDoNWinLoss(theme_id, true);
}

void Perl_Client_SetHideMe(Client* self, bool hide_me_state)
{
	self->SetHideMe(hide_me_state);
}

void Perl_Client_ResetAllDisciplineTimers(Client* self) // @categories Spells and Disciplines
{
	self->ResetAllDisciplineTimers();
}

void Perl_Client_SendToInstance(Client* self, std::string instance_type, std::string zone_short_name, uint32 instance_version, float x, float y, float z, float heading, std::string instance_identifier, uint32 duration)
{
	self->SendToInstance(instance_type, zone_short_name, instance_version, x, y, z, heading, instance_identifier, duration);
}

uint32 Perl_Client_CountItem(Client* self, uint32 item_id)
{
	return self->CountItem(item_id);
}

void Perl_Client_RemoveItem(Client* self, uint32 item_id) // @categories Spells and Disciplines
{
	self->RemoveItem(item_id);
}

void Perl_Client_RemoveItem(Client* self, uint32 item_id, uint32 quantity) // @categories Spells and Disciplines
{
	self->RemoveItem(item_id, quantity);
}

void Perl_Client_DialogueWindow(Client* self, std::string window_markdown) // @categories Script Utility
{
	DialogueWindow::Render(self, std::move(window_markdown));
}

void Perl_Client_DiaWind(Client* self, std::string window_markdown) // @categories Script Utility
{
	DialogueWindow::Render(self, std::move(window_markdown));
}

int Perl_Client_GetIPExemption(Client* self) // @categories Account and Character
{
	return self->GetIPExemption();
}

std::string Perl_Client_GetIPString(Client* self) // @categories Account and Character
{
	return self->GetIPString();
}

void Perl_Client_SetIPExemption(Client* self, int exemption_amount) // @categories Account and Character
{
	self->SetIPExemption(exemption_amount);
}

void Perl_Client_ReadBookByName(Client* self, std::string book_name, uint8 book_type) // @categories Script Utility
{
	self->ReadBookByName(book_name, book_type);
}

void Perl_Client_UntrainDiscBySpellID(Client* self, uint16 spell_id) // @categories Spells and Disciplines
{
	self->UntrainDiscBySpellID(spell_id);
}

void Perl_Client_UntrainDiscBySpellID(Client* self, uint16 spell_id, bool update_client) // @categories Spells and Disciplines
{
	self->UntrainDiscBySpellID(spell_id, update_client);
}

void Perl_Client_SummonBaggedItems(Client* self, uint32 bag_item_id, perl::reference bag_items_ref) // @categories Inventory and Items, Script Utility
{
	std::vector<LootItem> bagged_items;

	perl::array bag_items = bag_items_ref;
	for (perl::hash bag_item : bag_items) // only works if all elements are hashrefs
	{
		if (bag_item.exists("item_id") && bag_item.exists("charges"))
		{
			LootItem item{};
			item.item_id = bag_item["item_id"];
			item.charges = bag_item["charges"];
			item.attuned = bag_item.exists("attuned") ? bag_item["attuned"] : 0;
			item.aug_1 = bag_item.exists("augment_one") ? bag_item["augment_one"] : 0;
			item.aug_2 = bag_item.exists("augment_two") ? bag_item["augment_two"] : 0;
			item.aug_3 = bag_item.exists("augment_three") ? bag_item["augment_three"] : 0;
			item.aug_4 = bag_item.exists("augment_four") ? bag_item["augment_four"] : 0;
			item.aug_5 = bag_item.exists("augment_five") ? bag_item["augment_five"] : 0;
			item.aug_6 = bag_item.exists("augment_six") ? bag_item["augment_six"] : 0;
			bagged_items.emplace_back(item);
		}
	}

	self->SummonBaggedItems(bag_item_id, bagged_items);
}

void Perl_Client_RemoveLDoNLoss(Client* self, uint32 theme_id)
{
	self->UpdateLDoNWinLoss(theme_id, false, true);
}

void Perl_Client_RemoveLDoNWin(Client* self, uint32 theme_id)
{
	self->UpdateLDoNWinLoss(theme_id, true, true);
}

int Perl_Client_GetFreeDisciplineSlot(Client* self) // @categories Spells and Disciplines
{
	return self->GetNextAvailableDisciplineSlot();
}

int Perl_Client_GetFreeDisciplineSlot(Client* self, int starting_slot) // @categories Spells and Disciplines
{
	return self->GetNextAvailableDisciplineSlot(starting_slot);
}

int Perl_Client_ScribeSpells(Client* self, uint8 min_level, uint8 max_level) // @categories Spells and Disciplines
{
	return self->ScribeSpells(min_level, max_level);
}

int Perl_Client_LearnDisciplines(Client* self, uint8 min_level, uint8 max_level) // @categories Spells and Disciplines
{
	return self->LearnDisciplines(min_level, max_level);
}

void Perl_Client_ResetCastbarCooldownBySlot(Client* self, int slot)
{
	self->ResetCastbarCooldownBySlot(slot);
}

void Perl_Client_ResetAllCastbarCooldowns(Client* self)
{
	self->ResetAllCastbarCooldowns();
}

void Perl_Client_ResetCastbarCooldownBySpellID(Client* self, uint32 spell_id)
{
	self->ResetCastbarCooldownBySpellID(spell_id);
}

void Perl_Client_UnscribeSpellBySpellID(Client* self, uint16 spell_id)
{
	self->UnscribeSpellBySpellID(spell_id);
}

void Perl_Client_UnscribeSpellBySpellID(Client* self, uint16 spell_id, bool update_client)
{
	self->UnscribeSpellBySpellID(spell_id, update_client);
}

int Perl_Client_GetEnvironmentDamageModifier(Client* self) // @categories Script Utility
{
	return self->GetEnvironmentDamageModifier();
}

void Perl_Client_SetEnvironmentDamageModifier(Client* self, int modifier) // @categories Script Utility
{
	self->SetEnvironmentDamageModifier(modifier);
}

bool Perl_Client_GetInvulnerableEnvironmentDamage(Client* self) // @categories Script Utility
{
	return self->GetInvulnerableEnvironmentDamage();
}

void Perl_Client_SetInvulnerableEnvironmentDamage(Client* self, bool invul) // @categories Script Utility
{
	self->SetInvulnerableEnvironmentDamage(invul);
}

void Perl_Client_AddItem(Client* self, perl::reference table_ref)
{
	perl::hash table = table_ref;
	if (!table.exists("item_id") || !table.exists("charges"))
	{
		return;
	}

	uint32 item_id       = table["item_id"];
	int16 charges        = table["charges"];
	uint32 augment_one   = table.exists("augment_one") ? table["augment_one"] : 0;
	uint32 augment_two   = table.exists("augment_two") ? table["augment_two"] : 0;
	uint32 augment_three = table.exists("augment_three") ? table["augment_three"] : 0;
	uint32 augment_four  = table.exists("augment_four") ? table["augment_four"] : 0;
	uint32 augment_five  = table.exists("augment_five") ? table["augment_five"] : 0;
	uint32 augment_six   = table.exists("augment_six") ? table["augment_six"] : 0;
	bool attuned         = table.exists("attuned") ? table["attuned"] : false;
	uint16 slot_id       = table.exists("slot_id") ? table["slot_id"] : EQ::invslot::slotCursor;

	self->SummonItem(item_id, charges, augment_one, augment_two, augment_three,
		augment_four, augment_five, augment_six, attuned, slot_id);
}

uint32 Perl_Client_CountAugmentEquippedByID(Client* self, uint32 item_id)
{
	return self->GetInv().CountAugmentEquippedByID(item_id);
}

bool Perl_Client_HasAugmentEquippedByID(Client* self, uint32 item_id)
{
	return self->GetInv().HasAugmentEquippedByID(item_id);
}

uint32 Perl_Client_CountItemEquippedByID(Client* self, uint32 item_id)
{
	return self->GetInv().CountItemEquippedByID(item_id);
}

bool Perl_Client_HasItemEquippedByID(Client* self, uint32 item_id)
{
	return self->GetInv().HasItemEquippedByID(item_id);
}

void Perl_Client_AddPlatinum(Client* self, uint32 platinum)
{
	self->AddPlatinum(platinum);
}

void Perl_Client_AddPlatinum(Client* self, uint32 platinum, bool update_client)
{
	self->AddPlatinum(platinum, update_client);
}

uint32 Perl_Client_GetCarriedPlatinum(Client* self)
{
	return self->GetCarriedPlatinum();
}

bool Perl_Client_TakePlatinum(Client* self, uint32 platinum)
{
	return self->TakePlatinum(platinum);
}

bool Perl_Client_TakePlatinum(Client* self, uint32 platinum, bool update_client)
{
	return self->TakePlatinum(platinum, update_client);
}

void Perl_Client_ClearPEQZoneFlag(Client* self, uint32 zone_id)
{
	self->ClearPEQZoneFlag(zone_id);
}

bool Perl_Client_HasPEQZoneFlag(Client* self, uint32 zone_id)
{
	return self->HasPEQZoneFlag(zone_id);
}

void Perl_Client_LoadPEQZoneFlags(Client* self)
{
	self->LoadPEQZoneFlags();
}

void Perl_Client_SendPEQZoneFlagInfo(Client* self, Client* to)
{
	self->SendPEQZoneFlagInfo(to);
}

void Perl_Client_SetPEQZoneFlag(Client* self, uint32 zone_id)
{
	self->SetPEQZoneFlag(zone_id);
}

int Perl_Client_GetHealAmount(Client* self)
{
	return self->GetHealAmt();
}

int Perl_Client_GetSpellDamage(Client* self)
{
	return self->GetSpellDmg();
}

void Perl_Client_TaskSelector(Client* self, perl::array task_ids)
{
	std::vector<int> tasks;
	for (int i = 0; i < task_ids.size() && i < MAXCHOOSERENTRIES; ++i)
	{
		tasks.push_back(task_ids[i]);
	}

	self->TaskQuestSetSelector(self, tasks, false);
}

void Perl_Client_TaskSelectorNoCooldown(Client* self, perl::array task_ids)
{
	std::vector<int> tasks;
	for (int i = 0; i < task_ids.size() && i < MAXCHOOSERENTRIES; ++i)
	{
		tasks.push_back(task_ids[i]);
	}

	self->TaskQuestSetSelector(self, tasks, true);
}

bool Perl_Client_TeleportToPlayerByCharacterID(Client* self, uint32 character_id)
{
	return self->GotoPlayer(database.GetCharNameByID(character_id));
}

bool Perl_Client_TeleportToPlayerByName(Client* self, std::string player_name)
{
	return self->GotoPlayer(player_name);
}

bool Perl_Client_TeleportGroupToPlayerByCharacterID(Client* self, uint32 character_id)
{
	return self->GotoPlayerGroup(database.GetCharNameByID(character_id));
}

bool Perl_Client_TeleportGroupToPlayerByName(Client* self, std::string player_name)
{
	return self->GotoPlayerGroup(player_name);
}

bool Perl_Client_TeleportRaidToPlayerByCharacterID(Client* self, uint32 character_id)
{
	return self->GotoPlayerRaid(database.GetCharNameByID(character_id));
}

bool Perl_Client_TeleportRaidToPlayerByName(Client* self, std::string player_name)
{
	return self->GotoPlayerRaid(player_name);
}

int Perl_Client_GetRecipeMadeCount(Client* self, uint32 recipe_id) // @categories Skills and Recipes
{
	return self->GetRecipeMadeCount(recipe_id);
}

bool Perl_Client_HasRecipeLearned(Client* self, uint32 recipe_id) // @categories Skills and Recipes
{
	return self->HasRecipeLearned(recipe_id);
}

bool Perl_Client_SendGMCommand(Client* self, std::string message) // @categories Script Utility
{
	return self->SendGMCommand(message);
}

bool Perl_Client_SendGMCommand(Client* self, std::string message, bool ignore_status) // @categories Script Utility
{
	return self->SendGMCommand(message, ignore_status);
}

void Perl_Client_SendMarqueeMessage(Client* self, uint32 type, std::string message) // @categories Script Utility
{
	self->SendMarqueeMessage(type, message);
}

void Perl_Client_SendMarqueeMessage(Client* self, uint32 type, std::string message, uint32 duration) // @categories Script Utility
{
	self->SendMarqueeMessage(type, message, duration);
}

void Perl_Client_SendMarqueeMessage(Client* self, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, std::string message) // @categories Script Utility
{
	self->SendMarqueeMessage(type, priority, fade_in, fade_out, duration, message);
}

void Perl_Client_MoveZone(Client* self, const char* zone_short_name) // @categories Script Utility
{
	self->MoveZone(zone_short_name);
}

void Perl_Client_MoveZone(Client* self, const char* zone_short_name, float x, float y, float z) // @categories Script Utility
{
	self->MoveZone(zone_short_name, glm::vec4(x, y, z, 0.0f));
}

void Perl_Client_MoveZone(Client* self, const char* zone_short_name, float x, float y, float z, float heading) // @categories Script Utility
{
	self->MoveZone(zone_short_name, glm::vec4(x, y, z, heading));
}

void Perl_Client_MoveZoneGroup(Client* self, const char* zone_short_name) // @categories Script Utility, Group
{
	self->MoveZoneGroup(zone_short_name);
}

void Perl_Client_MoveZoneGroup(Client* self, const char* zone_short_name, float x, float y, float z) // @categories Script Utility
{
	self->MoveZoneGroup(zone_short_name, glm::vec4(x, y, z, 0.0f));
}

void Perl_Client_MoveZoneGroup(Client* self, const char* zone_short_name, float x, float y, float z, float heading) // @categories Script Utility
{
	self->MoveZoneGroup(zone_short_name, glm::vec4(x, y, z, heading));
}

void Perl_Client_MoveZoneRaid(Client* self, const char* zone_short_name) // @categories Script Utility, Raid
{
	self->MoveZoneRaid(zone_short_name);
}

void Perl_Client_MoveZoneRaid(Client* self, const char* zone_short_name, float x, float y, float z) // @categories Script Utility
{
	self->MoveZoneRaid(zone_short_name, glm::vec4(x, y, z, 0.0f));
}

void Perl_Client_MoveZoneRaid(Client* self, const char* zone_short_name, float x, float y, float z, float heading) // @categories Script Utility
{
	self->MoveZoneRaid(zone_short_name, glm::vec4(x, y, z, heading));
}

void Perl_Client_MoveZoneInstance(Client* self, uint16 instance_id) // @categories Adventures and Expeditions, Script Utility
{
	self->MoveZoneInstance(instance_id);
}

void Perl_Client_MoveZoneInstance(Client* self, uint16 instance_id, float x, float y, float z) // @categories Adventures and Expeditions, Script Utility
{
	self->MoveZoneInstance(instance_id, glm::vec4(x, y, z, 0.0f));
}

void Perl_Client_MoveZoneInstance(Client* self, uint16 instance_id, float x, float y, float z, float heading) // @categories Adventures and Expeditions, Script Utility
{
	self->MoveZoneInstance(instance_id, glm::vec4(x, y, z, heading));
}

void Perl_Client_MoveZoneInstanceGroup(Client* self, uint16 instance_id) // @categories Adventures and Expeditions, Script Utility, Group
{
	self->MoveZoneInstanceGroup(instance_id);
}

void Perl_Client_MoveZoneInstanceGroup(Client* self, uint16 instance_id, float x, float y, float z) // @categories Adventures and Expeditions, Script Utility
{
	self->MoveZoneInstanceGroup(instance_id, glm::vec4(x, y, z, 0.0f));
}

void Perl_Client_MoveZoneInstanceGroup(Client* self, uint16 instance_id, float x, float y, float z, float heading) // @categories Adventures and Expeditions, Script Utility
{
	self->MoveZoneInstanceGroup(instance_id, glm::vec4(x, y, z, heading));
}

void Perl_Client_MoveZoneInstanceRaid(Client* self, uint16 instance_id) // @categories Adventures and Expeditions, Script Utility, Raid
{
	self->MoveZoneInstanceRaid(instance_id);
}

void Perl_Client_MoveZoneInstanceRaid(Client* self, uint16 instance_id, float x, float y, float z) // @categories Adventures and Expeditions, Script Utility
{
	self->MoveZoneInstanceRaid(instance_id, glm::vec4(x, y, z, 0.0f));
}

void Perl_Client_MoveZoneInstanceRaid(Client* self, uint16 instance_id, float x, float y, float z, float heading) // @categories Adventures and Expeditions, Script Utility
{
	self->MoveZoneInstanceRaid(instance_id, glm::vec4(x, y, z, heading));
}

void Perl_Client_ApplySpell(Client* self, int spell_id)
{
	self->ApplySpell(spell_id);
}

void Perl_Client_ApplySpell(Client* self, int spell_id, int duration)
{
	self->ApplySpell(spell_id, duration);
}

void Perl_Client_ApplySpell(Client* self, int spell_id, int duration, int level)
{
	self->ApplySpell(spell_id, duration, level);
}

void Perl_Client_ApplySpell(Client* self, int spell_id, int duration, int level, bool allow_pets)
{
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Solo, allow_pets);
}

void Perl_Client_ApplySpell(Client* self, int spell_id, int duration, int level, bool allow_pets, bool allow_bots)
{
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Solo, allow_pets, true, allow_bots);
}

void Perl_Client_ApplySpellGroup(Client* self, int spell_id)
{
	self->ApplySpell(spell_id, 0, -1, ApplySpellType::Group);
}

void Perl_Client_ApplySpellGroup(Client* self, int spell_id, int duration)
{
	self->ApplySpell(spell_id, duration, -1, ApplySpellType::Group);
}

void Perl_Client_ApplySpellGroup(Client* self, int spell_id, int duration, int level)
{
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Group);
}

void Perl_Client_ApplySpellGroup(Client* self, int spell_id, int duration, int level, bool allow_pets)
{
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Group, allow_pets);
}

void Perl_Client_ApplySpellGroup(Client* self, int spell_id, int duration, int level, bool allow_pets, bool allow_bots)
{
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Group, allow_pets, true, allow_bots);
}

void Perl_Client_ApplySpellRaid(Client* self, int spell_id)
{
	self->ApplySpell(spell_id, 0, -1, ApplySpellType::Raid);
}

void Perl_Client_ApplySpellRaid(Client* self, int spell_id, int duration)
{
	self->ApplySpell(spell_id, duration, -1, ApplySpellType::Raid);
}

void Perl_Client_ApplySpellRaid(Client* self, int spell_id, int duration, int level)
{
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Raid);
}

void Perl_Client_ApplySpellRaid(Client* self, int spell_id, int duration, int level, bool allow_pets)
{
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Raid, allow_pets);
}

void Perl_Client_ApplySpellRaid(Client* self, int spell_id, int duration, int level, bool allow_pets, bool is_raid_group_only)
{
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Raid, allow_pets, is_raid_group_only);
}

void Perl_Client_ApplySpellRaid(Client* self, int spell_id, int duration, int level, bool allow_pets, bool is_raid_group_only, bool allow_bots)
{
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Raid, allow_pets, is_raid_group_only, allow_bots);
}

void Perl_Client_SetSpellDuration(Client* self, int spell_id)
{
	self->SetSpellDuration(spell_id);
}

void Perl_Client_SetSpellDuration(Client* self, int spell_id, int duration)
{
	self->SetSpellDuration(spell_id, duration);
}

void Perl_Client_SetSpellDuration(Client* self, int spell_id, int duration, int level)
{
	self->SetSpellDuration(spell_id, duration, level);
}

void Perl_Client_SetSpellDuration(Client* self, int spell_id, int duration, int level, bool allow_pets)
{
	self->SetSpellDuration(spell_id, duration, level, ApplySpellType::Solo, allow_pets);
}

void Perl_Client_SetSpellDuration(Client* self, int spell_id, int duration, int level, bool allow_pets, bool allow_bots)
{
	self->SetSpellDuration(spell_id, duration, level, ApplySpellType::Solo, allow_pets, true, allow_bots);
}

void Perl_Client_SetSpellDurationGroup(Client* self, int spell_id)
{
	self->SetSpellDuration(spell_id, 0, -1, ApplySpellType::Group);
}

void Perl_Client_SetSpellDurationGroup(Client* self, int spell_id, int duration)
{
	self->SetSpellDuration(spell_id, duration, -1, ApplySpellType::Group);
}

void Perl_Client_SetSpellDurationGroup(Client* self, int spell_id, int duration, int level)
{
	self->SetSpellDuration(spell_id, duration, level, ApplySpellType::Group);
}

void Perl_Client_SetSpellDurationGroup(Client* self, int spell_id, int duration, int level, bool allow_pets)
{
	self->SetSpellDuration(spell_id, duration, level, ApplySpellType::Group, allow_pets);
}

void Perl_Client_SetSpellDurationGroup(Client* self, int spell_id, int duration, int level, bool allow_pets, bool allow_bots)
{
	self->SetSpellDuration(spell_id, duration, level, ApplySpellType::Group, allow_pets, true, allow_bots);
}

void Perl_Client_SetSpellDurationRaid(Client* self, int spell_id)
{
	self->ApplySpell(spell_id, 0, -1, ApplySpellType::Raid);
}

void Perl_Client_SetSpellDurationRaid(Client* self, int spell_id, int duration)
{
	self->ApplySpell(spell_id, duration, -1, ApplySpellType::Raid);
}

void Perl_Client_SetSpellDurationRaid(Client* self, int spell_id, int duration, int level)
{
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Raid);
}

void Perl_Client_SetSpellDurationRaid(Client* self, int spell_id, int duration, int level, bool allow_pets)
{
	self->SetSpellDuration(spell_id, duration, level, ApplySpellType::Raid, allow_pets);
}

void Perl_Client_SetSpellDurationRaid(Client* self, int spell_id, int duration, int level, bool allow_pets, bool is_raid_group_only)
{
	self->SetSpellDuration(spell_id, duration, level, ApplySpellType::Raid, allow_pets, is_raid_group_only);
}

void Perl_Client_SetSpellDurationRaid(Client* self, int spell_id, int duration, int level, bool allow_pets, bool is_raid_group_only, bool allow_bots)
{
	self->SetSpellDuration(spell_id, duration, level, ApplySpellType::Raid, allow_pets, is_raid_group_only, allow_bots);
}

perl::array Perl_Client_GetPEQZoneFlags(Client* self)
{
	perl::array a;

	auto l = self->GetPEQZoneFlags();
	for (const auto& f : l) {
		a.push_back(f);
	}

	return a;
}

perl::array Perl_Client_GetZoneFlags(Client* self)
{
	perl::array a;

	auto l = self->GetZoneFlags();
	for (const auto& f : l) {
		a.push_back(f);
	}

	return a;
}

void Perl_Client_SendPayload(Client* self, int payload_id) // @categories Script Utility
{
	self->SendPayload(payload_id);
}

void Perl_Client_SendPayload(Client* self, int payload_id, std::string payload_value) // @categories Script Utility
{
	self->SendPayload(payload_id, payload_value);
}

void Perl_Client_Signal(Client* self, int signal_id)
{
	self->Signal(signal_id);
}

void Perl_Client_SignalClient(Client* self, int signal_id) // @categories Script Utility
{
	self->Signal(signal_id);
}

std::string Perl_Client_GetGuildPublicNote(Client* self)
{
	return self->GetGuildPublicNote();
}

void Perl_Client_MaxSkills(Client* self)
{
	self->MaxSkills();
}

perl::array Perl_Client_GetAugmentIDsBySlotID(Client* self, int16 slot_id)
{
	perl::array result;
	auto augments = self->GetInv().GetAugmentIDsBySlotID(slot_id);

	for (int i = 0; i < augments.size(); ++i) {
		result.push_back(augments[i]);
	}

	return result;
}

bool Perl_Client_IsEXPEnabled(Client* self)
{
	return self->IsEXPEnabled();
}

void Perl_Client_SetEXPEnabled(Client* self, bool is_exp_enabled)
{
	self->SetEXPEnabled(is_exp_enabled);
}

bool Perl_Client_CanEnterZone(Client* self, std::string zone_short_name)
{
	return self->CanEnterZone(zone_short_name);
}

bool Perl_Client_CanEnterZone(Client* self, std::string zone_short_name, int16 instance_version)
{
	return self->CanEnterZone(zone_short_name, instance_version);
}

void Perl_Client_SendPath(Client* self, Mob* target)
{
	self->SendPath(target);
}

int Perl_Client_GetBotRequiredLevel(Client* self)
{
	return self->GetBotRequiredLevel();
}

int Perl_Client_GetBotRequiredLevel(Client* self, uint8 class_id)
{
	return self->GetBotRequiredLevel(class_id);
}

uint32 Perl_Client_GetBotCreationLimit(Client* self)
{
	return self->GetBotCreationLimit();
}

uint32 Perl_Client_GetBotCreationLimit(Client* self, uint8 class_id)
{
	return self->GetBotCreationLimit(class_id);
}

int Perl_Client_GetBotSpawnLimit(Client* self)
{
	return self->GetBotSpawnLimit();
}

int Perl_Client_GetBotSpawnLimit(Client* self, uint8 class_id)
{
	return self->GetBotSpawnLimit(class_id);
}

void Perl_Client_SetBotRequiredLevel(Client* self, int new_required_level)
{
	self->SetBotRequiredLevel(new_required_level);
}

void Perl_Client_SetBotRequiredLevel(Client* self, int new_required_level, uint8 class_id)
{
	self->SetBotRequiredLevel(new_required_level, class_id);
}

void Perl_Client_SetBotCreationLimit(Client* self, uint32 new_creation_limit)
{
	self->SetBotCreationLimit(new_creation_limit);
}

void Perl_Client_SetBotCreationLimit(Client* self, uint32 new_creation_limit, uint8 class_id)
{
	self->SetBotCreationLimit(new_creation_limit, class_id);
}

void Perl_Client_SetBotSpawnLimit(Client* self, int new_spawn_limit)
{
	self->SetBotSpawnLimit(new_spawn_limit);
}

void Perl_Client_SetBotSpawnLimit(Client* self, int new_spawn_limit, uint8 class_id)
{
	self->SetBotSpawnLimit(new_spawn_limit, class_id);
}

void Perl_Client_CampAllBots(Client* self)
{
	self->CampAllBots();
}

void Perl_Client_CampAllBots(Client* self, uint8 class_id)
{
	self->CampAllBots(class_id);
}

void Perl_Client_ResetItemCooldown(Client* self, uint32 item_id)
{
	self->ResetItemCooldown(item_id);
}

void Perl_Client_SetItemCooldown(Client* self, uint32 item_id, uint32 in_time)
{
	self->SetItemCooldown(item_id, false, in_time);
}

uint32 Perl_Client_GetItemCooldown(Client* self, uint32 item_id)
{
	return self->GetItemCooldown(item_id);
}

void Perl_Client_UseAugmentContainer(Client* self, int container_slot)
{
	self->UseAugmentContainer(container_slot);
}

bool Perl_Client_IsAutoAttackEnabled(Client* self)
{
	return self->AutoAttackEnabled();
}

bool Perl_Client_IsAutoFireEnabled(Client* self)
{
	return self->AutoFireEnabled();
}

bool Perl_Client_ReloadDataBuckets(Client* self)
{
	return self->LoadDataBucketsCache();
}

uint32 Perl_Client_GetEXPForLevel(Client* self, uint16 check_level)
{
	return self->GetEXPForLevel(check_level);
}

std::string Perl_Client_GetClassAbbreviation(Client* self)
{
	return GetPlayerClassAbbreviation(self->GetBaseClass());
}

std::string Perl_Client_GetRaceAbbreviation(Client* self)
{
	return GetPlayerRaceAbbreviation(self->GetBaseRace());
}

void Perl_Client_SetLDoNPoints(Client* self, uint32 theme_id, uint32 points)
{
	self->SetLDoNPoints(theme_id, points);
}

void Perl_Client_GrantAllAAPoints(Client* self)
{
	self->GrantAllAAPoints();
}

void Perl_Client_GrantAllAAPoints(Client* self, uint8 unlock_level)
{
	self->GrantAllAAPoints(unlock_level);
}

void Perl_Client_GrantAllAAPoints(Client* self, uint8 unlock_level, bool skip_grant_only)
{
	self->GrantAllAAPoints(unlock_level, skip_grant_only);
}

void Perl_Client_AddEbonCrystals(Client* self, uint32 amount)
{
	self->AddEbonCrystals(amount);
}

void Perl_Client_AddRadiantCrystals(Client* self, uint32 amount)
{
	self->AddRadiantCrystals(amount);
}

void Perl_Client_RemoveEbonCrystals(Client* self, uint32 amount)
{
	self->RemoveEbonCrystals(amount);
}

void Perl_Client_RemoveRadiantCrystals(Client* self, uint32 amount)
{
	self->RemoveRadiantCrystals(amount);
}

void Perl_Client_SummonItemIntoInventory(Client* self, perl::reference table_ref)
{
	perl::hash table = table_ref;
	if (!table.exists("item_id") || !table.exists("charges")) {
		return;
	}

	const uint32 item_id       = table["item_id"];
	const int16 charges        = table["charges"];
	const uint32 augment_one   = table.exists("augment_one") ? table["augment_one"] : 0;
	const uint32 augment_two   = table.exists("augment_two") ? table["augment_two"] : 0;
	const uint32 augment_three = table.exists("augment_three") ? table["augment_three"] : 0;
	const uint32 augment_four  = table.exists("augment_four") ? table["augment_four"] : 0;
	const uint32 augment_five  = table.exists("augment_five") ? table["augment_five"] : 0;
	const uint32 augment_six   = table.exists("augment_six") ? table["augment_six"] : 0;
	const bool attuned         = table.exists("attuned") ? table["attuned"] : false;

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

bool Perl_Client_HasItemOnCorpse(Client* self, uint32 item_id)
{
	return self->HasItemOnCorpse(item_id);
}

void Perl_Client_ClearXTargets(Client* self)
{
	self->ClearXTargets();
}

int Perl_Client_GetAAEXPPercentage(Client* self)
{
	return self->GetAAEXPPercentage();
}

int Perl_Client_GetEXPPercentage(Client* self)
{
	return self->GetEXPPercentage();
}

bool Perl_Client_IsInAGuild(Client* self)
{
	return self->IsInAGuild();
}

bool Perl_Client_RemoveAAPoints(Client* self, uint32 points)
{
	return self->RemoveAAPoints(points);
}

bool Perl_Client_RemoveAlternateCurrencyValue(Client* self, uint32 currency_id, uint32 amount)
{
	return self->RemoveAlternateCurrencyValue(currency_id, amount);
}

perl::array Perl_Client_GetRaidOrGroupOrSelf(Client* self)
{
	perl::array result;

	const auto& l = self->GetRaidOrGroupOrSelf();

	result.reserve(l.size());

	for (const auto& e : l) {
		result.push_back(e);
	}

	return result;
}

perl::array Perl_Client_GetRaidOrGroupOrSelf(Client* self, bool clients_only)
{
	perl::array result;

	const auto& l = self->GetRaidOrGroupOrSelf(clients_only);

	result.reserve(l.size());

	for (const auto& e : l) {
		result.push_back(e);
	}

	return result;
}

std::string Perl_Client_GetAutoLoginCharacterName(Client* self)
{
	return quest_manager.GetAutoLoginCharacterNameByAccountID(self->AccountID());
}

bool Perl_Client_SetAutoLoginCharacterName(Client* self)
{
	return quest_manager.SetAutoLoginCharacterNameByAccountID(self->AccountID(), self->GetCleanName());
}

bool Perl_Client_SetAutoLoginCharacterName(Client* self, std::string character_name)
{
	return quest_manager.SetAutoLoginCharacterNameByAccountID(self->AccountID(), character_name);
}

void Perl_Client_DescribeSpecialAbilities(Client* self, NPC* n)
{
	n->DescribeSpecialAbilities(self);
}

void Perl_Client_ResetLeadershipAA(Client* self)
{
	self->ResetLeadershipAA();
}

uint8 Perl_Client_GetSkillTrainLevel(Client* self, int skill_id)
{
	return self->GetSkillTrainLevel(static_cast<EQ::skills::SkillType>(skill_id), self->GetClass());
}

bool Perl_Client_AreTasksCompleted(Client* self, perl::array task_ids)
{
	std::vector<int> v;

	for (const auto& e : task_ids) {
		v.push_back(static_cast<int>(e));
	}

	return self->AreTasksCompleted(v);
}

void Perl_Client_AreaTaunt(Client* self)
{
	entity_list.AETaunt(self);
}

void Perl_Client_AreaTaunt(Client* self, float range)
{
	entity_list.AETaunt(self, range);
}

void Perl_Client_AreaTaunt(Client* self, float range, int bonus_hate)
{
	entity_list.AETaunt(self, range, bonus_hate);
}

Merc* Perl_Client_GetMerc(Client* self)
{
	return self->GetMerc();
}

perl::array Perl_Client_GetInventorySlots(Client* self)
{
	perl::array result;
	const auto& v = self->GetInventorySlots();

	for (int i = 0; i < v.size(); ++i) {
		result.push_back(v[i]);
	}

	return result;
}

void Perl_Client_GrantPetNameChange(Client* self)
{
	self->GrantPetNameChange();
}

void Perl_Client_SetAAEXPPercentage(Client* self, uint8 percentage)
{
	self->SetAAEXPPercentage(percentage);
}

void Perl_Client_SetAccountBucket(Client* self, std::string bucket_name, std::string bucket_value)
{
	self->SetAccountBucket(bucket_name, bucket_value);
}

void Perl_Client_SetAccountBucket(Client* self, std::string bucket_name, std::string bucket_value, std::string expiration = "")
{
	self->SetAccountBucket(bucket_name, bucket_value, expiration);
}

void Perl_Client_DeleteAccountBucket(Client* self, std::string bucket_name)
{
	self->DeleteAccountBucket(bucket_name);
}

std::string Perl_Client_GetAccountBucket(Client* self, std::string bucket_name)
{
	return self->GetAccountBucket(bucket_name);
}

std::string Perl_Client_GetAccountBucketExpires(Client* self, std::string bucket_name)
{
	return self->GetAccountBucketExpires(bucket_name);
}

std::string Perl_Client_GetAccountBucketRemaining(Client* self, std::string bucket_name)
{
	return self->GetAccountBucketRemaining(bucket_name);
}

void Perl_Client_GrantNameChange(Client* self)
{
	self->GrantNameChange();
}

bool Perl_Client_IsNameChangeAllowed(Client* self)
{
	return self->IsNameChangeAllowed();
}

bool Perl_Client_ClearNameChange(Client* self)
{
	return self->ClearNameChange();
}

std::string Perl_Client_GetBandolierName(Client* self, uint8 bandolier_slot)
{
	return self->GetBandolierName(bandolier_slot);
}

uint32 Perl_Client_GetBandolierItemIcon(Client* self, uint8 bandolier_slot, uint8 slot_id)
{
	return self->GetBandolierItemIcon(bandolier_slot, slot_id);
}

uint32 Perl_Client_GetBandolierItemID(Client* self, uint8 bandolier_slot, uint8 slot_id)
{
	return self->GetBandolierItemID(bandolier_slot, slot_id);
}

std::string Perl_Client_GetBandolierItemName(Client* self, uint8 bandolier_slot, uint8 slot_id)
{
	return self->GetBandolierItemName(bandolier_slot, slot_id);
}

uint32 Perl_Client_GetPotionBeltItemIcon(Client* self, uint8 slot_id)
{
	return self->GetPotionBeltItemIcon(slot_id);
}

uint32 Perl_Client_GetPotionBeltItemID(Client* self, uint8 slot_id)
{
	return self->GetPotionBeltItemID(slot_id);
}

std::string Perl_Client_GetPotionBeltItemName(Client* self, uint8 slot_id)
{
	return self->GetPotionBeltItemName(slot_id);
}

bool Perl_Client_KeyRingAdd(Client* self, uint32 item_id) // @categories Account and Character, Inventory and Items
{
	return self->KeyRingAdd(item_id);
}

bool Perl_Client_KeyRingCheck(Client* self, uint32 item_id) // @categories Account and Character, Inventory and Items
{
	return self->KeyRingCheck(item_id);
}

bool Perl_Client_KeyRingClear(Client* self)
{
	return self->KeyRingClear();
}

void Perl_Client_KeyRingList(Client* self)
{
	self->KeyRingList(self);
}

void Perl_Client_KeyRingList(Client* self, Client* c)
{
	self->KeyRingList(c);
}

bool Perl_Client_KeyRingRemove(Client* self, uint32 item_id)
{
	return self->KeyRingRemove(item_id);
}

bool Perl_Client_CompleteTask(Client* self, int task_id)
{
	return self->CompleteTask(task_id);
}

bool Perl_Client_UncompleteTask(Client* self, int task_id)
{
	return self->UncompleteTask(task_id);
}

perl::array Perl_Client_GetKeyRing(Client* self)
{
	perl::array result;
	const auto& v = self->GetKeyRing();

	for (int i = 0; i < v.size(); ++i) {
		result.push_back(v[i]);
	}

	return result;
}

void perl_register_client()
{
	perl::interpreter perl(PERL_GET_THX);

	auto package = perl.new_class<Client>("Client");
	package.add_base_class("Mob");
	package.add("AccountID", &Perl_Client_AccountID);
	package.add("AccountName", &Perl_Client_AccountName);
	package.add("AddAAPoints", &Perl_Client_AddAAPoints);
	package.add("AddAlternateCurrencyValue", &Perl_Client_AddAlternateCurrencyValue);
	package.add("AddCrystals", &Perl_Client_AddCrystals);
	package.add("AddEXP", (void(*)(Client*, uint32))&Perl_Client_AddEXP);
	package.add("AddEXP", (void(*)(Client*, uint32, uint8))&Perl_Client_AddEXP);
	package.add("AddEXP", (void(*)(Client*, uint32, uint8, bool))&Perl_Client_AddEXP);
	package.add("AddEbonCrystals", &Perl_Client_AddEbonCrystals);
	package.add("AddExpeditionLockout", (void(*)(Client*, std::string, std::string, uint32))&Perl_Client_AddExpeditionLockout);
	package.add("AddExpeditionLockout", (void(*)(Client*, std::string, std::string, uint32, std::string))&Perl_Client_AddExpeditionLockout);
	package.add("AddExpeditionLockoutDuration", (void(*)(Client*, std::string, std::string, int))&Perl_Client_AddExpeditionLockoutDuration);
	package.add("AddExpeditionLockoutDuration", (void(*)(Client*, std::string, std::string, int, std::string))&Perl_Client_AddExpeditionLockoutDuration);
	package.add("AddItem", &Perl_Client_AddItem);
	package.add("AddLDoNLoss", &Perl_Client_AddLDoNLoss);
	package.add("AddLDoNWin", &Perl_Client_AddLDoNWin);
	package.add("AddLevelBasedExp", (void(*)(Client*, uint8))&Perl_Client_AddLevelBasedExp);
	package.add("AddLevelBasedExp", (void(*)(Client*, uint8, uint8))&Perl_Client_AddLevelBasedExp);
	package.add("AddLevelBasedExp", (void(*)(Client*, uint8, uint8, bool))&Perl_Client_AddLevelBasedExp);
	package.add("AddMoneyToPP", (void(*)(Client*, uint32, uint32, uint32, uint32))&Perl_Client_AddMoneyToPP);
	package.add("AddMoneyToPP", (void(*)(Client*, uint32, uint32, uint32, uint32, bool))&Perl_Client_AddMoneyToPP);
	package.add("AddPlatinum", (void(*)(Client*, uint32))&Perl_Client_AddPlatinum);
	package.add("AddPlatinum", (void(*)(Client*, uint32, bool))&Perl_Client_AddPlatinum);
	package.add("AddPVPPoints", &Perl_Client_AddPVPPoints);
	package.add("AddRadiantCrystals", &Perl_Client_AddRadiantCrystals);
	package.add("AddSkill", &Perl_Client_AddSkill);
	package.add("Admin", &Perl_Client_Admin);
	package.add("ApplySpell", (void(*)(Client*, int))&Perl_Client_ApplySpell);
	package.add("ApplySpell", (void(*)(Client*, int, int))&Perl_Client_ApplySpell);
	package.add("ApplySpell", (void(*)(Client*, int, int, int))&Perl_Client_ApplySpell);
	package.add("ApplySpell", (void(*)(Client*, int, int, int, bool))&Perl_Client_ApplySpell);
	package.add("ApplySpell", (void(*)(Client*, int, int, int, bool, bool))&Perl_Client_ApplySpell);
	package.add("ApplySpellGroup", (void(*)(Client*, int))&Perl_Client_ApplySpellGroup);
	package.add("ApplySpellGroup", (void(*)(Client*, int, int))&Perl_Client_ApplySpellGroup);
	package.add("ApplySpellGroup", (void(*)(Client*, int, int, int))&Perl_Client_ApplySpellGroup);
	package.add("ApplySpellGroup", (void(*)(Client*, int, int, int, bool))&Perl_Client_ApplySpellGroup);
	package.add("ApplySpellGroup", (void(*)(Client*, int, int, int, bool, bool))&Perl_Client_ApplySpellGroup);
	package.add("ApplySpellRaid", (void(*)(Client*, int))&Perl_Client_ApplySpellRaid);
	package.add("ApplySpellRaid", (void(*)(Client*, int, int))&Perl_Client_ApplySpellRaid);
	package.add("ApplySpellRaid", (void(*)(Client*, int, int, int))&Perl_Client_ApplySpellRaid);
	package.add("ApplySpellRaid", (void(*)(Client*, int, int, int, bool))&Perl_Client_ApplySpellRaid);
	package.add("ApplySpellRaid", (void(*)(Client*, int, int, int, bool, bool))&Perl_Client_ApplySpellRaid);
	package.add("ApplySpellRaid", (void(*)(Client*, int, int, int, bool, bool, bool))&Perl_Client_ApplySpellRaid);
	package.add("AreTasksCompleted", (bool(*)(Client*, perl::array))&Perl_Client_AreTasksCompleted);
	package.add("AreaTaunt", (void(*)(Client*))&Perl_Client_AreaTaunt);
	package.add("AreaTaunt", (void(*)(Client*, float))&Perl_Client_AreaTaunt);
	package.add("AreaTaunt", (void(*)(Client*, float, int))&Perl_Client_AreaTaunt);
	package.add("AssignTask", (void(*)(Client*, int))&Perl_Client_AssignTask);
	package.add("AssignTask", (void(*)(Client*, int, int))&Perl_Client_AssignTask);
	package.add("AssignTask", (void(*)(Client*, int, int, bool))&Perl_Client_AssignTask);
	package.add("AssignToInstance", &Perl_Client_AssignToInstance);
	package.add("AutoSplitEnabled", &Perl_Client_AutoSplitEnabled);
	package.add("BreakInvis", &Perl_Client_BreakInvis);
	package.add("CalcEXP", (uint64(*)(Client*, uint8))&Perl_Client_CalcEXP);
	package.add("CalcEXP", (uint64(*)(Client*, uint8, bool))&Perl_Client_CalcEXP);
	package.add("CalcPriceMod", (float(*)(Client*))&Perl_Client_CalcPriceMod);
	package.add("CalcPriceMod", (float(*)(Client*, Mob*))&Perl_Client_CalcPriceMod);
	package.add("CalcPriceMod", (float(*)(Client*, Mob*, bool))&Perl_Client_CalcPriceMod);
	package.add("CampAllBots", (void(*)(Client*))&Perl_Client_CampAllBots);
	package.add("CampAllBots", (void(*)(Client*, uint8))&Perl_Client_CampAllBots);
	package.add("CanEnterZone", (bool(*)(Client*, std::string))&Perl_Client_CanEnterZone);
	package.add("CanEnterZone", (bool(*)(Client*, std::string, int16))&Perl_Client_CanEnterZone);
	package.add("CanHaveSkill", &Perl_Client_CanHaveSkill);
	package.add("CashReward", &Perl_Client_CashReward);
	package.add("ChangeLastName", &Perl_Client_ChangeLastName);
	package.add("GrantPetNameChange", &Perl_Client_GrantPetNameChange);
	package.add("ClearNameChange", (bool(*)(Client*))&Perl_Client_ClearNameChange);
	package.add("CharacterID", &Perl_Client_CharacterID);
	package.add("CheckIncreaseSkill", (bool(*)(Client*, int))&Perl_Client_CheckIncreaseSkill);
	package.add("CheckIncreaseSkill", (bool(*)(Client*, int, int))&Perl_Client_CheckIncreaseSkill);
	package.add("CheckSpecializeIncrease", &Perl_Client_CheckSpecializeIncrease);
	package.add("ClearCompassMark", &Perl_Client_ClearCompassMark);
	package.add("ClearAccountFlag", &Perl_Client_ClearAccountFlag);
	package.add("ClearPEQZoneFlag", &Perl_Client_ClearPEQZoneFlag);
	package.add("ClearXTargets", &Perl_Client_ClearXTargets);
	package.add("ClearZoneFlag", &Perl_Client_ClearZoneFlag);
	package.add("CompleteTask", &Perl_Client_CompleteTask);
	package.add("Connected", &Perl_Client_Connected);
	package.add("CountAugmentEquippedByID", &Perl_Client_CountAugmentEquippedByID);
	package.add("CountItem", &Perl_Client_CountItem);
	package.add("CountItemEquippedByID", &Perl_Client_CountItemEquippedByID);
	package.add("CreateExpedition", (DynamicZone*(*)(Client*, perl::reference))&Perl_Client_CreateExpedition);
	package.add("CreateExpedition", (DynamicZone*(*)(Client*, std::string, uint32, uint32, std::string, uint32, uint32))&Perl_Client_CreateExpedition);
	package.add("CreateExpedition", (DynamicZone*(*)(Client*, std::string, uint32, uint32, std::string, uint32, uint32, bool))&Perl_Client_CreateExpedition);
	package.add("CreateExpeditionFromTemplate", &Perl_Client_CreateExpeditionFromTemplate);
	package.add("CreateTaskDynamicZone", &Perl_Client_CreateTaskDynamicZone);
	package.add("DecreaseByID", &Perl_Client_DecreaseByID);
	package.add("DescribeSpecialAbilities", &Perl_Client_DescribeSpecialAbilities);
	package.add("DeleteAccountBucket", &Perl_Client_DeleteAccountBucket);
	package.add("DeleteItemInInventory", (void(*)(Client*, int16))&Perl_Client_DeleteItemInInventory);
	package.add("DeleteItemInInventory", (void(*)(Client*, int16, int16))&Perl_Client_DeleteItemInInventory);
	package.add("DeleteItemInInventory", (void(*)(Client*, int16, int16, bool))&Perl_Client_DeleteItemInInventory);
	package.add("DiaWind", &Perl_Client_DiaWind);
	package.add("DialogueWindow", &Perl_Client_DialogueWindow);
	package.add("Disconnect", &Perl_Client_Disconnect);
	package.add("DropItem", &Perl_Client_DropItem);
	package.add("Duck", &Perl_Client_Duck);
	package.add("DyeArmorBySlot", (void(*)(Client*, uint8, uint8, uint8, uint8))&Perl_Client_DyeArmorBySlot);
	package.add("DyeArmorBySlot", (void(*)(Client*, uint8, uint8, uint8, uint8, uint8))&Perl_Client_DyeArmorBySlot);
	package.add("EndSharedTask", (void(*)(Client*))&Perl_Client_EndSharedTask);
	package.add("EndSharedTask", (void(*)(Client*, bool))&Perl_Client_EndSharedTask);
	package.add("Escape", &Perl_Client_Escape);
	package.add("ExpeditionMessage", &Perl_Client_ExpeditionMessage);
	package.add("FailTask", &Perl_Client_FailTask);
	package.add("FindEmptyMemSlot", &Perl_Client_FindEmptyMemSlot);
	package.add("FindMemmedSpellBySlot", &Perl_Client_FindMemmedSpellBySlot);
	package.add("FindMemmedSpellBySpellID", &Perl_Client_FindMemmedSpellBySpellID);
	package.add("Fling", (void(*)(Client*, float, float, float))&Perl_Client_Fling);
	package.add("Fling", (void(*)(Client*, float, float, float, bool))&Perl_Client_Fling);
	package.add("Fling", (void(*)(Client*, float, float, float, bool, bool))&Perl_Client_Fling);
	package.add("Fling", (void(*)(Client*, float, float, float, float))&Perl_Client_Fling);
	package.add("Fling", (void(*)(Client*, float, float, float, float, bool))&Perl_Client_Fling);
	package.add("Fling", (void(*)(Client*, float, float, float, float, bool, bool))&Perl_Client_Fling);
	package.add("ForageItem", &Perl_Client_ForageItem);
	package.add("Freeze", &Perl_Client_Freeze);
	package.add("GMKill", &Perl_Client_GMKill);
	package.add("GetAAEXPModifier", (float(*)(Client*))&Perl_Client_GetAAEXPModifier);
	package.add("GetAAEXPModifier", (float(*)(Client*, uint32))&Perl_Client_GetAAEXPModifier);
	package.add("GetAAEXPModifier", (float(*)(Client*, uint32, int16))&Perl_Client_GetAAEXPModifier);
	package.add("GetAAEXPPercentage", &Perl_Client_GetAAEXPPercentage);
	package.add("GetAAExp", &Perl_Client_GetAAExp);
	package.add("GetAALevel", &Perl_Client_GetAALevel);
	package.add("GetAAPercent", &Perl_Client_GetAAPercent);
	package.add("GetAAPoints", &Perl_Client_GetAAPoints);
	package.add("GetAFK", &Perl_Client_GetAFK);
	package.add("GetAccountAge", &Perl_Client_GetAccountAge);
	package.add("GetAccountBucket", &Perl_Client_GetAccountBucket);
	package.add("GetAccountBucketExpires", &Perl_Client_GetAccountBucketExpires);
	package.add("GetGetAccountBucketRemaining", &Perl_Client_GetAccountBucketRemaining);
	package.add("GetAccountFlag", &Perl_Client_GetAccountFlag);
	package.add("GetAccountFlags", &Perl_Client_GetAccountFlags);
	package.add("GetAggroCount", &Perl_Client_GetAggroCount);
	package.add("GetAllMoney", &Perl_Client_GetAllMoney);
	package.add("GetAlternateCurrencyValue", &Perl_Client_GetAlternateCurrencyValue);
	package.add("GetAnon", &Perl_Client_GetAnon);
	package.add("GetAugmentAt", &Perl_Client_GetAugmentAt);
	package.add("GetAugmentIDAt", &Perl_Client_GetAugmentIDAt);
	package.add("GetAugmentIDsBySlotID", &Perl_Client_GetAugmentIDsBySlotID);
	package.add("GetAutoLoginCharacterName", &Perl_Client_GetAutoLoginCharacterName);
	package.add("GetBandolierItemIcon", &Perl_Client_GetBandolierItemIcon);
	package.add("GetBandolierItemID", &Perl_Client_GetBandolierItemID);
	package.add("GetBandolierItemName", &Perl_Client_GetBandolierItemName);
	package.add("GetBandolierName", &Perl_Client_GetBandolierName);
	package.add("GetBaseAGI", &Perl_Client_GetBaseAGI);
	package.add("GetBaseCHA", &Perl_Client_GetBaseCHA);
	package.add("GetBaseDEX", &Perl_Client_GetBaseDEX);
	package.add("GetBaseFace", &Perl_Client_GetBaseFace);
	package.add("GetBaseINT", &Perl_Client_GetBaseINT);
	package.add("GetBaseSTA", &Perl_Client_GetBaseSTA);
	package.add("GetBaseSTR", &Perl_Client_GetBaseSTR);
	package.add("GetBaseWIS", &Perl_Client_GetBaseWIS);
	package.add("GetBecomeNPCLevel", &Perl_Client_GetBecomeNPCLevel);
	package.add("GetBindHeading", (float(*)(Client*))&Perl_Client_GetBindHeading);
	package.add("GetBindHeading", (float(*)(Client*, int))&Perl_Client_GetBindHeading);
	package.add("GetBindX", (float(*)(Client*))&Perl_Client_GetBindX);
	package.add("GetBindX", (float(*)(Client*, int))&Perl_Client_GetBindX);
	package.add("GetBindY", (float(*)(Client*))&Perl_Client_GetBindY);
	package.add("GetBindY", (float(*)(Client*, int))&Perl_Client_GetBindY);
	package.add("GetBindZ", (float(*)(Client*))&Perl_Client_GetBindZ);
	package.add("GetBindZ", (float(*)(Client*, int))&Perl_Client_GetBindZ);
	package.add("GetBindZoneID", (uint32_t(*)(Client*))&Perl_Client_GetBindZoneID);
	package.add("GetBindZoneID", (uint32_t(*)(Client*, int))&Perl_Client_GetBindZoneID);
	package.add("GetBotCreationLimit", (uint32(*)(Client*))&Perl_Client_GetBotCreationLimit);
	package.add("GetBotCreationLimit", (uint32(*)(Client*, uint8))&Perl_Client_GetBotCreationLimit);
	package.add("GetBotRequiredLevel", (int(*)(Client*))&Perl_Client_GetBotRequiredLevel);
	package.add("GetBotRequiredLevel", (int(*)(Client*, uint8))&Perl_Client_GetBotRequiredLevel);
	package.add("GetBotSpawnLimit", (int(*)(Client*))&Perl_Client_GetBotSpawnLimit);
	package.add("GetBotSpawnLimit", (int(*)(Client*, uint8))&Perl_Client_GetBotSpawnLimit);
	package.add("GetCarriedMoney", &Perl_Client_GetCarriedMoney);
	package.add("GetCarriedPlatinum", &Perl_Client_GetCarriedPlatinum);
	package.add("GetCharacterFactionLevel", &Perl_Client_GetCharacterFactionLevel);
	package.add("GetClassAbbreviation", &Perl_Client_GetClassAbbreviation);
	package.add("GetClassBitmask", &Perl_Client_GetClassBitmask);
	package.add("GetClientMaxLevel", &Perl_Client_GetClientMaxLevel);
	package.add("GetClientVersion", &Perl_Client_GetClientVersion);
	package.add("GetClientVersionBit", &Perl_Client_GetClientVersionBit);
	package.add("GetCorpseCount", &Perl_Client_GetCorpseCount);
	package.add("GetCorpseID", &Perl_Client_GetCorpseID);
	package.add("GetCorpseItemAt", &Perl_Client_GetCorpseItemAt);
	package.add("GetCustomItemData", &Perl_Client_GetCustomItemData);
	package.add("GetDeityBitmask", &Perl_Client_GetDeityBitmask);
	package.add("GetDiscSlotBySpellID", &Perl_Client_GetDiscSlotBySpellID);
	package.add("GetDisciplineTimer", &Perl_Client_GetDisciplineTimer);
	package.add("GetDuelTarget", &Perl_Client_GetDuelTarget);
	package.add("GetEnvironmentDamageModifier", &Perl_Client_GetEnvironmentDamageModifier);
	package.add("GetEXP", &Perl_Client_GetEXP);
	package.add("GetEXPForLevel", &Perl_Client_GetEXPForLevel);
	package.add("GetEXPModifier", (float(*)(Client*))&Perl_Client_GetEXPModifier);
	package.add("GetEXPModifier", (float(*)(Client*, uint32))&Perl_Client_GetEXPModifier);
	package.add("GetEXPModifier", (float(*)(Client*, uint32, int16))&Perl_Client_GetEXPModifier);
	package.add("GetEXPPercentage", &Perl_Client_GetEXPPercentage);
	package.add("GetEbonCrystals", &Perl_Client_GetEbonCrystals);
	package.add("GetEndurance", &Perl_Client_GetEndurance);
	package.add("GetEnduranceRatio", &Perl_Client_GetEnduranceRatio);
	package.add("GetExpedition", &Perl_Client_GetExpedition);
	package.add("GetExpeditionLockouts", (perl::reference(*)(Client*))&Perl_Client_GetExpeditionLockouts);
	package.add("GetExpeditionLockouts", (perl::reference(*)(Client*, std::string))&Perl_Client_GetExpeditionLockouts);
	package.add("GetFace", &Perl_Client_GetFace);
	package.add("GetFactionLevel", &Perl_Client_GetFactionLevel);
	package.add("GetFeigned", &Perl_Client_GetFeigned);
	package.add("GetFreeDisciplineSlot", (int(*)(Client*))&Perl_Client_GetFreeDisciplineSlot);
	package.add("GetFreeDisciplineSlot", (int(*)(Client*, int))&Perl_Client_GetFreeDisciplineSlot);
	package.add("GetFreeSpellBookSlot", (int(*)(Client*))&Perl_Client_GetFreeSpellBookSlot);
	package.add("GetFreeSpellBookSlot", (int(*)(Client*, uint32))&Perl_Client_GetFreeSpellBookSlot);
	package.add("GetGM", &Perl_Client_GetGM);
	package.add("GetGMStatus", &Perl_Client_GetGMStatus);
	package.add("GetGroup", &Perl_Client_GetGroup);
	package.add("GetGroupPoints", &Perl_Client_GetGroupPoints);
	package.add("GetGuildPublicNote", &Perl_Client_GetGuildPublicNote);
	package.add("GetHorseId", &Perl_Client_GetHorseId);
	package.add("GetHealAmount", &Perl_Client_GetHealAmount);
	package.add("GetHunger", &Perl_Client_GetHunger);
	package.add("GetIntoxication", &Perl_Client_GetIntoxication);
	package.add("GetIP", &Perl_Client_GetIP);
	package.add("GetIPExemption", &Perl_Client_GetIPExemption);
	package.add("GetIPString", &Perl_Client_GetIPString);
	package.add("GetInstanceID", &Perl_Client_GetInstanceID);
	package.add("GetInstrumentMod", &Perl_Client_GetInstrumentMod);
	package.add("GetInventory", &Perl_Client_GetInventory);
	package.add("GetInventorySlots", &Perl_Client_GetInventorySlots);
	package.add("GetInvulnerableEnvironmentDamage", &Perl_Client_GetInvulnerableEnvironmentDamage);
	package.add("GetItemAt", &Perl_Client_GetItemAt);
	package.add("GetItemCooldown", &Perl_Client_GetItemCooldown);
	package.add("GetItemIDAt", &Perl_Client_GetItemIDAt);
	package.add("GetItemInInventory", &Perl_Client_GetItemInInventory);
	package.add("GetKeyRing", &Perl_Client_GetKeyRing);
	package.add("GetLDoNLosses", &Perl_Client_GetLDoNLosses);
	package.add("GetLDoNLossesTheme", &Perl_Client_GetLDoNLossesTheme);
	package.add("GetLDoNPointsTheme", &Perl_Client_GetLDoNPointsTheme);
	package.add("GetLDoNWins", &Perl_Client_GetLDoNWins);
	package.add("GetLDoNWinsTheme", &Perl_Client_GetLDoNWinsTheme);
	package.add("GetLanguageSkill", &Perl_Client_GetLanguageSkill);
	package.add("GetLearnableDisciplines", (perl::array(*)(Client*))&Perl_Client_GetLearnableDisciplines);
	package.add("GetLearnableDisciplines", (perl::array(*)(Client*, uint8))&Perl_Client_GetLearnableDisciplines);
	package.add("GetLearnableDisciplines", (perl::array(*)(Client*, uint8, uint8))&Perl_Client_GetLearnableDisciplines);
	package.add("GetLearnedDisciplines", &Perl_Client_GetLearnedDisciplines);
	package.add("GetLockoutExpeditionUUID", &Perl_Client_GetLockoutExpeditionUUID);
	package.add("GetMaxEndurance", &Perl_Client_GetMaxEndurance);
	package.add("GetMerc", &Perl_Client_GetMerc);
	package.add("GetMemmedSpells", &Perl_Client_GetMemmedSpells);
	package.add("GetModCharacterFactionLevel", &Perl_Client_GetModCharacterFactionLevel);
	package.add("GetMoney", &Perl_Client_GetMoney);
	package.add("GetPotionBeltItemIcon", &Perl_Client_GetPotionBeltItemIcon);
	package.add("GetPotionBeltItemID", &Perl_Client_GetPotionBeltItemID);
	package.add("GetPotionBeltItemName", &Perl_Client_GetPotionBeltItemName);
	package.add("GetPVP", &Perl_Client_GetPVP);
	package.add("GetPVPPoints", &Perl_Client_GetPVPPoints);
	package.add("GetRaceAbbreviation", &Perl_Client_GetRaceAbbreviation);
	package.add("GetRaceBitmask", &Perl_Client_GetRaceBitmask);
	package.add("GetRadiantCrystals", &Perl_Client_GetRadiantCrystals);
	package.add("GetRaid", &Perl_Client_GetRaid);
	package.add("GetRaidOrGroupOrSelf", (perl::array(*)(Client*))&Perl_Client_GetRaidOrGroupOrSelf);
	package.add("GetRaidOrGroupOrSelf", (perl::array(*)(Client*, bool))&Perl_Client_GetRaidOrGroupOrSelf);
	package.add("GetRaidPoints", &Perl_Client_GetRaidPoints);
	package.add("GetRawItemAC", &Perl_Client_GetRawItemAC);
	package.add("GetRawSkill", &Perl_Client_GetRawSkill);
	package.add("GetRecipeMadeCount", &Perl_Client_GetRecipeMadeCount);
	package.add("GetScribeableSpells", (perl::array(*)(Client*))&Perl_Client_GetScribeableSpells);
	package.add("GetScribeableSpells", (perl::array(*)(Client*, uint8))&Perl_Client_GetScribeableSpells);
	package.add("GetScribeableSpells", (perl::array(*)(Client*, uint8, uint8))&Perl_Client_GetScribeableSpells);
	package.add("GetScribedSpells", &Perl_Client_GetScribedSpells);
	package.add("GetSkillPoints", &Perl_Client_GetSkillPoints);
	package.add("GetSpellDamage", &Perl_Client_GetSpellDamage);
	package.add("GetSpellBookSlotBySpellID", &Perl_Client_GetSpellBookSlotBySpellID);
	package.add("GetSpellIDByBookSlot", &Perl_Client_GetSpellIDByBookSlot);
	package.add("GetSpentAA", &Perl_Client_GetSpentAA);
	package.add("GetStartZone", &Perl_Client_GetStartZone);
	package.add("GetTargetRingX", &Perl_Client_GetTargetRingX);
	package.add("GetTargetRingY", &Perl_Client_GetTargetRingY);
	package.add("GetTargetRingZ", &Perl_Client_GetTargetRingZ);
	package.add("GetTaskActivityDoneCount", &Perl_Client_GetTaskActivityDoneCount);
	package.add("GetThirst", &Perl_Client_GetThirst);
	package.add("GetTotalSecondsPlayed", &Perl_Client_GetTotalSecondsPlayed);
	package.add("GetSkillTrainLevel", &Perl_Client_GetSkillTrainLevel);
	package.add("GetWeight", &Perl_Client_GetWeight);
	package.add("GetPEQZoneFlags", &Perl_Client_GetPEQZoneFlags);
	package.add("GetZoneFlags", &Perl_Client_GetZoneFlags);
	package.add("GoFish", &Perl_Client_GoFish);
	package.add("GrantAllAAPoints", (void(*)(Client*))&Perl_Client_GrantAllAAPoints);
	package.add("GrantAllAAPoints", (void(*)(Client*, uint8))&Perl_Client_GrantAllAAPoints);
	package.add("GrantAllAAPoints", (void(*)(Client*, uint8, bool))&Perl_Client_GrantAllAAPoints);
	package.add("GrantAlternateAdvancementAbility", (bool(*)(Client*, int, int))&Perl_Client_GrantAlternateAdvancementAbility);
	package.add("GrantAlternateAdvancementAbility", (bool(*)(Client*, int, int, bool))&Perl_Client_GrantAlternateAdvancementAbility);
	package.add("GrantNameChange", (void(*)(Client*))&Perl_Client_GrantNameChange);
	package.add("GuildID", &Perl_Client_GuildID);
	package.add("GuildRank", &Perl_Client_GuildRank);
	package.add("HasAugmentEquippedByID", &Perl_Client_HasAugmentEquippedByID);
	package.add("HasDisciplineLearned", &Perl_Client_HasDisciplineLearned);
	package.add("HasExpeditionLockout", &Perl_Client_HasExpeditionLockout);
	package.add("HasItemEquippedByID", &Perl_Client_HasItemEquippedByID);
	package.add("HasItemOnCorpse", &Perl_Client_HasItemOnCorpse);
	package.add("HasPEQZoneFlag", &Perl_Client_HasPEQZoneFlag);
	package.add("HasRecipeLearned", &Perl_Client_HasRecipeLearned);
	package.add("HasSkill", &Perl_Client_HasSkill);
	package.add("HasSpellScribed", &Perl_Client_HasSpellScribed);
	package.add("HasZoneFlag", &Perl_Client_HasZoneFlag);
	package.add("Hungry", &Perl_Client_Hungry);
	package.add("InZone", &Perl_Client_InZone);
	package.add("IncStats", &Perl_Client_IncStats);
	package.add("IncreaseLanguageSkill", (void(*)(Client*, uint8))&Perl_Client_IncreaseLanguageSkill);
	package.add("IncreaseLanguageSkill", (void(*)(Client*, uint8, uint8))&Perl_Client_IncreaseLanguageSkill);
	package.add("IncreaseSkill", (void(*)(Client*, int))&Perl_Client_IncreaseSkill);
	package.add("IncreaseSkill", (void(*)(Client*, int, int))&Perl_Client_IncreaseSkill);
	package.add("IncrementAA", &Perl_Client_IncrementAA);
	package.add("IsAutoAttackEnabled", &Perl_Client_IsAutoAttackEnabled);
	package.add("IsAutoFireEnabled", &Perl_Client_IsAutoFireEnabled);
	package.add("IsBecomeNPC", &Perl_Client_IsBecomeNPC);
	package.add("IsCrouching", &Perl_Client_IsCrouching);
	package.add("IsDueling", &Perl_Client_IsDueling);
	package.add("IsEXPEnabled", &Perl_Client_IsEXPEnabled);
	package.add("IsGrouped", &Perl_Client_IsGrouped);
	package.add("IsInAGuild", &Perl_Client_IsInAGuild);
	package.add("IsLD", &Perl_Client_IsLD);
	package.add("IsMedding", &Perl_Client_IsMedding);
	package.add("IsNameChangeAllowed", (bool(*)(Client*))&Perl_Client_IsNameChangeAllowed);
	package.add("IsRaidGrouped", &Perl_Client_IsRaidGrouped);
	package.add("IsSitting", &Perl_Client_IsSitting);
	package.add("IsStanding", &Perl_Client_IsStanding);
	package.add("IsTaskActive", &Perl_Client_IsTaskActive);
	package.add("IsTaskActivityActive", &Perl_Client_IsTaskActivityActive);
	package.add("IsTaskCompleted", &Perl_Client_IsTaskCompleted);
	package.add("KeyRingAdd", &Perl_Client_KeyRingAdd);
	package.add("KeyRingCheck", &Perl_Client_KeyRingCheck);
	package.add("KeyRingClear", &Perl_Client_KeyRingClear);
	package.add("KeyRingList", (void(*)(Client*))&Perl_Client_KeyRingList);
	package.add("KeyRingList", (void(*)(Client*, Client*))&Perl_Client_KeyRingList);
	package.add("KeyRingRemove", &Perl_Client_KeyRingRemove);
	package.add("Kick", &Perl_Client_Kick);
	package.add("LearnDisciplines", &Perl_Client_LearnDisciplines);
	package.add("LearnRecipe", &Perl_Client_LearnRecipe);
	package.add("LeaveGroup", &Perl_Client_LeaveGroup);
	package.add("LoadPEQZoneFlags", &Perl_Client_LoadPEQZoneFlags);
	package.add("LoadZoneFlags", &Perl_Client_LoadZoneFlags);
	package.add("LockSharedTask", &Perl_Client_LockSharedTask);
	package.add("MarkCompassLoc", &Perl_Client_MarkCompassLoc);
	package.add("Marquee", (void(*)(Client*, uint32, std::string))&Perl_Client_SendMarqueeMessage);
	package.add("Marquee", (void(*)(Client*, uint32, std::string, uint32))&Perl_Client_SendMarqueeMessage);
	package.add("Marquee", (void(*)(Client*, uint32, uint32, uint32, uint32, uint32, std::string))&Perl_Client_SendMarqueeMessage);
	package.add("MaxSkill", (int(*)(Client*, uint16))&Perl_Client_MaxSkill);
	package.add("MaxSkill", (int(*)(Client*, uint16, uint16))&Perl_Client_MaxSkill);
	package.add("MaxSkill", (int(*)(Client*, uint16, uint16, uint16))&Perl_Client_MaxSkill);
	package.add("MaxSkills", &Perl_Client_MaxSkills);
	package.add("MemSpell", (void(*)(Client*, uint16, int))&Perl_Client_MemSpell);
	package.add("MemSpell", (void(*)(Client*, uint16, int, bool))&Perl_Client_MemSpell);
	package.add("MemmedCount", &Perl_Client_MemmedCount);
	package.add("MovePC", &Perl_Client_MovePC);
	package.add("MovePCDynamicZone", (void(*)(Client*, perl::scalar))&Perl_Client_MovePCDynamicZone);
	package.add("MovePCDynamicZone", (void(*)(Client*, perl::scalar, int))&Perl_Client_MovePCDynamicZone);
	package.add("MovePCDynamicZone", (void(*)(Client*, perl::scalar, int, bool))&Perl_Client_MovePCDynamicZone);
	package.add("MovePCInstance", &Perl_Client_MovePCInstance);
	package.add("MoveZone", (void(*)(Client*, const char*))&Perl_Client_MoveZone);
	package.add("MoveZone", (void(*)(Client*, const char*, float, float, float))&Perl_Client_MoveZone);
	package.add("MoveZone", (void(*)(Client*, const char*, float, float, float, float))&Perl_Client_MoveZone);
	package.add("MoveZoneGroup", (void(*)(Client*, const char*))&Perl_Client_MoveZoneGroup);
	package.add("MoveZoneGroup", (void(*)(Client*, const char*, float, float, float))&Perl_Client_MoveZoneGroup);
	package.add("MoveZoneGroup", (void(*)(Client*, const char*, float, float, float, float))&Perl_Client_MoveZoneGroup);
	package.add("MoveZoneInstance", (void(*)(Client*, uint16))&Perl_Client_MoveZoneInstance);
	package.add("MoveZoneInstance", (void(*)(Client*, uint16, float, float, float))&Perl_Client_MoveZoneInstance);
	package.add("MoveZoneInstance", (void(*)(Client*, uint16, float, float, float, float))&Perl_Client_MoveZoneInstance);
	package.add("MoveZoneInstanceGroup", (void(*)(Client*, uint16))&Perl_Client_MoveZoneInstanceGroup);
	package.add("MoveZoneInstanceGroup", (void(*)(Client*, uint16, float, float, float))&Perl_Client_MoveZoneInstanceGroup);
	package.add("MoveZoneInstanceGroup", (void(*)(Client*, uint16, float, float, float, float))&Perl_Client_MoveZoneInstanceGroup);
	package.add("MoveZoneInstanceRaid", (void(*)(Client*, uint16))&Perl_Client_MoveZoneInstanceRaid);
	package.add("MoveZoneInstanceRaid", (void(*)(Client*, uint16, float, float, float))&Perl_Client_MoveZoneInstanceRaid);
	package.add("MoveZoneInstanceRaid", (void(*)(Client*, uint16, float, float, float, float))&Perl_Client_MoveZoneInstanceRaid);
	package.add("MoveZoneRaid", (void(*)(Client*, const char*))&Perl_Client_MoveZoneRaid);
	package.add("MoveZoneRaid", (void(*)(Client*, const char*, float, float, float))&Perl_Client_MoveZoneRaid);
	package.add("MoveZoneRaid", (void(*)(Client*, const char*, float, float, float, float))&Perl_Client_MoveZoneRaid);
	package.add("NPCSpawn", (void(*)(Client*, NPC*, const char*))&Perl_Client_NPCSpawn);
	package.add("NPCSpawn", (void(*)(Client*, NPC*, const char*, uint32))&Perl_Client_NPCSpawn);
	package.add("NotifyNewTitlesAvailable", &Perl_Client_NotifyNewTitlesAvailable);
	package.add("NukeItem", (uint32_t(*)(Client*, uint32))&Perl_Client_NukeItem);
	package.add("NukeItem", (uint32_t(*)(Client*, uint32, uint8))&Perl_Client_NukeItem);
	package.add("OpenLFGuildWindow", &Perl_Client_OpenLFGuildWindow);
	package.add("PlayMP3", &Perl_Client_PlayMP3);
	package.add("Popup2", (void(*)(Client*, const char*, const char*))&Perl_Client_Popup2);
	package.add("Popup2", (void(*)(Client*, const char*, const char*, uint32))&Perl_Client_Popup2);
	package.add("Popup2", (void(*)(Client*, const char*, const char*, uint32, uint32))&Perl_Client_Popup2);
	package.add("Popup2", (void(*)(Client*, const char*, const char*, uint32, uint32, uint32))&Perl_Client_Popup2);
	package.add("Popup2", (void(*)(Client*, const char*, const char*, uint32, uint32, uint32, uint32))&Perl_Client_Popup2);
	package.add("Popup2", (void(*)(Client*, const char*, const char*, uint32, uint32, uint32, uint32, const char*))&Perl_Client_Popup2);
	package.add("Popup2", (void(*)(Client*, const char*, const char*, uint32, uint32, uint32, uint32, const char*, const char*))&Perl_Client_Popup2);
	package.add("Popup2", (void(*)(Client*, const char*, const char*, uint32, uint32, uint32, uint32, const char*, const char*, uint32))&Perl_Client_Popup2);
	package.add("QuestReward", (void(*)(Client*, Mob*))&Perl_Client_QuestReward);
	package.add("QuestReward", (void(*)(Client*, Mob*, uint32))&Perl_Client_QuestReward);
	package.add("QuestReward", (void(*)(Client*, Mob*, uint32, uint32))&Perl_Client_QuestReward);
	package.add("QuestReward", (void(*)(Client*, Mob*, uint32, uint32, uint32))&Perl_Client_QuestReward);
	package.add("QuestReward", (void(*)(Client*, Mob*, uint32, uint32, uint32, uint32))&Perl_Client_QuestReward);
	package.add("QuestReward", (void(*)(Client*, Mob*, uint32, uint32, uint32, uint32, uint32))&Perl_Client_QuestReward);
	package.add("QuestReward", (void(*)(Client*, Mob*, uint32, uint32, uint32, uint32, uint32, uint32))&Perl_Client_QuestReward);
	package.add("QuestReward", (void(*)(Client*, Mob*, uint32, uint32, uint32, uint32, uint32, uint32, bool))&Perl_Client_QuestReward);
	package.add("ReadBook", &Perl_Client_ReadBook);
	package.add("ReadBookByName", &Perl_Client_ReadBookByName);
	package.add("RefundAA", &Perl_Client_RefundAA);
	package.add("ReloadDataBuckets", &Perl_Client_ReloadDataBuckets);
	package.add("RemoveAAPoints", &Perl_Client_RemoveAAPoints);
	package.add("RemoveAllExpeditionLockouts", (void(*)(Client*))&Perl_Client_RemoveAllExpeditionLockouts);
	package.add("RemoveAllExpeditionLockouts", (void(*)(Client*, std::string))&Perl_Client_RemoveAllExpeditionLockouts);
	package.add("RemoveAlternateCurrencyValue", (bool(*)(Client*, uint32, uint32))&Perl_Client_RemoveAlternateCurrencyValue);
	package.add("RemoveEbonCrystals", &Perl_Client_RemoveEbonCrystals);
	package.add("RemoveExpeditionLockout", &Perl_Client_RemoveExpeditionLockout);
	package.add("RemoveFromInstance", &Perl_Client_RemoveFromInstance);
	package.add("RemoveItem", (void(*)(Client*, uint32))&Perl_Client_RemoveItem);
	package.add("RemoveItem", (void(*)(Client*, uint32, uint32))&Perl_Client_RemoveItem);
	package.add("RemoveLDoNLoss", &Perl_Client_RemoveLDoNLoss);
	package.add("RemoveLDoNWin", &Perl_Client_RemoveLDoNWin);
	package.add("RemoveNoRent", &Perl_Client_RemoveNoRent);
	package.add("RemoveRadiantCrystals", &Perl_Client_RemoveRadiantCrystals);
	package.add("ResetAA", &Perl_Client_ResetAA);
	package.add("ResetAllDisciplineTimers", &Perl_Client_ResetAllDisciplineTimers);
	package.add("ResetAllCastbarCooldowns", &Perl_Client_ResetAllCastbarCooldowns);
	package.add("ResetAlternateAdvancementRank", &Perl_Client_ResetAlternateAdvancementRank);
	package.add("ResetCastbarCooldownBySlot", &Perl_Client_ResetCastbarCooldownBySlot);
	package.add("ResetCastbarCooldownBySpellID", &Perl_Client_ResetCastbarCooldownBySpellID);
	package.add("ResetDisciplineTimer", &Perl_Client_ResetDisciplineTimer);
	package.add("ResetItemCooldown", &Perl_Client_ResetItemCooldown);
	package.add("ResetLeadershipAA", &Perl_Client_ResetLeadershipAA);
	package.add("ResetTrade", &Perl_Client_ResetTrade);
	package.add("Save", &Perl_Client_Save);
	package.add("ScribeSpell", (void(*)(Client*, uint16, int))&Perl_Client_ScribeSpell);
	package.add("ScribeSpell", (void(*)(Client*, uint16, int, bool))&Perl_Client_ScribeSpell);
	package.add("ScribeSpells", &Perl_Client_ScribeSpells);
	package.add("SendColoredText", &Perl_Client_SendColoredText);
	package.add("SendGMCommand", (bool(*)(Client*, std::string))&Perl_Client_SendGMCommand);
	package.add("SendGMCommand", (bool(*)(Client*, std::string, bool))&Perl_Client_SendGMCommand);
	package.add("SendMarqueeMessage", (void(*)(Client*, uint32, std::string))&Perl_Client_SendMarqueeMessage);
	package.add("SendMarqueeMessage", (void(*)(Client*, uint32, std::string, uint32))&Perl_Client_SendMarqueeMessage);
	package.add("SendMarqueeMessage", (void(*)(Client*, uint32, uint32, uint32, uint32, uint32, std::string))&Perl_Client_SendMarqueeMessage);
	package.add("SendOPTranslocateConfirm", &Perl_Client_SendOPTranslocateConfirm);
	package.add("SendPayload", (void(*)(Client*, int))&Perl_Client_SendPayload);
	package.add("SendPayload", (void(*)(Client*, int, std::string))&Perl_Client_SendPayload);
	package.add("SendPath", &Perl_Client_SendPath);
	package.add("SendPEQZoneFlagInfo", &Perl_Client_SendPEQZoneFlagInfo);
	package.add("SendSound", &Perl_Client_SendSound);
	package.add("SendSpellAnim", &Perl_Client_SendSpellAnim);
	package.add("SendTargetCommand", &Perl_Client_SendTargetCommand);
	package.add("SendToGuildHall", &Perl_Client_SendToGuildHall);
	package.add("SendToInstance", &Perl_Client_SendToInstance);
	package.add("SendWebLink", &Perl_Client_SendWebLink);
	package.add("SendZoneFlagInfo", &Perl_Client_SendZoneFlagInfo);
	package.add("SetAAEXPModifier", (void(*)(Client*, float))&Perl_Client_SetAAEXPModifier);
	package.add("SetAAEXPModifier", (void(*)(Client*, uint32, float))&Perl_Client_SetAAEXPModifier);
	package.add("SetAAEXPModifier", (void(*)(Client*, uint32, float, int16))&Perl_Client_SetAAEXPModifier);
	package.add("SetAAEXPPercentage", &Perl_Client_SetAAEXPPercentage);
	package.add("SetAAPoints", &Perl_Client_SetAAPoints);
	package.add("SetAATitle", (void(*)(Client*, std::string))&Perl_Client_SetAATitle);
	package.add("SetAATitle", (void(*)(Client*, std::string, bool))&Perl_Client_SetAATitle);
	package.add("SetAFK", &Perl_Client_SetAFK);
	package.add("SetAccountFlag", &Perl_Client_SetAccountFlag);
	package.add("SetAccountBucket", (void(*)(Client*, std::string, std::string))&Perl_Client_SetAccountBucket);
	package.add("SetAccountBucket", (void(*)(Client*, std::string, std::string, std::string))&Perl_Client_SetAccountBucket);
	package.add("SetAlternateCurrencyValue", &Perl_Client_SetAlternateCurrencyValue);
	package.add("SetAnon", &Perl_Client_SetAnon);
	package.add("SetAutoLoginCharacterName", (bool(*)(Client*))&Perl_Client_SetAutoLoginCharacterName);
	package.add("SetAutoLoginCharacterName", (bool(*)(Client*, std::string))&Perl_Client_SetAutoLoginCharacterName);
	package.add("SetBaseClass", &Perl_Client_SetBaseClass);
	package.add("SetBaseGender", &Perl_Client_SetBaseGender);
	package.add("SetBaseRace", &Perl_Client_SetBaseRace);
	package.add("SetBecomeNPC", &Perl_Client_SetBecomeNPC);
	package.add("SetBecomeNPCLevel", &Perl_Client_SetBecomeNPCLevel);
	package.add("SetBindPoint", (void(*)(Client*))&Perl_Client_SetBindPoint);
	package.add("SetBindPoint", (void(*)(Client*, int))&Perl_Client_SetBindPoint);
	package.add("SetBindPoint", (void(*)(Client*, int, int))&Perl_Client_SetBindPoint);
	package.add("SetBindPoint", (void(*)(Client*, int, int, float))&Perl_Client_SetBindPoint);
	package.add("SetBindPoint", (void(*)(Client*, int, int, float, float))&Perl_Client_SetBindPoint);
	package.add("SetBindPoint", (void(*)(Client*, int, int, float, float, float))&Perl_Client_SetBindPoint);
	package.add("SetBindPoint", (void(*)(Client*, int, int, float, float, float, float))&Perl_Client_SetBindPoint);
	package.add("SetBotCreationLimit", (void(*)(Client*, uint32))&Perl_Client_SetBotCreationLimit);
	package.add("SetBotCreationLimit", (void(*)(Client*, uint32, uint8))&Perl_Client_SetBotCreationLimit);
	package.add("SetBotRequiredLevel", (void(*)(Client*, int))&Perl_Client_SetBotRequiredLevel);
	package.add("SetBotRequiredLevel", (void(*)(Client*, int, uint8))&Perl_Client_SetBotRequiredLevel);
	package.add("SetBotSpawnLimit", (void(*)(Client*, int))&Perl_Client_SetBotSpawnLimit);
	package.add("SetBotSpawnLimit", (void(*)(Client*, int, uint8))&Perl_Client_SetBotSpawnLimit);
	package.add("SetClientMaxLevel", &Perl_Client_SetClientMaxLevel);
	package.add("SetConsumption", &Perl_Client_SetConsumption);
	package.add("SetCustomItemData", &Perl_Client_SetCustomItemData);
	package.add("SetDeity", &Perl_Client_SetDeity);
	package.add("SetDuelTarget", &Perl_Client_SetDuelTarget);
	package.add("SetDueling", &Perl_Client_SetDueling);
	package.add("SetEXP", (void(*)(Client*, uint64, uint64))&Perl_Client_SetEXP);
	package.add("SetEXP", (void(*)(Client*, uint64, uint64, bool))&Perl_Client_SetEXP);
	package.add("SetEXPModifier", (void(*)(Client*, float))&Perl_Client_SetEXPModifier);
	package.add("SetEXPModifier", (void(*)(Client*, uint32, float))&Perl_Client_SetEXPModifier);
	package.add("SetEXPModifier", (void(*)(Client*, uint32, float, int16))&Perl_Client_SetEXPModifier);
	package.add("SetEbonCrystals", &Perl_Client_SetEbonCrystals);
	package.add("SetEndurance", &Perl_Client_SetEndurance);
	package.add("SetEnvironmentDamageModifier", &Perl_Client_SetEnvironmentDamageModifier);
	package.add("SetEXPEnabled", &Perl_Client_SetEXPEnabled);
	package.add("SetFactionLevel", &Perl_Client_SetFactionLevel);
	package.add("SetFactionLevel2", (void(*)(Client*, uint32, int32, uint8, uint8, uint8, int32))&Perl_Client_SetFactionLevel2);
	package.add("SetFactionLevel2", (void(*)(Client*, uint32, int32, uint8, uint8, uint8, int32, uint8))&Perl_Client_SetFactionLevel2);
	package.add("SetFeigned", &Perl_Client_SetFeigned);
	package.add("SetGM", &Perl_Client_SetGM);
	package.add("SetGMStatus", &Perl_Client_SetGMStatus);
	package.add("SetHideMe", &Perl_Client_SetHideMe);
	package.add("SetHorseId", &Perl_Client_SetHorseId);
	package.add("SetHunger", &Perl_Client_SetHunger);
	package.add("SetIPExemption", &Perl_Client_SetIPExemption);
	package.add("SetInvulnerableEnvironmentDamage", &Perl_Client_SetInvulnerableEnvironmentDamage);
	package.add("SetItemCooldown", &Perl_Client_SetItemCooldown);
	package.add("SetLanguageSkill", &Perl_Client_SetLanguageSkill);
	package.add("SetLDoNPoints", &Perl_Client_SetLDoNPoints);
	package.add("SetMaterial", &Perl_Client_SetMaterial);
	package.add("SetPEQZoneFlag", &Perl_Client_SetPEQZoneFlag);
	package.add("SetPVP", &Perl_Client_SetPVP);
	package.add("SetPrimaryWeaponOrnamentation", &Perl_Client_SetPrimaryWeaponOrnamentation);
	package.add("SetRadiantCrystals", &Perl_Client_SetRadiantCrystals);
	package.add("SetSecondaryWeaponOrnamentation", &Perl_Client_SetSecondaryWeaponOrnamentation);
	package.add("SetSkill", &Perl_Client_SetSkill);
	package.add("SetSkillPoints", &Perl_Client_SetSkillPoints);
	package.add("SetSpellDuration", (void(*)(Client*, int))&Perl_Client_SetSpellDuration);
	package.add("SetSpellDuration", (void(*)(Client*, int, int))&Perl_Client_SetSpellDuration);
	package.add("SetSpellDuration", (void(*)(Client*, int, int, int))&Perl_Client_SetSpellDuration);
	package.add("SetSpellDuration", (void(*)(Client*, int, int, int, bool))&Perl_Client_SetSpellDuration);
	package.add("SetSpellDuration", (void(*)(Client*, int, int, int, bool, bool))&Perl_Client_SetSpellDuration);
	package.add("SetSpellDurationGroup", (void(*)(Client*, int))&Perl_Client_SetSpellDurationGroup);
	package.add("SetSpellDurationGroup", (void(*)(Client*, int, int))&Perl_Client_SetSpellDurationGroup);
	package.add("SetSpellDurationGroup", (void(*)(Client*, int, int, int))&Perl_Client_SetSpellDurationGroup);
	package.add("SetSpellDurationGroup", (void(*)(Client*, int, int, int, bool))&Perl_Client_SetSpellDurationGroup);
	package.add("SetSpellDurationGroup", (void(*)(Client*, int, int, int, bool, bool))&Perl_Client_SetSpellDurationGroup);
	package.add("SetSpellDurationRaid", (void(*)(Client*, int))&Perl_Client_SetSpellDurationRaid);
	package.add("SetSpellDurationRaid", (void(*)(Client*, int, int))&Perl_Client_SetSpellDurationRaid);
	package.add("SetSpellDurationRaid", (void(*)(Client*, int, int, int))&Perl_Client_SetSpellDurationRaid);
	package.add("SetSpellDurationRaid", (void(*)(Client*, int, int, int, bool))&Perl_Client_SetSpellDurationRaid);
	package.add("SetSpellDurationRaid", (void(*)(Client*, int, int, int, bool, bool))&Perl_Client_SetSpellDurationRaid);
	package.add("SetSpellDurationRaid", (void(*)(Client*, int, int, int, bool, bool, bool))&Perl_Client_SetSpellDurationRaid);
	package.add("SetSpellDurationRaid", (void(*)(Client*, int, int, int, bool, bool, bool))&Perl_Client_SetSpellDurationRaid);
	package.add("SetStartZone", (void(*)(Client*, uint32))&Perl_Client_SetStartZone);
	package.add("SetStartZone", (void(*)(Client*, uint32, float, float, float))&Perl_Client_SetStartZone);
	package.add("SetStartZone", (void(*)(Client*, uint32, float, float, float, float))&Perl_Client_SetStartZone);
	package.add("SetStats", &Perl_Client_SetStats);
	package.add("SetThirst", &Perl_Client_SetThirst);
	package.add("SetTint", &Perl_Client_SetTint);
	package.add("SetTitleSuffix", (void(*)(Client*, std::string))&Perl_Client_SetTitleSuffix);
	package.add("SetTitleSuffix", (void(*)(Client*, std::string, bool))&Perl_Client_SetTitleSuffix);
	package.add("SetZoneFlag", &Perl_Client_SetZoneFlag);
	package.add("ShowZoneShardMenu", &Perl_Client_ShowZoneShardMenu);
	package.add("Signal", &Perl_Client_Signal);
	package.add("SignalClient", &Perl_Client_SignalClient);
	package.add("SilentMessage", &Perl_Client_SilentMessage);
	package.add("Sit", &Perl_Client_Sit);
	package.add("SlotConvert2", &Perl_Client_SlotConvert2);
	package.add("Stand", &Perl_Client_Stand);
	package.add("SummonBaggedItems", &Perl_Client_SummonBaggedItems);
	package.add("SummonItem", (void(*)(Client*, uint32))&Perl_Client_SummonItem);
	package.add("SummonItem", (void(*)(Client*, uint32, int16))&Perl_Client_SummonItem);
	package.add("SummonItem", (void(*)(Client*, uint32, int16, bool))&Perl_Client_SummonItem);
	package.add("SummonItem", (void(*)(Client*, uint32, int16, bool, uint32))&Perl_Client_SummonItem);
	package.add("SummonItem", (void(*)(Client*, uint32, int16, bool, uint32, uint32))&Perl_Client_SummonItem);
	package.add("SummonItem", (void(*)(Client*, uint32, int16, bool, uint32, uint32, uint32))&Perl_Client_SummonItem);
	package.add("SummonItem", (void(*)(Client*, uint32, int16, bool, uint32, uint32, uint32, uint32))&Perl_Client_SummonItem);
	package.add("SummonItem", (void(*)(Client*, uint32, int16, bool, uint32, uint32, uint32, uint32, uint32))&Perl_Client_SummonItem);
	package.add("SummonItem", (void(*)(Client*, uint32, int16, bool, uint32, uint32, uint32, uint32, uint32, uint16))&Perl_Client_SummonItem);
	package.add("SummonItemIntoInventory", &Perl_Client_SummonItemIntoInventory);
	package.add("TGB", &Perl_Client_TGB);
	package.add("TakeMoneyFromPP", (bool(*)(Client*, uint64_t))&Perl_Client_TakeMoneyFromPP);
	package.add("TakeMoneyFromPP", (bool(*)(Client*, uint64_t, bool))&Perl_Client_TakeMoneyFromPP);
	package.add("TakePlatinum", (bool(*)(Client*, uint32))&Perl_Client_TakePlatinum);
	package.add("TakePlatinum", (bool(*)(Client*, uint32, bool))&Perl_Client_TakePlatinum);
	package.add("TeleportToPlayerByCharID", &Perl_Client_TeleportToPlayerByCharacterID);
	package.add("TeleportToPlayerByName", &Perl_Client_TeleportToPlayerByName);
	package.add("TeleportGroupToPlayerByCharID", &Perl_Client_TeleportGroupToPlayerByCharacterID);
	package.add("TeleportGroupToPlayerByName", &Perl_Client_TeleportGroupToPlayerByName);
	package.add("TeleportRaidToPlayerByCharID", &Perl_Client_TeleportRaidToPlayerByCharacterID);
	package.add("TeleportRaidToPlayerByName", &Perl_Client_TeleportRaidToPlayerByName);
	package.add("TaskSelector", &Perl_Client_TaskSelector);
	package.add("TaskSelectorNoCooldown", &Perl_Client_TaskSelectorNoCooldown);
	package.add("Thirsty", &Perl_Client_Thirsty);
	package.add("TrainDiscBySpellID", &Perl_Client_TrainDiscBySpellID);
	package.add("UnFreeze", &Perl_Client_UnFreeze);
	package.add("UncompleteTask", &Perl_Client_UncompleteTask);
	package.add("Undye", &Perl_Client_Undye);
	package.add("UnmemSpell", (void(*)(Client*, int))&Perl_Client_UnmemSpell);
	package.add("UnmemSpell", (void(*)(Client*, int, bool))&Perl_Client_UnmemSpell);
	package.add("UnmemSpellAll", (void(*)(Client*))&Perl_Client_UnmemSpellAll);
	package.add("UnmemSpellAll", (void(*)(Client*, bool))&Perl_Client_UnmemSpellAll);
	package.add("UnmemSpellBySpellID", &Perl_Client_UnmemSpellBySpellID);
	package.add("UnscribeSpell", (void(*)(Client*, int))&Perl_Client_UnscribeSpell);
	package.add("UnscribeSpell", (void(*)(Client*, int, bool))&Perl_Client_UnscribeSpell);
	package.add("UnscribeSpellAll", (void(*)(Client*))&Perl_Client_UnscribeSpellAll);
	package.add("UnscribeSpellAll", (void(*)(Client*, bool))&Perl_Client_UnscribeSpellAll);
	package.add("UnscribeSpellBySpellID", (void(*)(Client*, uint16))&Perl_Client_UnscribeSpellBySpellID);
	package.add("UnscribeSpellBySpellID", (void(*)(Client*, uint16, bool))&Perl_Client_UnscribeSpellBySpellID);
	package.add("UntrainDisc", (void(*)(Client*, int))&Perl_Client_UntrainDisc);
	package.add("UntrainDisc", (void(*)(Client*, int, bool))&Perl_Client_UntrainDisc);
	package.add("UntrainDiscAll", (void(*)(Client*))&Perl_Client_UntrainDiscAll);
	package.add("UntrainDiscAll", (void(*)(Client*, bool))&Perl_Client_UntrainDiscAll);
	package.add("UntrainDiscBySpellID", (void(*)(Client*, uint16))&Perl_Client_UntrainDiscBySpellID);
	package.add("UntrainDiscBySpellID", (void(*)(Client*, uint16, bool))&Perl_Client_UntrainDiscBySpellID);
	package.add("UpdateAdmin", (void(*)(Client*))&Perl_Client_UpdateAdmin);
	package.add("UpdateAdmin", (void(*)(Client*, bool))&Perl_Client_UpdateAdmin);
	package.add("UpdateGroupAAs", &Perl_Client_UpdateGroupAAs);
	package.add("UpdateLDoNPoints", &Perl_Client_UpdateLDoNPoints);
	package.add("UpdateTaskActivity", (void(*)(Client*, int, int, int))&Perl_Client_UpdateTaskActivity);
	package.add("UpdateTaskActivity", (void(*)(Client*, int, int, int, bool))&Perl_Client_UpdateTaskActivity);
	package.add("UpdateWho", (void(*)(Client*))&Perl_Client_UpdateWho);
	package.add("UpdateWho", (void(*)(Client*, uint8))&Perl_Client_UpdateWho);
	package.add("UseDiscipline", &Perl_Client_UseDiscipline);
	package.add("UseAugmentContainer", &Perl_Client_UseAugmentContainer);
	package.add("WorldKick", &Perl_Client_WorldKick);
}

#endif //EMBPERL_XS_CLASSES
