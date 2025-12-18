#include "common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "common/spdat.h"
#include "zone/bot.h"
#include "zone/client.h"
#include "zone/dialogue_window.h"
#include "zone/embperl.h"
#include "zone/mob.h"
#include "zone/questmgr.h"

bool Perl_Mob_IsClient(Mob* self) // @categories Script Utility
{
	return self->IsClient();
}

bool Perl_Mob_IsNPC(Mob* self) // @categories Script Utility
{
	return self->IsNPC();
}

bool Perl_Mob_IsMob(Mob* self) // @categories Script Utility
{
	return self->IsMob();
}

bool Perl_Mob_IsMerc(Mob* self) // @categories Script Utility
{
	return self->IsMerc();
}

bool Perl_Mob_IsCorpse(Mob* self) // @categories Script Utility, Corpse
{
	return self->IsCorpse();
}

bool Perl_Mob_IsPlayerCorpse(Mob* self) // @categories Corpse
{
	return self->IsPlayerCorpse();
}

bool Perl_Mob_IsNPCCorpse(Mob* self) // @categories Corpse
{
	return self->IsNPCCorpse();
}

bool Perl_Mob_IsObject(Mob* self) // @categories Objects
{
	return self->IsObject();
}

bool Perl_Mob_IsDoor(Mob* self) // @categories Script Utility, Doors
{
	return self->IsDoor();
}

bool Perl_Mob_IsTrap(Mob* self) // @categories Script Utility
{
	return self->IsTrap();
}

bool Perl_Mob_IsBeacon(Mob* self) // @categories Script Utility
{
	return self->IsBeacon();
}

bool Perl_Mob_IsEncounter(Mob* self) // @categories Script Utility
{
	return self->IsEncounter();
}

bool Perl_Mob_IsBot(Mob* self) // @categories Script Utility
{
	return self->IsBot();
}

bool Perl_Mob_IsAura(Mob* self) // @categories Script Utility
{
	return self->IsAura();
}

bool Perl_Mob_IsOfClientBot(Mob* self) // @categories Script Utility
{
	return self->IsOfClientBot();
}

bool Perl_Mob_IsOfClientBotMerc(Mob* self) // @categories Script Utility
{
	return self->IsOfClientBotMerc();
}

bool Perl_Mob_IsTemporaryPet(Mob* self) // @categories Script Utility
{
	return self->IsTempPet();
}

Client* Perl_Mob_CastToClient(Mob* self) // @categories Account and Character, Script Utility
{
	return self->CastToClient();
}

NPC* Perl_Mob_CastToNPC(Mob* self) // @categories Script Utility
{
	return self->CastToNPC();
}

Mob* Perl_Mob_CastToMob(Mob* self) // @categories Script Utility
{
	return self->CastToMob();
}

Corpse* Perl_Mob_CastToCorpse(Mob* self) // @categories Script Utility, Corpse
{
	return self->CastToCorpse();
}

int Perl_Mob_GetID(Mob* self) // @categories Script Utility
{
	return self->GetID();
}

std::string Perl_Mob_GetName(Mob* self)// @categories Script Utility
{
	return self->GetName();
}

void Perl_Mob_Depop(Mob* self) // @categories Spawns
{
	self->Depop();
}

void Perl_Mob_Depop(Mob* self, bool start_spawn_timer) // @categories Spawns
{
	self->Depop(start_spawn_timer);
}

void Perl_Mob_RogueAssassinate(Mob* self, Mob* other) // @categories Script Utility
{
	return self->RogueAssassinate(other);
}

bool Perl_Mob_BehindMob(Mob* self, Mob* other) // @categories Script Utility
{
	return self->BehindMob(other);
}

bool Perl_Mob_BehindMob(Mob* self, Mob* other, float x) // @categories Script Utility
{
	return self->BehindMob(other, x);
}

bool Perl_Mob_BehindMob(Mob* self, Mob* other, float x, float y) // @categories Script Utility
{
	return self->BehindMob(other, x, y);
}

void Perl_Mob_SetLevel(Mob* self, uint8_t in_level) // @categories Stats and Attributes
{
	self->SetLevel(in_level);
}

void Perl_Mob_SetLevel(Mob* self, uint8_t in_level, bool command) // @categories Stats and Attributes
{
	self->SetLevel(in_level, command);
}

uint32_t Perl_Mob_GetSkill(Mob* self, int skill_id) // @categories Skills and Recipes, Script Utility
{
	return self->GetSkill(static_cast<EQ::skills::SkillType>(skill_id));
}

void Perl_Mob_SendWearChange(Mob* self, uint8_t material_slot) // @categories Script Utility
{
	self->SendWearChange(material_slot);
}

uint32_t Perl_Mob_GetEquipment(Mob* self, uint8_t material_slot) // @categories Inventory and Items
{
	return self->GetEquippedItemFromTextureSlot(material_slot);
}

uint32_t Perl_Mob_GetEquipmentMaterial(Mob* self, uint8_t material_slot) // @categories Inventory and Items
{
	return self->GetEquipmentMaterial(material_slot);
}

uint32_t Perl_Mob_GetEquipmentColor(Mob* self, uint8_t material_slot) // @categories Inventory and Items
{
	return self->GetEquipmentColor(material_slot);
}

int32_t Perl_Mob_GetArmorTint(Mob* self, uint8_t material_slot) // @categories Stats and Attributes
{
	return self->GetArmorTint(material_slot);
}

bool Perl_Mob_IsMoving(Mob* self) // @categories Script Utility
{
	return self->IsMoving();
}

void Perl_Mob_GoToBind(Mob* self) // @categories Script Utility
{
	self->GoToBind();
}

void Perl_Mob_Gate(Mob* self) // @categories Spells and Disciplines
{
	self->Gate();
}

bool Perl_Mob_Attack(Mob* self, Mob* other) // @categories Script Utility, Hate and Aggro
{
	return self->Attack(other);
}

bool Perl_Mob_Attack(Mob* self, Mob* other, int hand) // @categories Script Utility, Hate and Aggro
{
	return self->Attack(other, hand);
}

bool Perl_Mob_Attack(Mob* self, Mob* other, int hand, bool from_riposte) // @categories Script Utility, Hate and Aggro
{
	return self->Attack(other, hand, from_riposte);
}

void Perl_Mob_Damage(Mob* self, Mob* from, int64_t damage, uint16_t spell_id, int attack_skill) // @categories Script Utility
{
	self->Damage(from, damage, spell_id, static_cast<EQ::skills::SkillType>(attack_skill));
}

void Perl_Mob_Damage(Mob* self, Mob* from, int64_t damage, uint16_t spell_id, int attack_skill, bool avoidable) // @categories Script Utility
{
	self->Damage(from, damage, spell_id, static_cast<EQ::skills::SkillType>(attack_skill), avoidable);
}

void Perl_Mob_Damage(Mob* self, Mob* from, int64_t damage, uint16_t spell_id, int attack_skill, bool avoidable, int8_t buffslot) // @categories Script Utility
{
	self->Damage(from, damage, spell_id, static_cast<EQ::skills::SkillType>(attack_skill), avoidable, buffslot);
}

void Perl_Mob_Damage(Mob* self, Mob* from, int64_t damage, uint16_t spell_id, int attack_skill, bool avoidable, int8_t buffslot, bool buff_tic) // @categories Script Utility
{
	self->Damage(from, damage, spell_id, static_cast<EQ::skills::SkillType>(attack_skill), avoidable, buffslot, buff_tic);
}

void Perl_Mob_RangedAttack(Mob* self, Mob* other) // @categories Skills and Recipes, Script Utility
{
	self->RangedAttack(other);
}

void Perl_Mob_ThrowingAttack(Mob* self, Mob* other) // @categories Skills and Recipes, Script Utility
{
	self->ThrowingAttack(other);
}

void Perl_Mob_HealDamage(Mob* self, int64_t amount)  // @categories Script Utility
{
	self->HealDamage(amount);
}

void Perl_Mob_HealDamage(Mob* self, int64_t amount, Mob* caster) // @categories Script Utility
{
	self->HealDamage(amount, caster);
}

void Perl_Mob_SetMaxHP(Mob* self) // @categories Stats and Attributes
{
	self->SetMaxHP();
}

uint32_t Perl_Mob_GetLevelCon(Mob* self, uint8_t other_level) // @categories Stats and Attributes
{
	return self->GetLevelCon(other_level);
}

void Perl_Mob_SetHP(Mob* self, int64_t hp) // @categories Stats and Attributes
{
	self->SetHP(hp);
}

void Perl_Mob_DoAnim(Mob* self, int animation_id) // @categories Script Utility
{
	self->DoAnim(animation_id);
}

void Perl_Mob_DoAnim(Mob* self, int animation_id, int animation_speed) // @categories Script Utility
{
	self->DoAnim(animation_id, animation_speed);
}

void Perl_Mob_DoAnim(Mob* self, int animation_id, int animation_speed, bool ackreq) // @categories Script Utility
{
	self->DoAnim(animation_id, animation_speed, ackreq);
}

void Perl_Mob_DoAnim(Mob* self, int animation_id, int animation_speed, bool ackreq, int filter) // @categories Script Utility
{
	self->DoAnim(animation_id, animation_speed, ackreq, static_cast<eqFilterType>(filter));
}

void Perl_Mob_ChangeSize(Mob* self, float in_size) // @categories Script Utility
{
	self->ChangeSize(in_size);
}

void Perl_Mob_ChangeSize(Mob* self, float in_size, bool unrestricted) // @categories Script Utility
{
	self->ChangeSize(in_size, unrestricted);
}

void Perl_Mob_GMMove(Mob* self, float x, float y, float z) // @categories Script Utility
{
	self->GMMove(x, y, z);
}

void Perl_Mob_GMMove(Mob* self, float x, float y, float z, float heading) // @categories Script Utility
{
	self->GMMove(x, y, z, heading);
}

void Perl_Mob_GMMove(Mob* self, float x, float y, float z, float heading, bool save_guard_spot) // @categories Script Utility
{
	self->GMMove(x, y, z, heading, save_guard_spot);
}

bool Perl_Mob_HasProcs(Mob* self) // @categories Stats and Attributes
{
	return self->HasProcs();
}

bool Perl_Mob_IsInvisible(Mob* self) // @categories Script Utility
{
	return self->IsInvisible();
}

bool Perl_Mob_IsInvisible(Mob* self, Mob* other) // @categories Script Utility
{
	return self->IsInvisible(other);
}

void Perl_Mob_SetInvisible(Mob* self, uint8_t state) // @categories Script Utility
{
	self->SetInvisible(state);
}

void Perl_Mob_SetSeeInvisibleLevel(Mob* self, uint8 see_invis_level) // @categories Script Utility
{
	self->SetInnateSeeInvisible(see_invis_level);
	self->CalcSeeInvisibleLevel();
}

void Perl_Mob_SetSeeInvisibleUndeadLevel(Mob* self, uint8 see_invis_undead_level) // @categories Script Utility
{
	self->SetSeeInvisibleUndead(see_invis_undead_level);
}

bool Perl_Mob_FindBuff(Mob* self, uint16 spell_id) // @categories Spells and Disciplines, Script Utility
{
	return self->FindBuff(spell_id);
}

bool Perl_Mob_FindBuff(Mob* self, uint16 spell_id, uint16 caster_id) // @categories Spells and Disciplines, Script Utility
{
	return self->FindBuff(spell_id, caster_id);
}

int Perl_Mob_FindBuffBySlot(Mob* self, int slot) // @categories Spells and Disciplines, Script Utility
{
	return self->FindBuffBySlot(slot);
}

bool Perl_Mob_FindType(Mob* self, uint16_t type) // @categories Script Utility
{
	return self->FindType(type);
}

bool Perl_Mob_FindType(Mob* self, uint16_t type, bool offensive) // @categories Script Utility
{
	return self->FindType(type, offensive);
}

bool Perl_Mob_FindType(Mob* self, uint16_t type, bool offensive, uint16_t threshold) // @categories Script Utility
{
	return self->FindType(type, offensive, threshold);
}

int Perl_Mob_GetBuffSlotFromType(Mob* self, uint16 type) // @categories Spells and Disciplines, Script Utility
{
	return self->GetBuffSlotFromType(type);
}

void Perl_Mob_MakePet(Mob* self, uint16 spell_id, const char* pet_type) // @categories Pet
{
	self->MakePet(spell_id, pet_type);
}

void Perl_Mob_MakePet(Mob* self, uint16 spell_id, const char* pet_type, const char* name) // @categories Pet
{
	self->MakePet(spell_id, pet_type, name);
}

void Perl_Mob_MakeTempPet(Mob* self, uint16 spell_id) // @categories Pet
{
	self->TemporaryPets(spell_id, nullptr);
}

void Perl_Mob_MakeTempPet(Mob* self, uint16 spell_id, const char* name) // @categories Pet
{
	self->TemporaryPets(spell_id, nullptr, name);
}

void Perl_Mob_MakeTempPet(Mob* self, uint16 spell_id, const char* name, uint32 duration) // @categories Pet
{
	self->TemporaryPets(spell_id, nullptr, name, duration);
}

void Perl_Mob_MakeTempPet(Mob* self, uint16 spell_id, const char* name, uint32 duration, Mob* target) // @categories Pet
{
	self->TemporaryPets(spell_id, target, name, duration);
}

void Perl_Mob_MakeTempPet(Mob* self, uint16 spell_id, const char* name, uint32 duration, Mob* target, bool sticktarg) // @categories Pet
{
	self->TemporaryPets(spell_id, target, name, duration, true, sticktarg);
}

void Perl_Mob_TypesTempPet(Mob* self, uint32 type_id) // @categories Pet
{
	self->TypesTemporaryPets(type_id, nullptr);
}

void Perl_Mob_TypesTempPet(Mob* self, uint32 type_id, const char* name) // @categories Pet
{
	self->TypesTemporaryPets(type_id, nullptr, name);
}

void Perl_Mob_TypesTempPet(Mob* self, uint32 type_id, const char* name, uint32 duration) // @categories Pet
{
	self->TypesTemporaryPets(type_id, nullptr, name, duration);
}

void Perl_Mob_TypesTempPet(Mob* self, uint32 type_id, const char* name, uint32 duration, bool follow) // @categories Pet
{
	self->TypesTemporaryPets(type_id, nullptr, name, duration, follow);
}

void Perl_Mob_TypesTempPet(Mob* self, uint32 type_id, const char* name, uint32 duration, bool follow, Mob* target) // @categories Pet
{
	self->TypesTemporaryPets(type_id, target, name, duration, follow);
}

void Perl_Mob_TypesTempPet(Mob* self, uint32 type_id, const char* name, uint32 duration, bool follow, Mob* target, bool stick_targ) // @categories Pet
{
	self->TypesTemporaryPets(type_id, target, name, duration, follow, stick_targ);
}

int Perl_Mob_GetBaseRace(Mob* self) // @categories Stats and Attributes
{
	return self->GetBaseRace();
}

std::string Perl_Mob_GetBaseRaceName(Mob* self) // @categories Stats and Attributes
{
	return GetRaceIDName(self->GetBaseRace());
}

int Perl_Mob_GetBaseGender(Mob* self) // @categories Stats and Attributes
{
	return self->GetBaseGender();
}

int Perl_Mob_GetDeity(Mob* self) // @categories Stats and Attributes
{
	return self->GetDeity();
}

int Perl_Mob_GetRace(Mob* self) // @categories Stats and Attributes
{
	return self->GetRace();
}

int Perl_Mob_GetGender(Mob* self) // @categories Stats and Attributes
{
	return self->GetGender();
}

int Perl_Mob_GetTexture(Mob* self) // @categories Stats and Attributes
{
	return self->GetTexture();
}

int Perl_Mob_GetHelmTexture(Mob* self) // @categories Stats and Attributes
{
	return self->GetHelmTexture();
}

int Perl_Mob_GetHairColor(Mob* self) // @categories Stats and Attributes
{
	return self->GetHairColor();
}

int Perl_Mob_GetBeardColor(Mob* self) // @categories Stats and Attributes
{
	return self->GetBeardColor();
}

int Perl_Mob_GetEyeColor1(Mob* self) // @categories Stats and Attributes
{
	return self->GetEyeColor1();
}

int Perl_Mob_GetEyeColor2(Mob* self) // @categories Stats and Attributes
{
	return self->GetEyeColor2();
}

int Perl_Mob_GetHairStyle(Mob* self) // @categories Stats and Attributes
{
	return self->GetHairStyle();
}

int Perl_Mob_GetLuclinFace(Mob* self) // @categories Stats and Attributes
{
	return self->GetLuclinFace();
}

int Perl_Mob_GetBeard(Mob* self) // @categories Stats and Attributes
{
	return self->GetBeard();
}

int Perl_Mob_GetDrakkinHeritage(Mob* self) // @categories Stats and Attributes
{
	return self->GetDrakkinHeritage();
}

int Perl_Mob_GetDrakkinTattoo(Mob* self) // @categories Stats and Attributes
{
	return self->GetDrakkinTattoo();
}

int Perl_Mob_GetDrakkinDetails(Mob* self) // @categories Stats and Attributes
{
	return self->GetDrakkinDetails();
}

int Perl_Mob_GetClass(Mob* self) // @categories Stats and Attributes
{
	return self->GetClass();
}

int Perl_Mob_GetLevel(Mob* self) // @categories Stats and Attributes
{
	return self->GetLevel();
}

std::string Perl_Mob_GetCleanName(Mob* self) // @categories Script Utility
{
	return self->GetCleanName();
}

Mob* Perl_Mob_GetTarget(Mob* self) // @categories Script Utility
{
	return self->GetTarget();
}

void Perl_Mob_SetTarget(Mob* self, Mob* target) // @categories Script Utility
{
	self->SetTarget(target);
}

float Perl_Mob_GetHPRatio(Mob* self) // @categories Stats and Attributes
{
	return self->GetHPRatio();
}

bool Perl_Mob_IsWarriorClass(Mob* self) // @categories Script Utility
{
	return self->IsWarriorClass();
}

int64_t Perl_Mob_GetHP(Mob* self) // @categories Stats and Attributes
{
	return self->GetHP();
}

int64_t Perl_Mob_GetMaxHP(Mob* self) // @categories Stats and Attributes
{
	return self->GetMaxHP();
}

int64_t Perl_Mob_GetItemHPBonuses(Mob* self) // @categories Inventory and Items, Stats and Attributes
{
	return self->GetItemHPBonuses();
}

int64_t Perl_Mob_GetSpellHPBonuses(Mob* self) // @categories Spells and Disciplines
{
	return self->GetSpellHPBonuses();
}

int Perl_Mob_GetSpellIDFromSlot(Mob* self, uint8_t slot) // @categories Spells and Disciplines
{
	return slot <= self->GetMaxBuffSlots() ? self->GetSpellIDFromSlot(slot) : -1;
}

int Perl_Mob_GetWalkspeed(Mob* self) // @categories Stats and Attributes
{
	return self->GetWalkspeed();
}

int Perl_Mob_GetRunspeed(Mob* self) // @categories Stats and Attributes
{
	return self->GetRunspeed();
}

int Perl_Mob_GetCasterLevel(Mob* self, uint16_t spell_id) // @categories Stats and Attributes
{
	return self->GetCasterLevel(spell_id);
}

int64_t Perl_Mob_GetMaxMana(Mob* self) // @categories Stats and Attributes
{
	return self->GetMaxMana();
}

int64_t Perl_Mob_GetMana(Mob* self) // @categories Stats and Attributes
{
	return self->GetMana();
}

int64_t Perl_Mob_SetMana(Mob* self, int64_t amount) // @categories Stats and Attributes
{
	return self->SetMana(amount);
}

float Perl_Mob_GetManaRatio(Mob* self) // @categories Stats and Attributes
{
	return self->GetManaRatio();
}

int Perl_Mob_GetAC(Mob* self) // @categories Stats and Attributes
{
	return self->GetAC();
}

int Perl_Mob_GetDisplayAC(Mob* self)
{
	return self->GetDisplayAC();
}

int Perl_Mob_GetATK(Mob* self) // @categories Stats and Attributes
{
	return self->GetATK();
}

int Perl_Mob_GetSTR(Mob* self) // @categories Stats and Attributes
{
	return self->GetSTR();
}

int Perl_Mob_GetSTA(Mob* self) // @categories Stats and Attributes
{
	return self->GetSTA();
}

int Perl_Mob_GetDEX(Mob* self) // @categories Stats and Attributes
{
	return self->GetDEX();
}

int Perl_Mob_GetAGI(Mob* self) // @categories Stats and Attributes
{
	return self->GetAGI();
}

int Perl_Mob_GetINT(Mob* self) // @categories Stats and Attributes
{
	return self->GetINT();
}

int Perl_Mob_GetWIS(Mob* self) // @categories Stats and Attributes
{
	return self->GetWIS();
}

int Perl_Mob_GetCHA(Mob* self) // @categories Stats and Attributes
{
	return self->GetCHA();
}

int Perl_Mob_GetMR(Mob* self) // @categories Stats and Attributes
{
	return self->GetMR();
}

int Perl_Mob_GetFR(Mob* self) // @categories Stats and Attributes
{
	return self->GetFR();
}

int Perl_Mob_GetDR(Mob* self) // @categories Stats and Attributes
{
	return self->GetDR();
}

int Perl_Mob_GetPR(Mob* self) // @categories Stats and Attributes
{
	return self->GetPR();
}

int Perl_Mob_GetCR(Mob* self) // @categories Stats and Attributes
{
	return self->GetCR();
}

int Perl_Mob_GetCorruption(Mob* self) // @categories Stats and Attributes
{
 return self->GetCorrup();
}

int Perl_Mob_GetPhR(Mob* self) // @categories Stats and Attributes
{
	return self->GetPhR();
}

int Perl_Mob_GetMaxSTR(Mob* self) // @categories Stats and Attributes
{
	return self->GetMaxSTR();
}

int Perl_Mob_GetMaxSTA(Mob* self) // @categories Stats and Attributes
{
	return self->GetMaxSTA();
}

int Perl_Mob_GetMaxDEX(Mob* self) // @categories Stats and Attributes
{
	return self->GetMaxDEX();
}

int Perl_Mob_GetMaxAGI(Mob* self) // @categories Stats and Attributes
{
	return self->GetMaxAGI();
}

int Perl_Mob_GetMaxINT(Mob* self) // @categories Stats and Attributes
{
	return self->GetMaxINT();
}

int Perl_Mob_GetMaxWIS(Mob* self) // @categories Stats and Attributes
{
	return self->GetMaxWIS();
}

int Perl_Mob_GetMaxCHA(Mob* self) // @categories Stats and Attributes
{
	return self->GetMaxCHA();
}

float Perl_Mob_GetActSpellRange(Mob* self, uint16 spell_id, float range) // @categories Spells and Disciplines
{
	return self->GetActSpellRange(spell_id, range);
}

int64_t Perl_Mob_GetActSpellDamage(Mob* self, uint16 spell_id, int64 value) // @categories Spells and Disciplines
{
	return self->GetActSpellDamage(spell_id, value);
}

int64_t Perl_Mob_GetActSpellDamage(Mob* self, uint16 spell_id, int64 value, Mob* target) // @categories Spells and Disciplines
{
	return self->GetActSpellDamage(spell_id, value, target);
}

int64_t Perl_Mob_GetActDoTDamage(Mob* self, uint16 spell_id, int64 value, Mob* target) // @categories Spells and Disciplines
{
	return self->GetActDoTDamage(spell_id, value, target);
}

int64_t Perl_Mob_GetActDoTDamage(Mob* self, uint16 spell_id, int64 value, Mob* target, bool from_buff_tic) // @categories Spells and Disciplines
{
	return self->GetActDoTDamage(spell_id, value, target, from_buff_tic);
}

int64_t Perl_Mob_GetActSpellHealing(Mob* self, uint16 spell_id, int64 value) // @categories Spells and Disciplines
{
	return self->GetActSpellHealing(spell_id, value);
}

int64_t Perl_Mob_GetActSpellHealing(Mob* self, uint16 spell_id, int64 value, Mob* target) // @categories Spells and Disciplines
{
	return self->GetActSpellHealing(spell_id, value, target);
}

int64_t Perl_Mob_GetActSpellHealing(Mob* self, uint16 spell_id, int64 value, Mob* target, bool from_buff_tic) // @categories Spells and Disciplines
{
	return self->GetActSpellHealing(spell_id, value, target, from_buff_tic);
}

int Perl_Mob_GetActSpellCost(Mob* self, uint16 spell_id, int32 cost) // @categories Spells and Disciplines
{
	return self->GetActSpellCost(spell_id, cost);
}

int Perl_Mob_GetActSpellDuration(Mob* self, uint16 spell_id, int32 duration) // @categories Spells and Disciplines
{
	return self->GetActSpellDuration(spell_id, duration);
}

int Perl_Mob_GetActSpellCasttime(Mob* self, uint16 spell_id, uint32 cast_time) // @categories Spells and Disciplines
{
	return self->GetActSpellCasttime(spell_id, cast_time);
}

int64 Perl_Mob_GetActReflectedSpellDamage(Mob* self, uint16 spell_id, int64 value, int effectiveness) // @categories Spells and Disciplines
{
	return self->GetActReflectedSpellDamage(spell_id, value, effectiveness);
}

float Perl_Mob_ResistSpell(Mob* self, uint8 resist_type, uint16 spell_id, Mob* caster) // @categories Spells and Disciplines, Script Utility
{
	return self->ResistSpell(resist_type, spell_id, caster);
}

int Perl_Mob_GetSpecializeSkillValue(Mob* self, uint16 spell_id) // @categories Skills and Recipes, Spells and Disciplines
{
	return self->GetSpecializeSkillValue(spell_id);
}

uint32_t Perl_Mob_GetNPCTypeID(Mob* self) // @categories Script Utility
{
	return self->GetNPCTypeID();
}

bool Perl_Mob_IsTargeted(Mob* self) // @categories Script Utility
{
	return self->IsTargeted();
}

float Perl_Mob_GetX(Mob* self) // @categories Script Utility
{
	return self->GetX();
}

float Perl_Mob_GetY(Mob* self) // @categories Script Utility
{
	return self->GetY();
}

float Perl_Mob_GetZ(Mob* self) // @categories Script Utility
{
	return self->GetZ();
}

float Perl_Mob_GetHeading(Mob* self) // @categories Script Utility
{
	return self->GetHeading();
}

float Perl_Mob_GetWaypointX(Mob* self) // @categories Script Utility
{
	return self->GetCurrentWayPoint().x;
}

float Perl_Mob_GetWaypointY(Mob* self) // @categories Script Utility
{
	return self->GetCurrentWayPoint().y;
}

float Perl_Mob_GetWaypointZ(Mob* self) // @categories Script Utility
{
	return self->GetCurrentWayPoint().z;
}

float Perl_Mob_GetWaypointH(Mob* self) // @categories Script Utility
{
	return self->GetCurrentWayPoint().w;
}

float Perl_Mob_GetWaypointPause(Mob* self) // @categories Script Utility
{
	return self->GetCWPP();
}

int Perl_Mob_GetWaypointID(Mob* self) // @categories Script Utility
{
	return self->GetCWP();
}

void Perl_Mob_SetCurrentWP(Mob* self, int waypoint) // @categories Script Utility
{
	self->SetCurrentWP(waypoint);
}

float Perl_Mob_GetSize(Mob* self) // @categories Stats and Attributes
{
	return self->GetSize();
}

void Perl_Mob_SetFollowID(Mob* self, uint32_t id) // @categories Script Utility
{
	self->SetFollowID(id);
}

uint32_t Perl_Mob_GetFollowID(Mob* self) // @categories Script Utility
{
	return self->GetFollowID();
}

void Perl_Mob_Message(Mob* self, uint32 type, const char* message) // @categories Script Utility
{
	if (RuleB(Chat, QuestDialogueUsesDialogueWindow) && self->IsClient()) {
		DialogueWindow::Render(self->CastToClient(), message);
	}
	else if (RuleB(Chat, AutoInjectSaylinksToClientMessage)) {
		std::string new_message = EQ::SayLinkEngine::InjectSaylinksIfNotExist(message);
		self->Message(type, new_message.c_str());
	}
	else {
		self->Message(type, message);
	}
}

void Perl_Mob_Message_StringID(Mob* self, uint32 type, uint32 string_id) // @categories Script Utility
{
	self->MessageString(type, string_id);
}

void Perl_Mob_Message_StringID(Mob* self, uint32 type, uint32 string_id, uint32 distance) // @categories Script Utility
{
	self->MessageString(type, string_id, distance);
}

void Perl_Mob_Say(Mob* self, const char* message) // @categories Script Utility
{
	self->Say(message);
}

void Perl_Mob_Shout(Mob* self, const char* message) // @categories Script Utility
{
	self->Shout(message);
}

void Perl_Mob_Emote(Mob* self, const char* message) // @categories Script Utility
{
	self->Emote(message);
}

void Perl_Mob_InterruptSpell(Mob* self) // @categories Script Utility
{
	self->InterruptSpell();
}

void Perl_Mob_InterruptSpell(Mob* self, uint16 spell_id) // @categories Script Utility
{
	self->InterruptSpell(spell_id);
}

void Perl_Mob_CastSpell(Mob* self, uint16 spell_id, uint16 target_id) // @categories Spells and Disciplines
{
	self->CastSpell(spell_id, target_id);
}

void Perl_Mob_CastSpell(Mob* self, uint16 spell_id, uint16 target_id, int slot) // @categories Spells and Disciplines
{
	self->CastSpell(spell_id, target_id, static_cast<EQ::spells::CastingSlot>(slot));
}

void Perl_Mob_CastSpell(Mob* self, uint16 spell_id, uint16 target_id, int slot, int cast_time) // @categories Spells and Disciplines
{
	self->CastSpell(spell_id, target_id, static_cast<EQ::spells::CastingSlot>(slot), cast_time);
}

void Perl_Mob_CastSpell(Mob* self, uint16 spell_id, uint16 target_id, int slot, int cast_time, int mana_cost) // @categories Spells and Disciplines
{
	self->CastSpell(spell_id, target_id, static_cast<EQ::spells::CastingSlot>(slot), cast_time, mana_cost);
}

// args differ from lua api (item_slot, timer, timer_duration not supported)
void Perl_Mob_CastSpell(Mob* self, uint16 spell_id, uint16 target_id, int slot, int cast_time, int mana_cost, int16 resist_adjust) // @categories Spells and Disciplines
{
	int16 res = resist_adjust;
	self->CastSpell(spell_id, target_id, static_cast<EQ::spells::CastingSlot>(slot), cast_time, mana_cost, nullptr, 0xFFFFFFFF, 0xFFFFFFFF, 0, &res);
}

// these don't match lua api for default resist difficulty
void Perl_Mob_SpellFinished(Mob* self, uint16 spell_id) // @categories Spells and Disciplines
{
	self->SpellFinished(spell_id, self, EQ::spells::CastingSlot::Item, 0, -1, spells[spell_id].resist_difficulty);
}

void Perl_Mob_SpellFinished(Mob* self, uint16 spell_id, Mob* target) // @categories Spells and Disciplines
{
	self->SpellFinished(spell_id, target, EQ::spells::CastingSlot::Item, 0, -1, spells[spell_id].resist_difficulty);
}

void Perl_Mob_SpellFinished(Mob* self, uint16 spell_id, Mob* target, int32 mana_cost) // @categories Spells and Disciplines
{
	self->SpellFinished(spell_id, target, EQ::spells::CastingSlot::Item, mana_cost, -1, spells[spell_id].resist_difficulty);
}

void Perl_Mob_SpellFinished(Mob* self, uint16 spell_id, Mob* target, int32 mana_cost, uint16 resist_diff) // @categories Spells and Disciplines
{
	self->SpellFinished(spell_id, target, EQ::spells::CastingSlot::Item, mana_cost, -1, resist_diff);
}

bool Perl_Mob_IsImmuneToSpell(Mob* self, uint16 spell_id, Mob* caster) // @categories Spells and Disciplines, Script Utility
{
	return self->IsImmuneToSpell(spell_id, caster);
}

void Perl_Mob_BuffFadeBySpellID(Mob* self, uint16 spell_id) // @categories Script Utility, Spells and Disciplines
{
	self->BuffFadeBySpellID(spell_id);
}

void Perl_Mob_BuffFadeByEffect(Mob* self, int effect_id) // @categories Script Utility, Spells and Disciplines
{
	self->BuffFadeByEffect(effect_id);
}

void Perl_Mob_BuffFadeByEffect(Mob* self, int effect_id, int skip_slot) // @categories Script Utility, Spells and Disciplines
{
	self->BuffFadeByEffect(effect_id, skip_slot);
}

void Perl_Mob_BuffFadeAll(Mob* self) // @categories Script Utility, Spells and Disciplines
{
	self->BuffFadeAll();
}

void Perl_Mob_BuffFadeBySlot(Mob* self, int slot) // @categories Script Utility, Spells and Disciplines
{
	self->BuffFadeBySlot(slot);
}

void Perl_Mob_BuffFadeBySlot(Mob* self, int slot, bool recalc_bonuses) // @categories Script Utility, Spells and Disciplines
{
	self->BuffFadeBySlot(slot, recalc_bonuses);
}

bool Perl_Mob_CanBuffStack(Mob* self, uint16 spell_id, uint8 caster_level) // @categories Script Utility, Spells and Disciplines
{
	return self->CanBuffStack(spell_id, caster_level);
}

bool Perl_Mob_CanBuffStack(Mob* self, uint16 spell_id, uint8 caster_level, bool fail_if_overwritten) // @categories Script Utility, Spells and Disciplines
{
	return self->CanBuffStack(spell_id, caster_level, fail_if_overwritten);
}

bool Perl_Mob_IsCasting(Mob* self) // @categories Script Utility
{
	return self->IsCasting();
}

int Perl_Mob_CastingSpellID(Mob* self) // @categories Spells and Disciplines
{
	return self->CastingSpellID();
}

void Perl_Mob_SetAppearance(Mob* self, int app) // @categories Stats and Attributes
{
	self->SetAppearance(static_cast<EmuAppearance>(app));
}

void Perl_Mob_SetAppearance(Mob* self, int app, bool ignore_self) // @categories Stats and Attributes
{
	self->SetAppearance(static_cast<EmuAppearance>(app), ignore_self);
}

int Perl_Mob_GetAppearance(Mob* self) // @categories Stats and Attributes
{
	return self->GetAppearance();
}

int Perl_Mob_GetRunAnimSpeed(Mob* self) // @categories Stats and Attributes
{
	return self->GetRunAnimSpeed();
}

void Perl_Mob_SetRunAnimSpeed(Mob* self, int8 speed) // @categories Stats and Attributes
{
	self->SetRunAnimSpeed(speed);
}

void Perl_Mob_SetPetID(Mob* self, uint16 new_pet_id) // @categories Pet
{
	self->SetPetID(new_pet_id);
}

Mob* Perl_Mob_GetPet(Mob* self) // @categories Script Utility, Pet
{
	return self->GetPet();
}

int Perl_Mob_GetPetID(Mob* self) // @categories Script Utility, Pet
{
	return self->GetPetID();
}

void Perl_Mob_SetOwnerID(Mob* self, uint16 new_owner_id) // @categories Pet
{
	self->SetOwnerID(new_owner_id);
}

uint16 Perl_Mob_GetOwnerID(Mob* self) // @categories Script Utility, Pet
{
	return self->GetOwnerID();
}

uint8 Perl_Mob_GetPetType(Mob* self) // @categories Script Utility, Pet
{
	return self->GetPetType();
}

std::string Perl_Mob_GetPetTypeName(Mob* self) // @categories Script Utility, Pet
{
	return PetType::GetName(self->GetPetType());
}

int Perl_Mob_GetBodyType(Mob* self) // @categories Stats and Attributes
{
	return self->GetBodyType();
}

void Perl_Mob_Stun(Mob* self, int duration)
{
	self->Stun(duration);
}

void Perl_Mob_Spin(Mob* self) // @categories Script Utility
{
	self->Spin();
}

void Perl_Mob_Kill(Mob* self) // @categories Script Utility
{
	self->Kill();
}

void Perl_Mob_SetInvul(Mob* self, bool value) // @categories Script Utility
{
	self->SetInvul(value);
}

bool Perl_Mob_GetInvul(Mob* self) // @categories Script Utility, Stats and Attributes
{
	return self->GetInvul();
}

void Perl_Mob_SetExtraHaste(Mob* self, int haste) // @categories Script Utility, Stats and Attributes
{
	self->SetExtraHaste(haste);
}

void Perl_Mob_SetExtraHaste(Mob* self, int haste, bool need_to_save) // @categories Script Utility, Stats and Attributes
{
	self->SetExtraHaste(haste, need_to_save);
}

int Perl_Mob_GetExtraHaste(Mob* self) // @categories Script Utility, Stats and Attributes
{
	return self->GetExtraHaste();
}

int Perl_Mob_GetHaste(Mob* self) // @categories Stats and Attributes
{
	return self->GetHaste();
}

int Perl_Mob_GetHandToHandDamage(Mob* self) // @categories Stats and Attributes
{
	return self->GetHandToHandDamage();
}

bool Perl_Mob_CanThisClassDoubleAttack(Mob* self) // @categories Skills and Recipes
{
	return self->CanThisClassDoubleAttack();
}

bool Perl_Mob_CanThisClassDualWield(Mob* self) // @categories Skills and Recipes
{
	return self->CanThisClassDualWield();
}

bool Perl_Mob_CanThisClassRiposte(Mob* self) // @categories Skills and Recipes
{
	return self->CanThisClassRiposte();
}

bool Perl_Mob_CanThisClassDodge(Mob* self) // @categories Skills and Recipes
{
	return self->CanThisClassDodge();
}

bool Perl_Mob_CanThisClassParry(Mob* self) // @categories Skills and Recipes
{
	return self->CanThisClassParry();
}

int Perl_Mob_GetHandToHandDelay(Mob* self) // @categories Stats and Attributes
{
	return self->GetHandToHandDelay();
}

int Perl_Mob_GetClassLevelFactor(Mob* self) // @categories Stats and Attributes
{
	return self->GetClassLevelFactor();
}

void Perl_Mob_Mesmerize(Mob* self) // @categories Script Utility
{
	self->Mesmerize();
}

bool Perl_Mob_IsMezzed(Mob* self) // @categories Script Utility
{
	return self->IsMezzed();
}

bool Perl_Mob_IsStunned(Mob* self) // @categories Script Utility
{
	return self->IsStunned();
}

void Perl_Mob_StartEnrage(Mob* self) // @categories Script Utility
{
	self->StartEnrage();
}

bool Perl_Mob_IsEnraged(Mob* self) // @categories Script Utility
{
	return self->IsEnraged();
}

int Perl_Mob_GetReverseFactionCon(Mob* self, Mob* other) // @categories Faction
{
	return self->GetReverseFactionCon(other);
}

bool Perl_Mob_IsAIControlled(Mob* self) // @categories Script Utility
{
	return self->IsAIControlled();
}

float Perl_Mob_GetAggroRange(Mob* self) // @categories Stats and Attributes, Hate and Aggro
{
	return self->GetAggroRange();
}

float Perl_Mob_GetAssistRange(Mob* self) // @categories Stats and Attributes, Hate and Aggro
{
	return self->GetAssistRange();
}

void Perl_Mob_SetPetOrder(Mob* self, uint8 pet_order) // @categories Pet
{
	self->SetPetOrder(pet_order);
}

uint8 Perl_Mob_GetPetOrder(Mob* self) // @categories Script Utility, Pet
{
	return self->GetPetOrder();
}

bool Perl_Mob_IsRoamer(Mob* self) // @categories Script Utility, Spawns
{
	return self->IsRoamer();
}

bool Perl_Mob_IsRooted(Mob* self) // @categories Script Utility
{
	return self->IsRooted();
}

void Perl_Mob_AddToHateList(Mob* self, Mob* other) // @categories Hate and Aggro
{
	self->AddToHateList(other);
}

void Perl_Mob_AddToHateList(Mob* self, Mob* other, int64_t hate) // @categories Hate and Aggro
{
	self->AddToHateList(other, hate);
}

void Perl_Mob_AddToHateList(Mob* self, Mob* other, int64_t hate, int64_t damage) // @categories Hate and Aggro
{
	self->AddToHateList(other, hate, damage);
}

void Perl_Mob_AddToHateList(Mob* self, Mob* other, int64_t hate, int64_t damage, bool yell_for_help) // @categories Hate and Aggro
{
	self->AddToHateList(other, hate, damage, yell_for_help);
}

void Perl_Mob_AddToHateList(Mob* self, Mob* other, int64_t hate, int64_t damage, bool yell_for_help, bool frenzy) // @categories Hate and Aggro
{
	self->AddToHateList(other, hate, damage, yell_for_help, frenzy);
}

void Perl_Mob_AddToHateList(Mob* self, Mob* other, int64_t hate, int64_t damage, bool yell_for_help, bool frenzy, bool buff_tic) // @categories Hate and Aggro
{
	self->AddToHateList(other, hate, damage, yell_for_help, frenzy, buff_tic);
}

void Perl_Mob_SetHate(Mob* self, Mob* other) // @categories Hate and Aggro
{
	self->SetHateAmountOnEnt(other);
}

void Perl_Mob_SetHate(Mob* self, Mob* other, int64_t hate) // @categories Hate and Aggro
{
	self->SetHateAmountOnEnt(other, hate);
}

void Perl_Mob_SetHate(Mob* self, Mob* other, int64_t hate, int64_t damage) // @categories Hate and Aggro
{
	self->SetHateAmountOnEnt(other, hate, damage);
}

void Perl_Mob_HalveAggro(Mob* self, Mob* other) // @categories Hate and Aggro
{
	self->HalveAggro(other);
}

void Perl_Mob_DoubleAggro(Mob* self, Mob* other) // @categories Hate and Aggro
{
	self->DoubleAggro(other);
}

int64_t Perl_Mob_GetHateAmount(Mob* self, Mob* target) // @categories Hate and Aggro
{
	return self->GetHateAmount(target);
}

int64_t Perl_Mob_GetHateAmount(Mob* self, Mob* target, bool is_damage) // @categories Hate and Aggro
{
	return self->GetHateAmount(target, is_damage);
}

int64_t Perl_Mob_GetDamageAmount(Mob* self, Mob* target) // @categories Stats and Attributes
{
	return self->GetDamageAmount(target);
}

Mob* Perl_Mob_GetHateTop(Mob* self) // @categories Hate and Aggro
{
	return self->GetHateTop();
}

Bot* Perl_Mob_GetHateTopBot(Mob* self) // @categories Hate and Aggro
{
	return self->GetHateTopBot();
}

Client* Perl_Mob_GetHateTopClient(Mob* self) // @categories Hate and Aggro
{
	return self->GetHateTopClient();
}

NPC* Perl_Mob_GetHateTopNPC(Mob* self) // @categories Hate and Aggro
{
	return self->GetHateTopNPC();
}

Mob* Perl_Mob_GetHateDamageTop(Mob* self, Mob* other) // @categories Hate and Aggro
{
	return self->GetHateDamageTop(other);
}

Mob* Perl_Mob_GetHateRandom(Mob* self) // @categories Hate and Aggro
{
	return self->GetHateRandom();
}

bool Perl_Mob_IsEngaged(Mob* self) // @categories Script Utility
{
	return self->IsEngaged();
}

bool Perl_Mob_HateSummon(Mob* self)  // @categories Hate and Aggro
{
	return self->HateSummon();
}

void Perl_Mob_FaceTarget(Mob* self) // @categories Script Utility
{
	self->FaceTarget();
}

void Perl_Mob_FaceTarget(Mob* self, Mob* target) // @categories Script Utility
{
	self->FaceTarget(target);
}

void Perl_Mob_SetHeading(Mob* self, float heading) // @categories Script Utility
{
	self->SetHeading(heading);
}

void Perl_Mob_WipeHateList(Mob* self) // @categories Hate and Aggro
{
	self->WipeHateList();
}

bool Perl_Mob_CheckAggro(Mob* self, Mob* other) // @categories Hate and Aggro
{
	return self->CheckAggro(other);
}

float Perl_Mob_CalculateHeadingToTarget(Mob* self, float x, float y) // @categories Script Utility
{
	return self->CalculateHeadingToTarget(x, y);
}

void Perl_Mob_RunTo(Mob* self, float x, float y, float z)
{
	self->RunTo(x, y, z);
}

void Perl_Mob_WalkTo(Mob* self, float x, float y, float z)
{
	self->WalkTo(x, y, z);
}

void Perl_Mob_NavigateTo(Mob* self, float x, float y, float z) // @categories Script Utility
{
	self->NavigateTo(x, y, z);
}

void Perl_Mob_StopNavigation(Mob* self)
{
	self->StopNavigation();
}

float Perl_Mob_CalculateDistance(Mob* self, float x, float y, float z) // @categories Script Utility
{
	return self->CalculateDistance(x, y, z);
}

float Perl_Mob_CalculateDistance(Mob* self, Mob* mob) // @categories Script Utility
{
	return self->CalculateDistance(mob);
}

void Perl_Mob_SendTo(Mob* self, float new_x, float new_y, float new_z) // @categories Script Utility
{
	self->SendTo(new_x, new_y, new_z);
}

void Perl_Mob_SendToFixZ(Mob* self, float new_x, float new_y, float new_z) // @categories Script Utility
{
	self->SendToFixZ(new_x, new_y, new_z);
}

void Perl_Mob_NPCSpecialAttacks(Mob* self, const char* abilities_string, int perm_tag) // @categories Stats and Attributes
{
	self->NPCSpecialAttacks(abilities_string, perm_tag);
}

void Perl_Mob_NPCSpecialAttacks(Mob* self, const char* abilities_string, int perm_tag, bool reset) // @categories Stats and Attributes
{
	self->NPCSpecialAttacks(abilities_string, perm_tag, reset);
}

void Perl_Mob_NPCSpecialAttacks(Mob* self, const char* abilities_string, int perm_tag, bool reset, bool remove) // @categories Stats and Attributes
{
	self->NPCSpecialAttacks(abilities_string, perm_tag, reset, remove);
}

uint32_t Perl_Mob_DontHealMeBefore(Mob* self) // @categories Script Utility
{
	return self->DontHealMeBefore();
}

uint32_t Perl_Mob_DontBuffMeBefore(Mob* self) // @categories Script Utility
{
	return self->DontBuffMeBefore();
}

uint32_t Perl_Mob_DontDotMeBefore(Mob* self) // @categories Script Utility
{
	return self->DontDotMeBefore();
}

uint32_t Perl_Mob_DontRootMeBefore(Mob* self) // @categories Script Utility
{
	return self->DontRootMeBefore();
}

uint32_t Perl_Mob_DontSnareMeBefore(Mob* self) // @categories Script Utility
{
	return self->DontSnareMeBefore();
}

int Perl_Mob_GetResist(Mob* self, uint8_t type) // @categories Stats and Attributes
{
	return self->GetResist(type);
}

bool Perl_Mob_Charmed(Mob* self) // @categories Script Utility
{
	return self->Charmed();
}

uint32_t Perl_Mob_GetLevelHP(Mob* self, uint8 level) // @categories Stats and Attributes
{
	return self->GetLevelHP(level);
}

uint32_t Perl_Mob_GetZoneID(Mob* self) // @categories Zones
{
	return self->GetZoneID();
}

int Perl_Mob_CheckAggroAmount(Mob* self, uint16 spell_id) // @categories Hate and Aggro
{
	return self->CheckAggroAmount(spell_id, nullptr);
}

int Perl_Mob_CheckHealAggroAmount(Mob* self, uint16 spell_id) // @categories Hate and Aggro
{
	return self->CheckHealAggroAmount(spell_id, nullptr);
}

int Perl_Mob_CheckHealAggroAmount(Mob* self, uint16 spell_id, uint32 possible) // @categories Hate and Aggro
{
	return self->CheckHealAggroAmount(spell_id, nullptr, possible);
}

uint32_t Perl_Mob_GetAA(Mob* self, uint32 rank_id) // @categories Alternative Advancement
{
	return self->GetAA(rank_id);
}

uint32_t Perl_Mob_GetAAByAAID(Mob* self, uint32 aa_id) // @categories Alternative Advancement
{
	return self->GetAAByAAID(aa_id);
}

bool Perl_Mob_SetAA(Mob* self, int aa_id, int points) // @categories Alternative Advancement, Script Utility
{
	return self->SetAA(aa_id, points);
}

bool Perl_Mob_SetAA(Mob* self, int aa_id, int points, int charges) // @categories Alternative Advancement, Script Utility
{
	return self->SetAA(aa_id, points, charges);
}

bool Perl_Mob_DivineAura(Mob* self) // @categories Spells and Disciplines
{
	return self->DivineAura();
}

void Perl_Mob_AddFeignMemory(Mob* self, Client* attacker) // @categories Script Utility
{
	self->AddFeignMemory(attacker);
}

void Perl_Mob_RemoveFromFeignMemory(Mob* self, Client* attacker) // @categories Script Utility, Hate and Aggro
{
	self->RemoveFromFeignMemory(attacker);
}

void Perl_Mob_ClearFeignMemory(Mob* self) // @categories Script Utility, Hate and Aggro
{
	self->ClearFeignMemory();
}

void Perl_Mob_SetOOCRegen(Mob* self, int64 new_ooc_regen) // @categories Stats and Attributes
{
	self->SetOOCRegen(new_ooc_regen);
}

perl::array Perl_Mob_GetHateList(Mob* self)
{
	perl::array result;
	auto hate_list = self->GetHateList();
	for (struct_HateList* entry : hate_list)
	{
		result.push_back(entry);
	}
	return result;
}

void Perl_Mob_SignalClient(Mob* self, Client* client, int signal_id) // @categories Script Utility
{
	client->Signal(signal_id);
}

bool Perl_Mob_CombatRange(Mob* self, Mob* target) // @categories Script Utility
{
	return self->CombatRange(target);
}

void Perl_Mob_DoSpecialAttackDamage(Mob* self, Mob* other, int skill, int max_damage) // @categories Script Utility, Skills and Attributes
{
	self->DoSpecialAttackDamage(other, static_cast<EQ::skills::SkillType>(skill), max_damage);
}

void Perl_Mob_DoSpecialAttackDamage(Mob* self, Mob* other, int skill, int max_damage, int min_damage) // @categories Script Utility, Skills and Attributes
{
	self->DoSpecialAttackDamage(other, static_cast<EQ::skills::SkillType>(skill), max_damage, min_damage);
}

void Perl_Mob_DoSpecialAttackDamage(Mob* self, Mob* other, int skill, int max_damage, int min_damage, int hate_override) // @categories Script Utility, Skills and Attributes
{
	self->DoSpecialAttackDamage(other, static_cast<EQ::skills::SkillType>(skill), max_damage, min_damage, hate_override);
}

void Perl_Mob_DoSpecialAttackDamage(Mob* self, Mob* other, int skill, int max_damage, int min_damage, int hate_override, int reuse_time) // @categories Script Utility, Skills and Attributes
{
	self->DoSpecialAttackDamage(other, static_cast<EQ::skills::SkillType>(skill), max_damage, min_damage, hate_override, reuse_time);
}

bool Perl_Mob_CheckLoS(Mob* self, Mob* other) // @categories Script Utility
{
	return self->CheckLosFN(other);
}

bool Perl_Mob_CheckLoSToLoc(Mob* self, float x, float y, float z)
{
	return self->CheckLosFN(x, y, z, 6.0f);
}

bool Perl_Mob_CheckLoSToLoc(Mob* self, float x, float y, float z, float mob_size) // @categories Script Utility
{
	return self->CheckLosFN(x, y, z, mob_size);
}

float Perl_Mob_FindGroundZ(Mob* self, float x, float y)
{
	return self->GetGroundZ(x, y);
}

float Perl_Mob_FindGroundZ(Mob* self, float x, float y, float z_offset) // @categories Script Utility
{
	return self->GetGroundZ(x, y, z_offset);
}

void Perl_Mob_ProjectileAnim(Mob* self, Mob* to, int item_id) // @categories Script Utility
{
	self->ProjectileAnimation(to, item_id);
}

void Perl_Mob_ProjectileAnim(Mob* self, Mob* to, int item_id, bool is_arrow) // @categories Script Utility
{
	self->ProjectileAnimation(to, item_id, is_arrow);
}

void Perl_Mob_ProjectileAnim(Mob* self, Mob* to, int item_id, bool is_arrow, float speed) // @categories Script Utility
{
	self->ProjectileAnimation(to, item_id, is_arrow, speed);
}

void Perl_Mob_ProjectileAnim(Mob* self, Mob* to, int item_id, bool is_arrow, float speed, float angle) // @categories Script Utility
{
	self->ProjectileAnimation(to, item_id, is_arrow, speed, angle);
}

void Perl_Mob_ProjectileAnim(Mob* self, Mob* to, int item_id, bool is_arrow, float speed, float angle, float tilt) // @categories Script Utility
{
	self->ProjectileAnimation(to, item_id, is_arrow, speed, angle, tilt);
}

void Perl_Mob_ProjectileAnim(Mob* self, Mob* to, int item_id, bool is_arrow, float speed, float angle, float tilt, float arc) // @categories Script Utility
{
	self->ProjectileAnimation(to, item_id, is_arrow, speed, angle, tilt, arc);
}

void Perl_Mob_ProjectileAnim(Mob* self, Mob* to, int item_id, bool is_arrow, float speed, float angle, float tilt, float arc, const char* idfile) // @categories Script Utility
{
	self->ProjectileAnimation(to, item_id, is_arrow, speed, angle, tilt, arc, idfile);
}

bool Perl_Mob_HasNPCSpecialAtk(Mob* self, const char* ability_string) // @categories Stats and Attributes
{
	return self->HasNPCSpecialAtk(ability_string);
}

// todo: SendAppearanceEffect should be changed to use a table for arguments
void Perl_Mob_SendAppearanceEffect(Mob* self, int32 parm1) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, 0, 0, 0, 0);
}

void Perl_Mob_SendAppearanceEffect(Mob* self, int32 parm1, int32 parm2) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, 0, 0, 0);
}

void Perl_Mob_SendAppearanceEffect(Mob* self, int32 parm1, int32 parm2, int32 parm3) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, parm3, 0, 0);
}

void Perl_Mob_SendAppearanceEffect(Mob* self, int32 parm1, int32 parm2, int32 parm3, int32 parm4) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, parm3, parm4, 0);
}

void Perl_Mob_SendAppearanceEffect(Mob* self, int32 parm1, int32 parm2, int32 parm3, int32 parm4, int32 parm5) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, parm3, parm4, parm5);
}

void Perl_Mob_SendAppearanceEffect(Mob* self, int32 parm1, int32 parm2, int32 parm3, int32 parm4, int32 parm5, perl::nullable<Client*> client) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, parm3, parm4, parm5, client.get());
}

void Perl_Mob_SendAppearanceEffect(Mob* self, int32 parm1, int32 parm2, int32 parm3, int32 parm4, int32 parm5, perl::nullable<Client*> client, uint32 slot1) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, parm3, parm4, parm5, client.get(), slot1);
}

void Perl_Mob_SendAppearanceEffect(Mob* self, int32 parm1, int32 parm2, int32 parm3, int32 parm4, int32 parm5, perl::nullable<Client*> client, uint32 slot1, uint32 ground1) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, parm3, parm4, parm5, client.get(), slot1, ground1);
}

void Perl_Mob_SendAppearanceEffect(Mob* self, int32 parm1, int32 parm2, int32 parm3, int32 parm4, int32 parm5, perl::nullable<Client*> client, uint32 slot1, uint32 ground1, uint32 slot2) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, parm3, parm4, parm5, client.get(), slot1, ground1, slot2);
}

void Perl_Mob_SendAppearanceEffect(Mob* self, int32 parm1, int32 parm2, int32 parm3, int32 parm4, int32 parm5, perl::nullable<Client*> client, uint32 slot1, uint32 ground1, uint32 slot2, uint32 ground2) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, parm3, parm4, parm5, client.get(), slot1, ground1, slot2, ground2);
}

void Perl_Mob_SendAppearanceEffect(Mob* self, int32 parm1, int32 parm2, int32 parm3, int32 parm4, int32 parm5, perl::nullable<Client*> client, uint32 slot1, uint32 ground1, uint32 slot2, uint32 ground2, uint32 slot3) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, parm3, parm4, parm5, client.get(), slot1, ground1, slot2, ground2, slot3);
}

void Perl_Mob_SendAppearanceEffect(Mob* self, int32 parm1, int32 parm2, int32 parm3, int32 parm4, int32 parm5, perl::nullable<Client*> client, uint32 slot1, uint32 ground1, uint32 slot2, uint32 ground2, uint32 slot3, uint32 ground3) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, parm3, parm4, parm5, client.get(), slot1, ground1, slot2, ground2, slot3, ground3);
}

void Perl_Mob_SendAppearanceEffect(Mob* self, int32 parm1, int32 parm2, int32 parm3, int32 parm4, int32 parm5, perl::nullable<Client*> client, uint32 slot1, uint32 ground1, uint32 slot2, uint32 ground2, uint32 slot3, uint32 ground3, uint32 slot4) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, parm3, parm4, parm5, client.get(), slot1, ground1, slot2, ground2, slot3, ground3, slot4);
}

void Perl_Mob_SendAppearanceEffect(Mob* self, int32 parm1, int32 parm2, int32 parm3, int32 parm4, int32 parm5, perl::nullable<Client*> client, uint32 slot1, uint32 ground1, uint32 slot2, uint32 ground2, uint32 slot3, uint32 ground3, uint32 slot4, uint32 ground4) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, parm3, parm4, parm5, client.get(), slot1, ground1, slot2, ground2, slot3, ground3, slot4, ground4);
}

void Perl_Mob_SendAppearanceEffect(Mob* self, int32 parm1, int32 parm2, int32 parm3, int32 parm4, int32 parm5, perl::nullable<Client*> client, uint32 slot1, uint32 ground1, uint32 slot2, uint32 ground2, uint32 slot3, uint32 ground3, uint32 slot4, uint32 ground4, uint32 slot5) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, parm3, parm4, parm5, client.get(), slot1, ground1, slot2, ground2, slot3, ground3, slot4, ground4, slot5);
}

void Perl_Mob_SendAppearanceEffect(Mob* self, int32 parm1, int32 parm2, int32 parm3, int32 parm4, int32 parm5, perl::nullable<Client*> client, uint32 slot1, uint32 ground1, uint32 slot2, uint32 ground2, uint32 slot3, uint32 ground3, uint32 slot4, uint32 ground4, uint32 slot5, uint32 ground5) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, parm3, parm4, parm5, client.get(), slot1, ground1, slot2, ground2, slot3, ground3, slot4, ground4, slot5, ground5);
}

// todo: SendAppearanceEffectActor should be changed to use a table for arguments
void Perl_Mob_SendAppearanceEffectActor(Mob* self, int32 parm1, uint32 slot1) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, 0, 0, 0, 0, nullptr, slot1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

void Perl_Mob_SendAppearanceEffectActor(Mob* self, int32 parm1, uint32 slot1, int32 parm2) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, 0, 0, 0, nullptr, slot1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

void Perl_Mob_SendAppearanceEffectActor(Mob* self, int32 parm1, uint32 slot1, int32 parm2, uint32 slot2) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, 0, 0, 0, nullptr, slot1, 0, slot2, 0, 0, 0, 0, 0, 0, 0);
}

void Perl_Mob_SendAppearanceEffectActor(Mob* self, int32 parm1, uint32 slot1, int32 parm2, uint32 slot2, int32 parm3) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, parm3, 0, 0, nullptr, slot1, 0, slot2, 0, 0, 0, 0, 0, 0, 0);
}

void Perl_Mob_SendAppearanceEffectActor(Mob* self, int32 parm1, uint32 slot1, int32 parm2, uint32 slot2, int32 parm3, uint32 slot3) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, parm3, 0, 0, nullptr, slot1, 0, slot2, 0, slot3, 0, 0, 0, 0, 0);
}

void Perl_Mob_SendAppearanceEffectActor(Mob* self, int32 parm1, uint32 slot1, int32 parm2, uint32 slot2, int32 parm3, uint32 slot3, int32 parm4) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, parm3, parm4, 0, nullptr, slot1, 0, slot2, 0, slot3, 0, 0, 0, 0, 0);
}

void Perl_Mob_SendAppearanceEffectActor(Mob* self, int32 parm1, uint32 slot1, int32 parm2, uint32 slot2, int32 parm3, uint32 slot3, int32 parm4, uint32 slot4) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, parm3, parm4, 0, nullptr, slot1, 0, slot2, 0, slot3, 0, slot4, 0, 0, 0);
}

void Perl_Mob_SendAppearanceEffectActor(Mob* self, int32 parm1, uint32 slot1, int32 parm2, uint32 slot2, int32 parm3, uint32 slot3, int32 parm4, uint32 slot4, int32 parm5) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, parm3, parm4, parm5, nullptr, slot1, 0, slot2, 0, slot3, 0, slot4, 0, 0, 0);
}

void Perl_Mob_SendAppearanceEffectActor(Mob* self, int32 parm1, uint32 slot1, int32 parm2, uint32 slot2, int32 parm3, uint32 slot3, int32 parm4, uint32 slot4, int32 parm5, uint32 slot5) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, parm3, parm4, parm5, nullptr, slot1, 0, slot2, 0, slot3, 0, slot4, 0, slot5, 0);
}

void Perl_Mob_SendAppearanceEffectActor(Mob* self, int32 parm1, uint32 slot1, int32 parm2, uint32 slot2, int32 parm3, uint32 slot3, int32 parm4, uint32 slot4, int32 parm5, uint32 slot5, Client* client) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, parm3, parm4, parm5, client, slot1, 0, slot2, 0, slot3, 0, slot4, 0, slot5, 0);
}

void Perl_Mob_SendAppearanceEffectGround(Mob* self, int32 parm1) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, 0, 0, 0, 0);
}

void Perl_Mob_SendAppearanceEffectGround(Mob* self, int32 parm1, int32 parm2) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, 0, 0, 0);
}

void Perl_Mob_SendAppearanceEffectGround(Mob* self, int32 parm1, int32 parm2, int32 parm3) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, parm3, 0, 0);
}

void Perl_Mob_SendAppearanceEffectGround(Mob* self, int32 parm1, int32 parm2, int32 parm3, int32 parm4) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, parm3, parm4, 0);
}

void Perl_Mob_SendAppearanceEffectGround(Mob* self, int32 parm1, int32 parm2, int32 parm3, int32 parm4, int32 parm5) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, parm3, parm4, parm5);
}

void Perl_Mob_SendAppearanceEffectGround(Mob* self, int32 parm1, int32 parm2, int32 parm3, int32 parm4, int32 parm5, Client* client) // @categories Script Utility
{
	self->SendAppearanceEffect(parm1, parm2, parm3, parm4, parm5, client);
}

void Perl_Mob_RemoveAllAppearanceEffects(Mob* self) // @categories Script Utility
{
	self->SendIllusionPacket(
		AppearanceStruct{
			.beard = self->GetBeard(),
			.beard_color = self->GetBeardColor(),
			.drakkin_details = self->GetDrakkinDetails(),
			.drakkin_heritage = self->GetDrakkinHeritage(),
			.drakkin_tattoo = self->GetDrakkinTattoo(),
			.eye_color_one = self->GetEyeColor1(),
			.eye_color_two = self->GetEyeColor2(),
			.face = self->GetLuclinFace(),
			.gender_id = self->GetGender(),
			.hair = self->GetHairStyle(),
			.hair_color = self->GetHairColor(),
			.helmet_texture = self->GetHelmTexture(),
			.race_id = self->GetRace(),
			.send_effects = false,
			.size = self->GetSize(),
			.texture = self->GetTexture(),
		}
	);
	self->ClearAppearanceEffects();
}

void Perl_Mob_SetFlyMode(Mob* self, int flymode) // @categories Script Utility
{
	self->SetFlyMode(static_cast<GravityBehavior>(flymode));
}

void Perl_Mob_SetTexture(Mob* self, uint8 texture) // @categories Stats and Attributes
{
	self->SendIllusionPacket(
		AppearanceStruct{
			.race_id = self->GetRace(),
			.texture = texture,
		}
	);
}

void Perl_Mob_SetRace(Mob* self, uint16 race_id) // @categories Stats and Attributes
{
	self->SendIllusionPacket(
		AppearanceStruct{
			.race_id = race_id,
		}
	);
}

void Perl_Mob_SetGender(Mob* self, uint8 gender_id) // @categories Stats and Attributes
{
	self->SendIllusionPacket(
		AppearanceStruct{
			.gender_id = gender_id,
			.race_id = self->GetRace(),
		}
	);
}

// todo: SendIllusion should be sent in a hash like lua
void Perl_Mob_SendIllusion(Mob *self, uint16 race_id) // @categories Script Utility
{
	self->SendIllusionPacket(
		AppearanceStruct{
			.race_id = race_id,
		}
	);
}

void Perl_Mob_SendIllusion(Mob *self, uint16 race_id, uint8 gender_id) // @categories Script Utility
{
	self->SendIllusionPacket(
		AppearanceStruct{
			.gender_id = gender_id,
			.race_id = race_id,
		}
	);
}

void Perl_Mob_SendIllusion(Mob *self, uint16 race_id, uint8 gender_id, uint8 texture) // @categories Script Utility
{
	self->SendIllusionPacket(
		AppearanceStruct{
			.gender_id = gender_id,
			.race_id = race_id,
			.texture = texture,
		}
	);
}

void Perl_Mob_SendIllusion(Mob *self, uint16 race_id, uint8 gender_id, uint8 texture, uint8 helmet_texture) // @categories Script Utility
{
	self->SendIllusionPacket(
		AppearanceStruct{
			.gender_id = gender_id,
			.helmet_texture = helmet_texture,
			.race_id = race_id,
			.texture = texture,
		}
	);
}

void Perl_Mob_SendIllusion(Mob *self, uint16 race_id, uint8 gender_id, uint8 texture, uint8 helmet_texture, uint8 face) // @categories Script Utility
{
	self->SendIllusionPacket(
		AppearanceStruct{
			.face = face,
			.gender_id = gender_id,
			.helmet_texture = helmet_texture,
			.race_id = race_id,
			.texture = texture,
		}
	);
}

void Perl_Mob_SendIllusion(Mob *self, uint16 race_id, uint8 gender_id, uint8 texture, uint8 helmet_texture, uint8 face, uint8 hair) // @categories Script Utility
{
	self->SendIllusionPacket(
		AppearanceStruct{
			.face = face,
			.gender_id = gender_id,
			.hair = hair,
			.helmet_texture = helmet_texture,
			.race_id = race_id,
			.texture = texture,
		}
	);
}

void Perl_Mob_SendIllusion(Mob *self, uint16 race_id, uint8 gender_id, uint8 texture, uint8 helmet_texture, uint8 face, uint8 hair, uint8 hair_color) // @categories Script Utility
{
	self->SendIllusionPacket(
		AppearanceStruct{
			.face = face,
			.gender_id = gender_id,
			.hair = hair,
			.hair_color = hair_color,
			.helmet_texture = helmet_texture,
			.race_id = race_id,
			.texture = texture,
		}
	);
}

void Perl_Mob_SendIllusion(Mob *self, uint16 race_id, uint8 gender_id, uint8 texture, uint8 helmet_texture, uint8 face, uint8 hair, uint8 hair_color, uint8 beard) // @categories Script Utility
{
	self->SendIllusionPacket(
		AppearanceStruct{
			.beard = beard,
			.face = face,
			.gender_id = gender_id,
			.hair = hair,
			.hair_color = hair_color,
			.helmet_texture = helmet_texture,
			.race_id = race_id,
			.texture = texture,
		}
	);
}

void Perl_Mob_SendIllusion(Mob *self, uint16 race_id, uint8 gender_id, uint8 texture, uint8 helmet_texture, uint8 face, uint8 hair, uint8 hair_color, uint8 beard, uint8 beard_color) // @categories Script Utility
{
	self->SendIllusionPacket(
		AppearanceStruct{
			.beard = beard,
			.beard_color = beard_color,
			.face = face,
			.gender_id = gender_id,
			.hair = hair,
			.hair_color = hair_color,
			.helmet_texture = helmet_texture,
			.race_id = race_id,
			.texture = texture,
		}
	);
}

void Perl_Mob_SendIllusion(Mob *self, uint16 race_id, uint8 gender_id, uint8 texture, uint8 helmet_texture, uint8 face, uint8 hair, uint8 hair_color, uint8 beard, uint8 beard_color, uint32 drakkin_heritage) // @categories Script Utility
{
	self->SendIllusionPacket(
		AppearanceStruct{
			.beard = beard,
			.beard_color = beard_color,
			.drakkin_heritage = drakkin_heritage,
			.face = face,
			.gender_id = gender_id,
			.hair = hair,
			.hair_color = hair_color,
			.helmet_texture = helmet_texture,
			.race_id = race_id,
			.texture = texture,
		}
	);
}

void Perl_Mob_SendIllusion(Mob *self, uint16 race_id, uint8 gender_id, uint8 texture, uint8 helmet_texture, uint8 face, uint8 hair, uint8 hair_color, uint8 beard, uint8 beard_color, uint32 drakkin_heritage, uint32 drakkin_tattoo) // @categories Script Utility
{
	self->SendIllusionPacket(
		AppearanceStruct{
			.beard = beard,
			.beard_color = beard_color,
			.drakkin_heritage = drakkin_heritage,
			.drakkin_tattoo = drakkin_tattoo,
			.face = face,
			.gender_id = gender_id,
			.hair = hair,
			.hair_color = hair_color,
			.helmet_texture = helmet_texture,
			.race_id = race_id,
			.texture = texture,
		}
	);
}

void Perl_Mob_SendIllusion(Mob *self, uint16 race_id, uint8 gender_id, uint8 texture, uint8 helmet_texture, uint8 face, uint8 hair, uint8 hair_color, uint8 beard, uint8 beard_color, uint32 drakkin_heritage, uint32 drakkin_tattoo, uint32 drakkin_details) // @categories Script Utility
{
	self->SendIllusionPacket(
		AppearanceStruct{
			.beard = beard,
			.beard_color = beard_color,
			.drakkin_details = drakkin_details,
			.drakkin_heritage = drakkin_heritage,
			.drakkin_tattoo = drakkin_tattoo,
			.face = face,
			.gender_id = gender_id,
			.hair = hair,
			.hair_color = hair_color,
			.helmet_texture = helmet_texture,
			.race_id = race_id,
			.texture = texture,
		}
	);
}

void Perl_Mob_SendIllusion(Mob *self, uint16 race_id, uint8 gender_id, uint8 texture, uint8 helmet_texture, uint8 face, uint8 hair, uint8 hair_color, uint8 beard, uint8 beard_color, uint32 drakkin_heritage, uint32 drakkin_tattoo, uint32 drakkin_details, float size) // @categories Script Utility
{
	self->SendIllusionPacket(
		AppearanceStruct{
			.beard = beard,
			.beard_color = beard_color,
			.drakkin_details = drakkin_details,
			.drakkin_heritage = drakkin_heritage,
			.drakkin_tattoo = drakkin_tattoo,
			.face = face,
			.gender_id = gender_id,
			.hair = hair,
			.hair_color = hair_color,
			.helmet_texture = helmet_texture,
			.race_id = race_id,
			.size = size,
			.texture = texture,
		}
	);
}

void Perl_Mob_SendIllusion(Mob *self, uint16 race_id, uint8 gender_id, uint8 texture, uint8 helmet_texture, uint8 face, uint8 hair, uint8 hair_color, uint8 beard, uint8 beard_color, uint32 drakkin_heritage, uint32 drakkin_tattoo, uint32 drakkin_details, float size, Client *target) // @categories Script Utility
{
	self->SendIllusionPacket(
		AppearanceStruct{
			.beard = beard,
			.beard_color = beard_color,
			.drakkin_details = drakkin_details,
			.drakkin_heritage = drakkin_heritage,
			.drakkin_tattoo = drakkin_tattoo,
			.face = face,
			.gender_id = gender_id,
			.hair = hair,
			.hair_color = hair_color,
			.helmet_texture = helmet_texture,
			.race_id = race_id,
			.size = size,
			.target = target,
			.texture = texture,
		}
	);
}

void Perl_Mob_SendIllusionPacket(Mob* self, perl::reference table_ref)
{
	perl::hash table = table_ref;

	uint16 race_id                 = table.exists("race") ? table["race"] : self->GetRace();
	uint8  gender_id               = table.exists("gender") ? table["gender"] : self->GetGender();
	uint8  texture                 = table.exists("texture") ? table["texture"] : self->GetTexture();
	uint8  helmet_texture          = table.exists("helmtexture") ? table["helmtexture"] : self->GetHelmTexture();
	uint8  hair_color              = table.exists("haircolor") ? table["haircolor"] : self->GetHairColor();
	uint8  beard_color             = table.exists("beardcolor") ? table["beardcolor"] : self->GetBeardColor();
	uint8  eye_color_one           = table.exists("eyecolor1") ? table["eyecolor1"] : self->GetEyeColor1();
	uint8  eye_color_two           = table.exists("eyecolor2") ? table["eyecolor2"] : self->GetEyeColor2();
	uint8  hair                    = table.exists("hairstyle") ? table["hairstyle"] : self->GetHairStyle();
	uint8  face                    = table.exists("luclinface") ? table["luclinface"] : self->GetLuclinFace();
	uint8  beard                   = table.exists("beard") ? table["beard"] : self->GetBeard();
	uint8  aa_title                = table.exists("aa_title") ? table["aa_title"] : 255;
	uint32 drakkin_heritage        = table.exists("drakkin_heritage") ? table["drakkin_heritage"] : self->GetDrakkinHeritage();
	uint32 drakkin_tattoo          = table.exists("drakkin_tattoo") ? table["drakkin_tattoo"] : self->GetDrakkinTattoo();
	uint32 drakkin_details         = table.exists("drakkin_details") ? table["drakkin_details"] : self->GetDrakkinDetails();
	float  size                    = table.exists("size") ? table["size"] : self->GetSize();
	bool   send_appearance_effects = table.exists("send_appearance_effects") ? table["send_appearance_effects"] : true;

	Client *target = table.exists("target") ? static_cast<Client *>(table["target"]) : nullptr;

	self->SendIllusionPacket(
		AppearanceStruct{
			.beard = beard,
			.beard_color = beard_color,
			.drakkin_details = drakkin_details,
			.drakkin_heritage = drakkin_heritage,
			.drakkin_tattoo = drakkin_tattoo,
			.face = face,
			.gender_id = gender_id,
			.hair = hair,
			.hair_color = hair_color,
			.helmet_texture = helmet_texture,
			.race_id = race_id,
			.send_effects = send_appearance_effects,
			.size = size,
			.target = target,
			.texture = texture,
		}
	);
}

void Perl_Mob_CameraEffect(Mob* self, uint32 duration) // @categories Script Utility
{
	self->CameraEffect(duration, 0.03125f);
}

void Perl_Mob_CameraEffect(Mob* self, uint32 duration, float intensity) // @categories Script Utility
{
	self->CameraEffect(duration, intensity);
}

void Perl_Mob_CameraEffect(Mob* self, uint32 duration, float intensity, Client* client) // @categories Script Utility
{
	self->CameraEffect(duration, intensity, client);
}

void Perl_Mob_CameraEffect(Mob* self, uint32 duration, float intensity, perl::nullable<Client*> client, bool global) // @categories Script Utility
{
	self->CameraEffect(duration, intensity, client.get(), global);
}

void Perl_Mob_SpellEffect(Mob* self, uint32 effect) // @categories Spells and Disciplines
{
	self->SendSpellEffect(effect, 5000, 0, true, 3000);
}

void Perl_Mob_SpellEffect(Mob* self, uint32 effect, uint32 duration) // @categories Spells and Disciplines
{
	self->SendSpellEffect(effect, duration, 0, true, 3000);
}

void Perl_Mob_SpellEffect(Mob* self, uint32 effect, uint32 duration, uint32 finish_delay) // @categories Spells and Disciplines
{
	self->SendSpellEffect(effect, duration, finish_delay, true, 3000);
}

void Perl_Mob_SpellEffect(Mob* self, uint32 effect, uint32 duration, uint32 finish_delay, bool zone_wide) // @categories Spells and Disciplines
{
	self->SendSpellEffect(effect, duration, finish_delay, zone_wide, 3000);
}

void Perl_Mob_SpellEffect(Mob* self, uint32 effect, uint32 duration, uint32 finish_delay, bool zone_wide, uint32 unk20) // @categories Spells and Disciplines
{
	self->SendSpellEffect(effect, duration, finish_delay, zone_wide, unk20);
}

void Perl_Mob_SpellEffect(Mob* self, uint32 effect, uint32 duration, uint32 finish_delay, bool zone_wide, uint32 unk20, bool perm_effect) // @categories Spells and Disciplines
{
	self->SendSpellEffect(effect, duration, finish_delay, zone_wide, unk20, perm_effect);
}

void Perl_Mob_SpellEffect(Mob* self, uint32 effect, uint32 duration, uint32 finish_delay, bool zone_wide, uint32 unk20, bool perm_effect, Client* client) // @categories Spells and Disciplines
{
	self->SendSpellEffect(effect, duration, finish_delay, zone_wide, unk20, perm_effect, client);
}

void Perl_Mob_SpellEffect(Mob* self, uint32 effect, uint32 duration, uint32 finish_delay, bool zone_wide, uint32 unk20, bool perm_effect, perl::nullable<Client*> client, uint32 caster_id) // @categories Spells and Disciplines
{
	self->SendSpellEffect(effect, duration, finish_delay, zone_wide, unk20, perm_effect, client.get(), caster_id);
}

void Perl_Mob_SpellEffect(Mob* self, uint32 effect, uint32 duration, uint32 finish_delay, bool zone_wide, uint32 unk20, bool perm_effect, perl::nullable<Client*> client, uint32 caster_id, uint32 target_id) // @categories Spells and Disciplines
{
	self->SendSpellEffect(effect, duration, finish_delay, zone_wide, unk20, perm_effect, client.get(), caster_id, target_id);
}

void Perl_Mob_TempName(Mob* self) // @categories Script Utility
{
	self->TempName();
}

void Perl_Mob_TempName(Mob* self, const char* name) // @categories Script Utility
{
	self->TempName(name);
}

const int Perl_Mob_GetItemStat(Mob* self, uint32 item_id, std::string identifier) // @categories Inventory and Items, Stats and Attributes
{
	return self->GetItemStat(item_id, identifier);
}

std::string Perl_Mob_GetGlobal(Mob* self, const char* varname)
{
	return self->GetGlobal(varname);
}

void Perl_Mob_SetGlobal(Mob* self, const char* varname, const char* newvalue, int options, const char* duration)
{
	self->SetGlobal(varname, newvalue, options, duration);
}

void Perl_Mob_SetGlobal(Mob* self, const char* varname, const char* newvalue, int options, const char* duration, Mob* other)
{
	self->SetGlobal(varname, newvalue, options, duration, other);
}

void Perl_Mob_TarGlobal(Mob* self, const char* varname, const char* value, const char* duration, int npc_id, int char_id, int zone_id)
{
	self->TarGlobal(varname, value, duration, npc_id, char_id, zone_id);
}

void Perl_Mob_DelGlobal(Mob* self, const char* var_name)
{
	self->DelGlobal(var_name);
}

void Perl_Mob_SetSlotTint(Mob* self, uint8 material_slot, uint8 red_tint, uint8 green_tint, uint8 blue_tint) // @categories Stats and Attributes
{
	self->SetSlotTint(material_slot, red_tint, green_tint, blue_tint);
}

void Perl_Mob_WearChange(Mob* self, uint8 material_slot, uint32 texture) // @categories Script Utility
{
	self->WearChange(material_slot, texture);
}

void Perl_Mob_WearChange(Mob* self, uint8 material_slot, uint32 texture, uint32 color) // @categories Script Utility
{
	self->WearChange(material_slot, texture, color);
}

void Perl_Mob_WearChange(Mob* self, uint8 material_slot, uint32 texture, uint32 color, uint32 hero_forge_model) // @categories Script Utility
{
	self->WearChange(material_slot, texture, color, hero_forge_model);
}

void Perl_Mob_DoKnockback(Mob* self, Mob* caster, uint32 push_back, uint32 push_up) // @categories Script Utility
{
	self->DoKnockback(caster, push_back, push_up);
}

void Perl_Mob_RemoveNimbusEffect(Mob* self, int effect_id) // @categories Script Utility
{
	self->RemoveNimbusEffect(effect_id);
}

void Perl_Mob_SetRunning(Mob* self, bool value) // @categories Script Utility
{
	self->SetRunning(value);
}

bool Perl_Mob_IsRunning(Mob* self) // @categories Script Utility
{
	return self->IsRunning();
}

void Perl_Mob_SetBodyType(Mob* self, uint8 body_type_id) // @categories Stats and Attributes
{
	self->SetBodyType(body_type_id, false);
}

void Perl_Mob_SetBodyType(Mob* self, uint8 body_type_id, bool overwrite_orig) // @categories Stats and Attributes
{
	self->SetBodyType(body_type_id, overwrite_orig);
}

void Perl_Mob_SetDeltas(Mob* self, float delta_x, float delta_y, float delta_z, float delta_h) // @categories Script Utility
{
	auto delta = glm::vec4(delta_x, delta_y, delta_z, delta_h);
	self->SetDelta(delta);
}

void Perl_Mob_SetLD(Mob* self, bool value) // @categories Script Utility
{
	self->SendAppearancePacket(AppearanceType::Linkdead, value);
}

void Perl_Mob_SetTargetable(Mob* self, bool on) // @categories Stats and Attributes
{
	self->SetTargetable(on);
}

void Perl_Mob_ModSkillDmgTaken(Mob* self, int skill, int16 value) // @categories Skills and Recipes, Script Utility
{
	self->ModSkillDmgTaken(static_cast<EQ::skills::SkillType>(skill), value);
}

int Perl_Mob_GetModSkillDmgTaken(Mob* self, int skill_id) // @categories Stats and Attributes
{
	return self->GetModSkillDmgTaken(static_cast<EQ::skills::SkillType>(skill_id));
}

int Perl_Mob_GetSkillDmgTaken(Mob* self, int skill_id) // @categories Skills and Recipes, Script Utility
{
	return self->GetSkillDmgTaken(static_cast<EQ::skills::SkillType>(skill_id));
}

void Perl_Mob_SetAllowBeneficial(Mob* self, bool value) // @categories Stats and Attributes
{
	self->SetAllowBeneficial(value);
}

bool Perl_Mob_GetAllowBeneficial(Mob* self) // @categories Stats and Attributes
{
	return self->GetAllowBeneficial();
}

bool Perl_Mob_IsBeneficialAllowed(Mob* self, Mob* target) // @categories Stats and Attributes
{
	return self->IsBeneficialAllowed(target);
}

void Perl_Mob_ModVulnerability(Mob* self, uint8 resist, int16 value) // @categories Stats and Attributes
{
	self->ModVulnerability(resist, value);
}

int Perl_Mob_GetModVulnerability(Mob* self, uint8 resist) // @categories Stats and Attributes
{
	return self->GetModVulnerability(resist);
}

void Perl_Mob_DoMeleeSkillAttackDmg(Mob* self, Mob* target, uint16 weapon_damage, int skill, int16 chance_mod, int16 focus, uint8 can_riposte) // @categories Script Utility, Skills and Attributes
{
	self->DoMeleeSkillAttackDmg(target, weapon_damage, static_cast<EQ::skills::SkillType>(skill), chance_mod, focus, can_riposte);
}

void Perl_Mob_DoArcheryAttackDmg(Mob* self, Mob* target, uint16 weapon_damage, int16 chance_mod, int16 focus) // @categories Script Utility, Skills and Attributes
{
	self->DoArcheryAttackDmg(target, nullptr, nullptr, weapon_damage, chance_mod, focus);
}

void Perl_Mob_DoThrowingAttackDmg(Mob* self, Mob* target, uint16 weapon_damage, int16 chance_mod, int16 focus) // @categories Script Utility, Skills and Attributes
{
	self->DoThrowingAttackDmg(target, nullptr, nullptr, weapon_damage, chance_mod, focus);
}

void Perl_Mob_SetDisableMelee(Mob* self, bool value) // @categories Script Utility, Stats and Attributes
{
	self->SetDisableMelee(value);
}

bool Perl_Mob_IsMeleeDisabled(Mob* self) // @categories Stats and Attributes
{
	return self->IsMeleeDisabled();
}

void Perl_Mob_SetFlurryChance(Mob* self, uint8 value) // @categories Stats and Attributes
{
	self->SetFlurryChance(value);
}

int Perl_Mob_GetFlurryChance(Mob* self) // @categories Stats and Attributes
{
	return self->GetFlurryChance();
}

int Perl_Mob_GetSpellStat(Mob* self, uint32 spell_id, const char* stat) // @categories Spells and Disciplines
{
	return self->GetSpellStat(spell_id, stat);
}

int Perl_Mob_GetSpellStat(Mob* self, uint32 spell_id, const char* stat, uint8 slot) // @categories Spells and Disciplines
{
	return self->GetSpellStat(spell_id, stat, slot);
}

int Perl_Mob_GetBuffStatValueBySpell(Mob* self, int spell_id, const char* stat) // @categories Spells and Disciplines
{
	return self->GetBuffStatValueBySpell(spell_id, stat);
}

int Perl_Mob_GetBuffStatValueBySlot(Mob* self, uint8 slot, const char* stat)// @categories Script Utility, Spells and Disciplines
{
	return self->GetBuffStatValueBySlot(slot, stat);
}

int Perl_Mob_GetSpecialAbility(Mob* self, int ability) // @categories Stats and Attributes
{
	return self->GetSpecialAbility(ability);
}

int Perl_Mob_GetSpecialAbilityParam(Mob* self, int ability, int param) // @categories Stats and Attributes
{
	return self->GetSpecialAbilityParam(ability, param);
}

void Perl_Mob_SetSpecialAbility(Mob* self, int ability, int value) // @categories Stats and Attributes
{
	self->SetSpecialAbility(ability, value);
}

void Perl_Mob_SetSpecialAbilityParam(Mob* self, int ability, int param, int value) // @categories Stats and Attributes
{
	self->SetSpecialAbilityParam(ability, param, value);
}

void Perl_Mob_ClearSpecialAbilities(Mob* self)// @categories Script Utility
{
	self->ClearSpecialAbilities();
}

void Perl_Mob_ProcessSpecialAbilities(Mob* self, std::string str) // @categories Script Utility
{
	self->ProcessSpecialAbilities(str);
}

bool Perl_Mob_CanClassEquipItem(Mob* self, uint32 item_id) // @categories Inventory and Items, Script Utility
{
	return self->CanClassEquipItem(item_id);
}

bool Perl_Mob_IsFeared(Mob* self) // @categories Script Utility
{
	return self->IsFeared();
}

bool Perl_Mob_IsBlind(Mob* self) // @categories Script Utility
{
	return self->IsBlind();
}

int Perl_Mob_GetInvisibleLevel(Mob* self) // @categories Stats and Attributes
{
	return self->GetInvisibleLevel();
}

int Perl_Mob_GetInvisibleUndeadLevel(Mob* self) // @categories Stats and Attributes
{
	return self->GetInvisibleUndeadLevel();
}

int Perl_Mob_SeeInvisible(Mob* self) // @categories Stats and Attributes
{
	return self->SeeInvisible();
}

int Perl_Mob_SeeInvisibleUndead(Mob* self) // @categories Stats and Attributes
{
	return self->SeeInvisibleUndead();
}

bool Perl_Mob_SeeHide(Mob* self) // @categories Stats and Attributes
{
	return self->SeeHide();
}

bool Perl_Mob_SeeImprovedHide(Mob* self) // @categories Stats and Attributes
{
	return self->SeeImprovedHide();
}

int Perl_Mob_GetNimbusEffect1(Mob* self) // @categories Script Utility
{
	return self->GetNimbusEffect1();
}

int Perl_Mob_GetNimbusEffect2(Mob* self) // @categories Script Utility
{
	return self->GetNimbusEffect2();
}

int Perl_Mob_GetNimbusEffect3(Mob* self) // @categories Script Utility
{
	return self->GetNimbusEffect3();
}

bool Perl_Mob_IsTargetable(Mob* self) // @categories Stats and Attributes
{
	return self->IsTargetable();
}

bool Perl_Mob_HasShieldEquipped(Mob* self) // @categories Stats and Attributes
{
	return self->HasShieldEquipped();
}

bool Perl_Mob_HasTwoHandBluntEquipped(Mob* self) // @categories Stats and Attributes
{
	return self->HasTwoHandBluntEquipped();
}

bool Perl_Mob_HasTwoHanderEquipped(Mob* self) // @categories Stats and Attributes
{
	return self->HasTwoHanderEquipped();
}

int32_t Perl_Mob_GetHerosForgeModel(Mob* self, uint8_t material_slot) // @categories Stats and Attributes
{
	return self->GetHerosForgeModel(material_slot);
}

bool Perl_Mob_IsEliteMaterialItem(Mob* self, uint8_t material_slot) // @categories Script Utility, Stats and Attributes
{
	return self->IsEliteMaterialItem(material_slot);
}

float Perl_Mob_GetBaseSize(Mob* self) // @categories Stats and Attributes
{
	return self->GetBaseSize();
}

bool Perl_Mob_HasOwner(Mob* self) // @categories Pet
{
	return self->HasOwner();
}

bool Perl_Mob_IsPet(Mob* self) // @categories Pet
{
	return self->IsPet();
}

bool Perl_Mob_HasPet(Mob* self)  // @categories Pet
{
	return self->HasPet();
}

void Perl_Mob_RemovePet(Mob* self) // @categories Pet
{
	self->SetPet(nullptr);
}

void Perl_Mob_SetPet(Mob* self, Mob* new_pet) // @categories Pet
{
	self->SetPet(new_pet);
}

bool Perl_Mob_IsSilenced(Mob* self) // @categories Script Utility
{
	return self->IsSilenced();
}

bool Perl_Mob_IsAmnesiad(Mob* self) // @categories Script Utility
{
	return self->IsAmnesiad();
}

int32_t Perl_Mob_GetMeleeMitigation(Mob* self) // @categories Stats and Attributes
{
	return self->GetMeleeMitigation();
}

void Perl_Mob_TryMoveAlong(Mob* self, float distance, float angle) // @categories Script Utility
{
	self->TryMoveAlong(distance, angle);
}

void Perl_Mob_TryMoveAlong(Mob* self, float distance, float angle, bool send) // @categories Script Utility
{
	self->TryMoveAlong(distance, angle, send);
}

std::string Perl_Mob_GetClassName(Mob* self)
{
	return GetClassIDName(self->GetClass());
}

std::string Perl_Mob_GetRaceName(Mob* self)
{
	return GetRaceIDName(self->GetRace());
}

void Perl_Mob_DeleteBucket(Mob* self, std::string bucket_name) // @categories Script Utility
{
	self->DeleteBucket(bucket_name);
}

std::string Perl_Mob_GetBucket(Mob* self, std::string bucket_name) // @categories Script Utility
{
	return self->GetBucket(bucket_name);
}

std::string Perl_Mob_GetBucketExpires(Mob* self, std::string bucket_name) // @categories Script Utility
{
	return self->GetBucketExpires(bucket_name);
}

std::string Perl_Mob_GetBucketKey(Mob* self) // @categories Script Utility
{
	return {};
}

std::string Perl_Mob_GetBucketRemaining(Mob* self, std::string bucket_name) // @categories Script Utility
{
	return self->GetBucketRemaining(bucket_name);
}

void Perl_Mob_SetBucket(Mob* self, std::string bucket_name, std::string bucket_value) // @categories Script Utility
{
	self->SetBucket(bucket_name, bucket_value);
}

void Perl_Mob_SetBucket(Mob* self, std::string bucket_name, std::string bucket_value, std::string expiration) // @categories Script Utility
{
	self->SetBucket(bucket_name, bucket_value, expiration);
}

bool Perl_Mob_IsHorse(Mob* self) // @categories Script Utility
{
	return self->IsHorse();
}

perl::array Perl_Mob_GetHateListByDistance(Mob* self) // @categories Hate and Aggro
{
	perl::array result;

	auto h_list = self->GetFilteredHateList();
	for (auto h : h_list) {
		result.push_back(h);
	}

	return result;
}

perl::array Perl_Mob_GetHateListByDistance(Mob* self, uint32 distance) // @categories Hate and Aggro
{
	perl::array result;

	auto h_list = self->GetFilteredHateList(EntityFilterType::All, distance);
	for (auto h : h_list) {
		result.push_back(h);
	}

	return result;
}

Mob* Perl_Mob_GetHateClosest(Mob* self) // @categories Hate and Aggro
{
	return self->GetHateClosest();
}

Mob* Perl_Mob_GetHateClosest(Mob* self, bool skip_mezzed) // @categories Hate and Aggro
{
	return self->GetHateClosest(skip_mezzed);
}

Bot* Perl_Mob_GetHateClosestBot(Mob* self) // @categories Hate and Aggro
{
	return self->GetHateClosestBot();
}

Bot* Perl_Mob_GetHateClosestBot(Mob* self, bool skip_mezzed) // @categories Hate and Aggro
{
	return self->GetHateClosestBot(skip_mezzed);
}

Client* Perl_Mob_GetHateClosestClient(Mob* self) // @categories Hate and Aggro
{
	return self->GetHateClosestClient();
}

Client* Perl_Mob_GetHateClosestClient(Mob* self, bool skip_mezzed) // @categories Hate and Aggro
{
	return self->GetHateClosestClient(skip_mezzed);
}

NPC* Perl_Mob_GetHateClosestNPC(Mob* self) // @categories Hate and Aggro
{
	return self->GetHateClosestNPC();
}

NPC* Perl_Mob_GetHateClosestNPC(Mob* self, bool skip_mezzed) // @categories Hate and Aggro
{
	return self->GetHateClosestNPC(skip_mezzed);
}

std::string Perl_Mob_GetLastName(Mob* self) // @categories Script Utility
{
	return self->GetLastName();
}

bool Perl_Mob_CanRaceEquipItem(Mob* self, uint32 item_id) // @categories Inventory and Items, Script Utility
{
	return self->CanRaceEquipItem(item_id);
}

void Perl_Mob_RemoveAllNimbusEffects(Mob* self) // @categories Script Utility
{
	self->RemoveAllNimbusEffects();
}

void Perl_Mob_AddNimbusEffect(Mob* self, int effect_id) // @categories Script Utility
{
	self->AddNimbusEffect(effect_id);
}

void Perl_Mob_ShieldAbility(Mob* self, uint32 target_id) // @categories Spells and Disciplines
{
	self->ShieldAbility(target_id);
}

void Perl_Mob_ShieldAbility(Mob* self, uint32 target_id, int32 shielder_max_distance) // @categories Spells and Disciplines
{
	self->ShieldAbility(target_id, shielder_max_distance);
}

void Perl_Mob_ShieldAbility(Mob* self, uint32 target_id, int32 shielder_max_distance, int32 shield_duration) // @categories Spells and Disciplines
{
	self->ShieldAbility(target_id, shielder_max_distance, shield_duration);
}

void Perl_Mob_ShieldAbility(Mob* self, uint32 target_id, int32 shielder_max_distance, int32 shield_duration, int32 shield_target_mitigation) // @categories Spells and Disciplines
{
	self->ShieldAbility(target_id, shielder_max_distance, shield_duration, shield_target_mitigation);
}

void Perl_Mob_ShieldAbility(Mob* self, uint32 target_id, int32 shielder_max_distance, int32 shield_duration, int32 shield_target_mitigation, int32 shielder_mitigation) // @categories Spells and Disciplines
{
	self->ShieldAbility(target_id, shielder_max_distance, shield_duration, shield_target_mitigation, shielder_mitigation);
}

void Perl_Mob_ShieldAbility(Mob* self, uint32 target_id, int32 shielder_max_distance, int32 shield_duration, int32 shield_target_mitigation, int32 shielder_mitigation, bool use_aa) // @categories Spells and Disciplines
{
	self->ShieldAbility(target_id, shielder_max_distance, shield_duration, shield_target_mitigation, shielder_mitigation, use_aa);
}

void Perl_Mob_ShieldAbility(Mob* self, uint32 target_id, int32 shielder_max_distance, int32 shield_duration, int32 shield_target_mitigation, int32 shielder_mitigation, bool use_aa, bool can_shield_npc) // @categories Spells and Disciplines
{
	self->ShieldAbility(target_id, shielder_max_distance, shield_duration, shield_target_mitigation, shielder_mitigation, use_aa, can_shield_npc);
}

Client* Perl_Mob_GetHateRandomClient(Mob* self) // @categories Hate and Aggro
{
	return self->GetHateRandomClient();
}

NPC* Perl_Mob_GetHateRandomNPC(Mob* self) // @categories Hate and Aggro
{
	return self->GetHateRandomNPC();
}

void Perl_Mob_SetBuffDuration(Mob* self, int spell_id) // @categories Script Utility, Spells and Disciplines
{
	self->SetBuffDuration(spell_id);
}

void Perl_Mob_SetBuffDuration(Mob* self, int spell_id, int duration) // @categories Script Utility, Spells and Disciplines
{
	self->SetBuffDuration(spell_id, duration);
}

void Perl_Mob_SetBuffDuration(Mob* self, int spell_id, int duration, int level) // @categories Script Utility, Spells and Disciplines
{
	self->SetBuffDuration(spell_id, duration, level);
}

void Perl_Mob_ApplySpellBuff(Mob* self, int spell_id) // @categories Script Utility, Spells and Disciplines
{
	self->ApplySpellBuff(spell_id);
}

void Perl_Mob_ApplySpellBuff(Mob* self, int spell_id, int duration) // @categories Script Utility, Spells and Disciplines
{
	self->ApplySpellBuff(spell_id, duration);
}

void Perl_Mob_ApplySpellBuff(Mob* self, int spell_id, int duration, int level) // @categories Script Utility, Spells and Disciplines
{
	self->ApplySpellBuff(spell_id, duration, level);
}

int Perl_Mob_GetSkillDmgAmt(Mob* self, int skill_id)
{
	return self->GetSkillDmgAmt(skill_id);
}

Mob* Perl_Mob_GetUltimateOwner(Mob* self) // @categories Script Utility, Pet
{
	return self->GetUltimateOwner();
}

bool Perl_Mob_RandomizeFeatures(Mob* self) // @categories Script Utility
{
	return self->RandomizeFeatures();
}

bool Perl_Mob_RandomizeFeatures(Mob* self, bool send_illusion) // @categories Script Utility
{
	return self->RandomizeFeatures(send_illusion);
}

bool Perl_Mob_RandomizeFeatures(Mob* self, bool send_illusion, bool save_variables) // @categories Script Utility
{
	return self->RandomizeFeatures(send_illusion, save_variables);
}

void Perl_Mob_CloneAppearance(Mob* self, Mob* other) // @categories Script Utility
{
	self->CloneAppearance(other);
}

void Perl_Mob_CloneAppearance(Mob* self, Mob* other, bool clone_name) // @categories Script Utility
{
	self->CloneAppearance(other, clone_name);
}

Mob* Perl_Mob_GetOwner(Mob* self) // @categories Script Utility, Pet
{
	return self->GetOwner();
}

void Perl_Mob_DamageArea(Mob* self, int64 damage) // @categories Hate and Aggro
{
	self->DamageArea(damage);
}

void Perl_Mob_DamageArea(Mob* self, int64 damage, uint32 distance) // @categories Hate and Aggro
{
	self->DamageArea(damage, distance, EntityFilterType::All);
}

void Perl_Mob_DamageAreaPercentage(Mob* self, int64 damage) // @categories Hate and Aggro
{
	self->DamageArea(damage, 0, EntityFilterType::All, true);
}

void Perl_Mob_DamageAreaPercentage(Mob* self, int64 damage, uint32 distance) // @categories Hate and Aggro
{
	self->DamageArea(damage, distance, EntityFilterType::All, true);
}

void Perl_Mob_DamageAreaClients(Mob* self, int64 damage) // @categories Hate and Aggro
{
	self->DamageArea(damage, 0, EntityFilterType::Clients);
}

void Perl_Mob_DamageAreaClients(Mob* self, int64 damage, uint32 distance) // @categories Hate and Aggro
{
	self->DamageArea(damage, distance, EntityFilterType::Clients);
}

void Perl_Mob_DamageAreaClientsPercentage(Mob* self, int64 damage) // @categories Hate and Aggro
{
	self->DamageArea(damage, 0, EntityFilterType::Clients, true);
}

void Perl_Mob_DamageAreaClientsPercentage(Mob* self, int64 damage, uint32 distance) // @categories Hate and Aggro
{
	self->DamageArea(damage, distance, EntityFilterType::Clients, true);
}

void Perl_Mob_DamageAreaNPCs(Mob* self, int64 damage) // @categories Hate and Aggro
{
	self->DamageArea(damage, 0, EntityFilterType::NPCs);
}

void Perl_Mob_DamageAreaNPCs(Mob* self, int64 damage, uint32 distance) // @categories Hate and Aggro
{
	self->DamageArea(damage, distance, EntityFilterType::NPCs);
}

void Perl_Mob_DamageAreaNPCsPercentage(Mob* self, int64 damage) // @categories Hate and Aggro
{
	self->DamageArea(damage, 0, EntityFilterType::NPCs, true);
}

void Perl_Mob_DamageAreaNPCsPercentage(Mob* self, int64 damage, uint32 distance) // @categories Hate and Aggro
{
	self->DamageArea(damage, distance, EntityFilterType::NPCs, true);
}

void Perl_Mob_DamageHateList(Mob* self, int64 damage) // @categories Hate and Aggro
{
	self->DamageHateList(damage);
}

void Perl_Mob_DamageHateList(Mob* self, int64 damage, uint32 distance) // @categories Hate and Aggro
{
	self->DamageHateList(damage, distance, EntityFilterType::All);
}

void Perl_Mob_DamageHateListPercentage(Mob* self, int64 damage) // @categories Hate and Aggro
{
	self->DamageHateList(damage, 0, EntityFilterType::All, true);
}

void Perl_Mob_DamageHateListPercentage(Mob* self, int64 damage, uint32 distance) // @categories Hate and Aggro
{
	self->DamageHateList(damage, distance, EntityFilterType::All, true);
}

void Perl_Mob_DamageHateListClients(Mob* self, int64 damage) // @categories Hate and Aggro
{
	self->DamageHateList(damage, 0, EntityFilterType::Clients);
}

void Perl_Mob_DamageHateListClients(Mob* self, int64 damage, uint32 distance) // @categories Hate and Aggro
{
	self->DamageHateList(damage, distance, EntityFilterType::Clients);
}

void Perl_Mob_DamageHateListClientsPercentage(Mob* self, int64 damage) // @categories Hate and Aggro
{
	self->DamageHateList(damage, 0, EntityFilterType::Clients, true);
}

void Perl_Mob_DamageHateListClientsPercentage(Mob* self, int64 damage, uint32 distance) // @categories Hate and Aggro
{
	self->DamageHateList(damage, distance, EntityFilterType::Clients, true);
}

void Perl_Mob_DamageHateListNPCs(Mob* self, int64 damage) // @categories Hate and Aggro
{
	self->DamageHateList(damage, 0, EntityFilterType::NPCs);
}

void Perl_Mob_DamageHateListNPCs(Mob* self, int64 damage, uint32 distance) // @categories Hate and Aggro
{
	self->DamageHateList(damage, distance, EntityFilterType::NPCs);
}

void Perl_Mob_DamageHateListNPCsPercentage(Mob* self, int64 damage) // @categories Hate and Aggro
{
	self->DamageHateList(damage, 0, EntityFilterType::NPCs, true);
}

void Perl_Mob_DamageHateListNPCsPercentage(Mob* self, int64 damage, uint32 distance) // @categories Hate and Aggro
{
	self->DamageHateList(damage, distance, EntityFilterType::NPCs, true);
}

perl::array Perl_Mob_GetHateListClients(Mob* self)
{
	perl::array result;

	auto h_list = self->GetFilteredHateList(EntityFilterType::Clients);
	for (auto h : h_list) {
		result.push_back(h);
	}

	return result;
}

perl::array Perl_Mob_GetHateListClients(Mob* self, uint32 distance)
{
	perl::array result;

	auto h_list = self->GetFilteredHateList(EntityFilterType::Clients, distance);
	for (auto h : h_list) {
		result.push_back(h);
	}

	return result;
}

perl::array Perl_Mob_GetHateListNPCs(Mob* self)
{
	perl::array result;

	auto h_list = self->GetFilteredHateList(EntityFilterType::NPCs);
	for (auto h : h_list) {
		result.push_back(h);
	}

	return result;
}

perl::array Perl_Mob_GetHateListNPCs(Mob* self, uint32 distance)
{
	perl::array result;

	auto h_list = self->GetFilteredHateList(EntityFilterType::NPCs, distance);
	for (auto h : h_list) {
		result.push_back(h);
	}

	return result;
}

bool Perl_Mob_ClearEntityVariables(Mob* self) // @categories Script Utility
{
	return self->ClearEntityVariables();
}

bool Perl_Mob_DeleteEntityVariable(Mob* self, std::string variable_name) // @categories Script Utility
{
	return self->DeleteEntityVariable(variable_name);
}

bool Perl_Mob_EntityVariableExists(Mob* self, std::string variable_name) // @categories Script Utility
{
	return self->EntityVariableExists(variable_name);
}

std::string Perl_Mob_GetEntityVariable(Mob* self, std::string variable_name) // @categories Script Utility
{
	return self->GetEntityVariable(variable_name);
}

perl::array Perl_Mob_GetEntityVariables(Mob* self) // @categories Script Utility
{
	perl::array a;

	const auto& l = self->GetEntityVariables();
	for (const auto& v : l) {
		a.push_back(v);
	}

	return a;
}

void Perl_Mob_SetEntityVariable(Mob* self, std::string variable_name, std::string variable_value) // @categories Script Utility
{
	self->SetEntityVariable(variable_name, variable_value);
}

void Perl_Mob_CopyHateList(Mob* self, Mob* to)
{
	self->CopyHateList(to);
}

bool Perl_Mob_IsAttackAllowed(Mob* self, Mob* target)
{
	return self->IsAttackAllowed(target);
}

bool Perl_Mob_IsAttackAllowed(Mob* self, Mob* target, bool is_spell_attack)
{
	return self->IsAttackAllowed(target, is_spell_attack);
}

uint32 Perl_Mob_BuffCount(Mob* self) // @categories Script Utility, Spells and Disciplines
{
	return self->BuffCount();
}

uint32 Perl_Mob_BuffCount(Mob* self, bool is_beneficial) // @categories Script Utility, Spells and Disciplines
{
	return self->BuffCount(is_beneficial);
}

uint32 Perl_Mob_BuffCount(Mob* self, bool is_beneficial, bool is_detrimental) // @categories Script Utility, Spells and Disciplines
{
	return self->BuffCount(is_beneficial, is_detrimental);
}

void Perl_Mob_DamageAreaBots(Mob* self, int64 damage) // @categories Hate and Aggro
{
	self->DamageArea(damage, 0, EntityFilterType::Bots);
}

void Perl_Mob_DamageAreaBots(Mob* self, int64 damage, uint32 distance) // @categories Hate and Aggro
{
	self->DamageArea(damage, distance, EntityFilterType::Bots);
}

void Perl_Mob_DamageAreaBotsPercentage(Mob* self, int64 damage) // @categories Hate and Aggro
{
	self->DamageArea(damage, 0, EntityFilterType::Bots, true);
}

void Perl_Mob_DamageAreaBotsPercentage(Mob* self, int64 damage, uint32 distance) // @categories Hate and Aggro
{
	self->DamageArea(damage, distance, EntityFilterType::Bots, true);
}

void Perl_Mob_DamageHateListBots(Mob* self, int64 damage) // @categories Hate and Aggro
{
	self->DamageHateList(damage, 0, EntityFilterType::Bots);
}

void Perl_Mob_DamageHateListBots(Mob* self, int64 damage, uint32 distance) // @categories Hate and Aggro
{
	self->DamageHateList(damage, distance, EntityFilterType::Bots);
}

void Perl_Mob_DamageHateListBotsPercentage(Mob* self, int64 damage) // @categories Hate and Aggro
{
	self->DamageHateList(damage, 0, EntityFilterType::Bots, true);
}

void Perl_Mob_DamageHateListBotsPercentage(Mob* self, int64 damage, uint32 distance) // @categories Hate and Aggro
{
	self->DamageHateList(damage, distance, EntityFilterType::Bots, true);
}

perl::array Perl_Mob_GetHateListBots(Mob* self)
{
	perl::array result;

	auto h_list = self->GetFilteredHateList(EntityFilterType::Bots);
	for (auto h : h_list) {
		result.push_back(h);
	}

	return result;
}

perl::array Perl_Mob_GetHateListBots(Mob* self, uint32 distance)
{
	perl::array result;

	auto h_list = self->GetFilteredHateList(EntityFilterType::Bots, distance);
	for (auto h : h_list)
	{
		result.push_back(h);
	}

	return result;
}

Bot* Perl_Mob_CastToBot(Mob* self)
{
	return self->CastToBot();
}

Bot* Perl_Mob_GetHateRandomBot(Mob* self) // @categories Hate and Aggro
{
	return self->GetHateRandomBot();
}

bool Perl_Mob_IsFindable(Mob* self) // @categories Script Utility
{
	return self->IsFindable();
}

bool Perl_Mob_IsTrackable(Mob* self) // @categories Script Utility
{
	return self->IsTrackable();
}

bool Perl_Mob_IsBerserk(Mob* self) // @categories Script Utility
{
	return self->IsBerserk();
}

float Perl_Mob_GetDefaultRaceSize(Mob* self) // @categories Script Utility
{
	return self->GetDefaultRaceSize();
}

float Perl_Mob_GetDefaultRaceSize(Mob* self, int race_id) // @categories Script Utility
{
	return self->GetDefaultRaceSize(race_id);
}

float Perl_Mob_GetDefaultRaceSize(Mob* self, int race_id, int gender_id) // @categories Script Utility
{
	return self->GetDefaultRaceSize(race_id, gender_id);
}

uint32 Perl_Mob_GetRemainingTimeMS(Mob* self, const char* timer_name)
{
	return quest_manager.getremainingtimeMS(timer_name, self);
}

uint32 Perl_Mob_GetTimerDurationMS(Mob* self, const char* timer_name)
{
	return quest_manager.gettimerdurationMS(timer_name, self);
}

bool Perl_Mob_HasTimer(Mob* self, const char* timer_name)
{
	return quest_manager.hastimer(timer_name, self);
}

bool Perl_Mob_IsPausedTimer(Mob* self, const char* timer_name)
{
	return quest_manager.ispausedtimer(timer_name, self);
}

void Perl_Mob_PauseTimer(Mob* self, const char* timer_name)
{
	quest_manager.pausetimer(timer_name, self);
}

void Perl_Mob_ResumeTimer(Mob* self, const char* timer_name)
{
	quest_manager.resumetimer(timer_name, self);
}

void Perl_Mob_SetTimer(Mob* self, const char* timer_name, int seconds)
{
	quest_manager.settimer(timer_name, seconds, self);
}

void Perl_Mob_SetTimerMS(Mob* self, const char* timer_name, int milliseconds)
{
	quest_manager.settimerMS(timer_name, milliseconds, self);
}

void Perl_Mob_StopAllTimers(Mob* self)
{
	quest_manager.stopalltimers(self);
}

void Perl_Mob_StopTimer(Mob* self, const char* timer_name)
{
	quest_manager.stoptimer(timer_name, self);
}

perl::array Perl_Mob_GetBuffSpellIDs(Mob* self)
{
	perl::array l;

	const auto& b = self->GetBuffSpellIDs();

	for (const auto& e : b) {
		l.push_back(e);
	}

	return l;
}

bool Perl_Mob_HasSpellEffect(Mob* self, int effect_id)
{
	return self->HasSpellEffect(effect_id);
}

perl::array Perl_Mob_GetCloseMobList(Mob* self)
{
	perl::array result;

	const auto& l = entity_list.GetCloseMobList(self);

	result.reserve(l.size());

	for (const auto& e : l) {
		result.push_back(e.second);
	}

	return result;
}

perl::array Perl_Mob_GetCloseMobList(Mob* self, float distance)
{
	perl::array result;

	const auto& l = entity_list.GetCloseMobList(self, distance);

	result.reserve(l.size());

	for (const auto& e : l) {
		if (self->CalculateDistance(e.second) <= distance) {
			result.push_back(e.second);
		}
	}

	return result;
}

perl::array Perl_Mob_GetCloseMobList(Mob* self, float distance, bool ignore_self)
{
	perl::array result;

	const auto& l = entity_list.GetCloseMobList(self, distance);

	result.reserve(l.size());

	for (const auto& e : l) {
		if (ignore_self && e.second == self) {
			continue;
		}

		if (self->CalculateDistance(e.second) <= distance) {
			result.push_back(e.second);
		}
	}

	return result;
}

StatBonuses* Perl_Mob_GetAABonuses(Mob* self)
{
	return self->GetAABonusesPtr();
}

StatBonuses* Perl_Mob_GetItemBonuses(Mob* self)
{
	return self->GetItemBonusesPtr();
}

StatBonuses* Perl_Mob_GetSpellBonuses(Mob* self)
{
	return self->GetSpellBonusesPtr();
}

std::string Perl_Mob_GetClassPlural(Mob* self)
{
	return self->GetClassPlural();
}

uint32 Perl_Mob_GetMobTypeIdentifier(Mob* self)
{
	return self->GetMobTypeIdentifier();
}

std::string Perl_Mob_GetRacePlural(Mob* self)
{
	return self->GetRacePlural();
}

uint32 Perl_Mob_GetHateListCount(Mob* self)
{
	return self->GetHateListCount();
}

uint32 Perl_Mob_GetHateListBotCount(Mob* self)
{
	return self->GetHateListCount(HateListCountType::Bot);
}

uint32 Perl_Mob_GetHateListClientCount(Mob* self)
{
	return self->GetHateListCount(HateListCountType::Client);
}

uint32 Perl_Mob_GetHateListNPCCount(Mob* self)
{
	return self->GetHateListCount(HateListCountType::NPC);
}

bool Perl_Mob_IsAnimation(Mob* self)
{
	return self->IsAnimation();
}

bool Perl_Mob_IsCharmed(Mob* self)
{
	return self->IsCharmed();
}

bool Perl_Mob_IsFamiliar(Mob* self)
{
	return self->IsFamiliar();
}

bool Perl_Mob_IsTargetLockPet(Mob* self)
{
	return self->IsTargetLockPet();
}

bool Perl_Mob_IsPetOwnerBot(Mob* self)
{
	return self->IsPetOwnerBot();
}

bool Perl_Mob_IsPetOwnerClient(Mob* self)
{
	return self->IsPetOwnerClient();
}

bool Perl_Mob_IsPetOwnerNPC(Mob* self)
{
	return self->IsPetOwnerNPC();
}

bool Perl_Mob_IsPetOwnerOfClientBot(Mob* self)
{
	return self->IsPetOwnerOfClientBot();
}

bool Perl_Mob_IsDestructibleObject(Mob* self)
{
	return self->IsDestructibleObject();
}

bool Perl_Mob_IsBoat(Mob* self)
{
	return self->IsBoat();
}

bool Perl_Mob_IsControllableBoat(Mob* self)
{
	return self->IsControllableBoat();
}

int Perl_Mob_GetHeroicStrikethrough(Mob* self)
{
	return self->GetHeroicStrikethrough();
}

bool Perl_Mob_IsAlwaysAggro(Mob* self)
{
	return self->AlwaysAggro();
}

std::string Perl_Mob_GetDeityName(Mob* self)
{
	return Deity::GetName(self->GetDeity());
}

perl::array Perl_Mob_GetBuffs(Mob* self)
{
	perl::array result;

	const auto &buffs = self->GetBuffs();

	for (int slot_id = 0; slot_id < self->GetMaxBuffSlots(); slot_id++) {
		result.push_back(&buffs[slot_id]);
	}

	return result;
}

void Perl_Mob_RestoreEndurance(Mob* self)
{
	self->RestoreEndurance();
}

void Perl_Mob_RestoreHealth(Mob* self)
{
	self->RestoreHealth();
}

void Perl_Mob_RestoreMana(Mob* self)
{
	self->RestoreMana();
}

std::string Perl_Mob_GetArchetypeName(Mob* self)
{
	return self->GetArchetypeName();
}

bool Perl_Mob_IsIntelligenceCasterClass(Mob* self)
{
	return self->IsIntelligenceCasterClass();
}

bool Perl_Mob_IsPureMeleeClass(Mob* self)
{
	return self->IsPureMeleeClass();
}

bool Perl_Mob_IsWisdomCasterClass(Mob* self)
{
	return self->IsWisdomCasterClass();
}

std::string Perl_Mob_GetConsiderColor(Mob* self, Mob* other)
{
	return EQ::constants::GetConsiderColorName(self->GetLevelCon(other->GetLevel()));
}

std::string Perl_Mob_GetConsiderColor(Mob* self, uint8 other_level)
{
	return EQ::constants::GetConsiderColorName(self->GetLevelCon(other_level));
}

void Perl_Mob_AreaAttack(Mob* self, float distance)
{
	entity_list.AEAttack(self, distance);
}

void Perl_Mob_AreaAttack(Mob* self, float distance, int16 slot_id)
{
	entity_list.AEAttack(self, distance, slot_id);
}

void Perl_Mob_AreaAttack(Mob* self, float distance, int16 slot_id, int count)
{
	entity_list.AEAttack(self, distance, slot_id, count);
}

void Perl_Mob_AreaAttack(Mob* self, float distance, int16 slot_id, int count, bool is_from_spell)
{
	entity_list.AEAttack(self, distance, slot_id, count, is_from_spell);
}

void Perl_Mob_AreaAttack(Mob* self, float distance, int16 slot_id, int count, bool is_from_spell, int attack_rounds)
{
	entity_list.AEAttack(self, distance, slot_id, count, is_from_spell, attack_rounds);
}

void Perl_Mob_AreaSpell(Mob* self, Mob* center, uint16 spell_id)
{
	entity_list.AESpell(self, center, spell_id, true, 0, nullptr, true);
}

void Perl_Mob_AreaSpell(Mob* self, Mob* center, uint16 spell_id, bool affect_caster)
{
	entity_list.AESpell(self, center, spell_id, affect_caster, 0, nullptr, true);
}

void Perl_Mob_AreaSpell(Mob* self, Mob* center, uint16 spell_id, bool affect_caster, int16 resist_adjust)
{
	entity_list.AESpell(self, center, spell_id, affect_caster, resist_adjust, nullptr, true);
}

void Perl_Mob_AreaSpell(Mob* self, Mob* center, uint16 spell_id, bool affect_caster, int16 resist_adjust, int max_targets)
{
	entity_list.AESpell(self, center, spell_id, affect_caster, resist_adjust, &max_targets, true);
}

void Perl_Mob_MassGroupBuff(Mob* self, Mob* center, uint16 spell_id)
{
	entity_list.MassGroupBuff(self, center, spell_id);
}

void Perl_Mob_MassGroupBuff(Mob* self, Mob* center, uint16 spell_id, bool affect_caster)
{
	entity_list.MassGroupBuff(self, center, spell_id, affect_caster);
}

void Perl_Mob_BuffFadeBeneficial(Mob* self)
{
	self->BuffFadeBeneficial();
}

void Perl_Mob_BuffFadeDetrimental(Mob* self)
{
	self->BuffFadeDetrimental();
}

void Perl_Mob_BuffFadeDetrimentalByCaster(Mob* self, Mob* caster)
{
	self->BuffFadeDetrimentalByCaster(caster);
}

void Perl_Mob_BuffFadeNonPersistDeath(Mob* self)
{
	self->BuffFadeNonPersistDeath();
}

void Perl_Mob_BuffFadeSongs(Mob* self)
{
	self->BuffFadeSongs();
}

perl::array Perl_Mob_GetPausedTimers(Mob* self)
{
	perl::array a;

	const auto& l = quest_manager.GetPausedTimers(self);
	for (const auto& v : l) {
		a.push_back(v);
	}

	return a;
}

perl::array Perl_Mob_GetTimers(Mob* self)
{
	perl::array a;

	const auto& l = quest_manager.GetTimers(self);
	for (const auto& v : l) {
		a.push_back(v);
	}

	return a;
}

void Perl_Mob_SetPetType(Mob* self, uint8 pet_type)
{
	self->SetPetType(pet_type);
}

void perl_register_mob()
{
	perl::interpreter perl(PERL_GET_THX);

	auto package = perl.new_class<Mob>("Mob");
	package.add("AddFeignMemory", &Perl_Mob_AddFeignMemory);
	package.add("AddNimbusEffect", &Perl_Mob_AddNimbusEffect);
	package.add("AddToHateList", (void(*)(Mob*, Mob*))&Perl_Mob_AddToHateList);
	package.add("AddToHateList", (void(*)(Mob*, Mob*, int64_t))&Perl_Mob_AddToHateList);
	package.add("AddToHateList", (void(*)(Mob*, Mob*, int64_t, int64_t))&Perl_Mob_AddToHateList);
	package.add("AddToHateList", (void(*)(Mob*, Mob*, int64_t, int64_t, bool))&Perl_Mob_AddToHateList);
	package.add("AddToHateList", (void(*)(Mob*, Mob*, int64_t, int64_t, bool, bool))&Perl_Mob_AddToHateList);
	package.add("AddToHateList", (void(*)(Mob*, Mob*, int64_t, int64_t, bool, bool, bool))&Perl_Mob_AddToHateList);
	package.add("ApplySpellBuff", (void(*)(Mob*, int))&Perl_Mob_ApplySpellBuff);
	package.add("ApplySpellBuff", (void(*)(Mob*, int, int))&Perl_Mob_ApplySpellBuff);
	package.add("ApplySpellBuff", (void(*)(Mob*, int, int, int))&Perl_Mob_ApplySpellBuff);
	package.add("AreaAttack", (void(*)(Mob*, float))&Perl_Mob_AreaAttack);
	package.add("AreaAttack", (void(*)(Mob*, float, int16))&Perl_Mob_AreaAttack);
	package.add("AreaAttack", (void(*)(Mob*, float, int16, int))&Perl_Mob_AreaAttack);
	package.add("AreaAttack", (void(*)(Mob*, float, int16, int, bool))&Perl_Mob_AreaAttack);
	package.add("AreaAttack", (void(*)(Mob*, float, int16, int, bool, int))&Perl_Mob_AreaAttack);
	package.add("AreaSpell", (void(*)(Mob*, Mob*, uint16))&Perl_Mob_AreaSpell);
	package.add("AreaSpell", (void(*)(Mob*, Mob*, uint16, bool))&Perl_Mob_AreaSpell);
	package.add("AreaSpell", (void(*)(Mob*, Mob*, uint16, bool, int16))&Perl_Mob_AreaSpell);
	package.add("AreaSpell", (void(*)(Mob*, Mob*, uint16, bool, int16, int))&Perl_Mob_AreaSpell);
	package.add("Attack", (bool(*)(Mob*, Mob*))&Perl_Mob_Attack);
	package.add("Attack", (bool(*)(Mob*, Mob*, int))&Perl_Mob_Attack);
	package.add("Attack", (bool(*)(Mob*, Mob*, int, bool))&Perl_Mob_Attack);
	package.add("BehindMob", (bool(*)(Mob*, Mob*))&Perl_Mob_BehindMob);
	package.add("BehindMob", (bool(*)(Mob*, Mob*, float))&Perl_Mob_BehindMob);
	package.add("BehindMob", (bool(*)(Mob*, Mob*, float, float))&Perl_Mob_BehindMob);
	package.add("BuffCount", (uint32(*)(Mob*))&Perl_Mob_BuffCount);
	package.add("BuffCount", (uint32(*)(Mob*, bool))&Perl_Mob_BuffCount);
	package.add("BuffCount", (uint32(*)(Mob*, bool, bool))&Perl_Mob_BuffCount);
	package.add("BuffFadeAll", &Perl_Mob_BuffFadeAll);
	package.add("BuffFadeBeneficial", &Perl_Mob_BuffFadeBeneficial);
	package.add("BuffFadeByEffect", (void(*)(Mob*, int))&Perl_Mob_BuffFadeByEffect);
	package.add("BuffFadeByEffect", (void(*)(Mob*, int, int))&Perl_Mob_BuffFadeByEffect);
	package.add("BuffFadeBySlot", (void(*)(Mob*, int))&Perl_Mob_BuffFadeBySlot);
	package.add("BuffFadeBySlot", (void(*)(Mob*, int, bool))&Perl_Mob_BuffFadeBySlot);
	package.add("BuffFadeBySpellID", &Perl_Mob_BuffFadeBySpellID);
	package.add("BuffFadeDetrimental", &Perl_Mob_BuffFadeDetrimental);
	package.add("BuffFadeDetrimentalByCaster", &Perl_Mob_BuffFadeDetrimentalByCaster);
	package.add("BuffFadeNonPersistDeath", &Perl_Mob_BuffFadeNonPersistDeath);
	package.add("BuffFadeSongs", &Perl_Mob_BuffFadeSongs);
	package.add("CalculateDistance", (float(*)(Mob*, float, float, float))&Perl_Mob_CalculateDistance);
	package.add("CalculateDistance", (float(*)(Mob*, Mob*))&Perl_Mob_CalculateDistance);
	package.add("CalculateHeadingToTarget", &Perl_Mob_CalculateHeadingToTarget);
	package.add("CameraEffect", (void(*)(Mob*, uint32))&Perl_Mob_CameraEffect);
	package.add("CameraEffect", (void(*)(Mob*, uint32, float))&Perl_Mob_CameraEffect);
	package.add("CameraEffect", (void(*)(Mob*, uint32, float, Client*))&Perl_Mob_CameraEffect);
	package.add("CameraEffect", (void(*)(Mob*, uint32, float, perl::nullable<Client*>, bool))&Perl_Mob_CameraEffect);
	package.add("CanBuffStack", (bool(*)(Mob*, uint16, uint8))&Perl_Mob_CanBuffStack);
	package.add("CanBuffStack", (bool(*)(Mob*, uint16, uint8, bool))&Perl_Mob_CanBuffStack);
	package.add("CanClassEquipItem", &Perl_Mob_CanClassEquipItem);
	package.add("CanRaceEquipItem", &Perl_Mob_CanRaceEquipItem);
	package.add("CanThisClassDodge", &Perl_Mob_CanThisClassDodge);
	package.add("CanThisClassDoubleAttack", &Perl_Mob_CanThisClassDoubleAttack);
	package.add("CanThisClassDualWield", &Perl_Mob_CanThisClassDualWield);
	package.add("CanThisClassParry", &Perl_Mob_CanThisClassParry);
	package.add("CanThisClassRiposte", &Perl_Mob_CanThisClassRiposte);
	package.add("CastSpell", (void(*)(Mob*, uint16, uint16))&Perl_Mob_CastSpell);
	package.add("CastSpell", (void(*)(Mob*, uint16, uint16, int))&Perl_Mob_CastSpell);
	package.add("CastSpell", (void(*)(Mob*, uint16, uint16, int, int))&Perl_Mob_CastSpell);
	package.add("CastSpell", (void(*)(Mob*, uint16, uint16, int, int, int))&Perl_Mob_CastSpell);
	package.add("CastSpell", (void(*)(Mob*, uint16, uint16, int, int, int, int16))&Perl_Mob_CastSpell);
	package.add("CastToBot", &Perl_Mob_CastToBot);
	package.add("CastToClient", &Perl_Mob_CastToClient);
	package.add("CastToCorpse", &Perl_Mob_CastToCorpse);
	package.add("CastToMob", &Perl_Mob_CastToMob);
	package.add("CastToNPC", &Perl_Mob_CastToNPC);
	package.add("CastingSpellID", &Perl_Mob_CastingSpellID);
	package.add("ChangeSize", (void(*)(Mob*, float))&Perl_Mob_ChangeSize);
	package.add("ChangeSize", (void(*)(Mob*, float, bool))&Perl_Mob_ChangeSize);
	package.add("Charmed", &Perl_Mob_Charmed);
	package.add("CheckAggro", &Perl_Mob_CheckAggro);
	package.add("CheckAggroAmount", &Perl_Mob_CheckAggroAmount);
	package.add("CheckHealAggroAmount", (int(*)(Mob*, uint16))&Perl_Mob_CheckHealAggroAmount);
	package.add("CheckHealAggroAmount", (int(*)(Mob*, uint16, uint32))&Perl_Mob_CheckHealAggroAmount);
	package.add("CheckLoS", &Perl_Mob_CheckLoS);
	package.add("CheckLoSToLoc", (bool(*)(Mob*, float, float, float))&Perl_Mob_CheckLoSToLoc);
	package.add("CheckLoSToLoc", (bool(*)(Mob*, float, float, float, float))&Perl_Mob_CheckLoSToLoc);
	package.add("ClearEntityVariables", &Perl_Mob_ClearEntityVariables);
	package.add("ClearFeignMemory", &Perl_Mob_ClearFeignMemory);
	package.add("ClearSpecialAbilities", &Perl_Mob_ClearSpecialAbilities);
	package.add("CloneAppearance", (void(*)(Mob*, Mob*))&Perl_Mob_CloneAppearance);
	package.add("CloneAppearance", (void(*)(Mob*, Mob*, bool))&Perl_Mob_CloneAppearance);
	package.add("CombatRange", &Perl_Mob_CombatRange);
	package.add("CopyHateList", &Perl_Mob_CopyHateList);
	package.add("Damage", (void(*)(Mob*, Mob*, int64, uint16_t, int))&Perl_Mob_Damage);
	package.add("Damage", (void(*)(Mob*, Mob*, int64, uint16_t, int, bool))&Perl_Mob_Damage);
	package.add("Damage", (void(*)(Mob*, Mob*, int64, uint16_t, int, bool, int8_t))&Perl_Mob_Damage);
	package.add("Damage", (void(*)(Mob*, Mob*, int64, uint16_t, int, bool, int8_t, bool))&Perl_Mob_Damage);
	package.add("DamageArea", (void(*)(Mob*, int64))&Perl_Mob_DamageArea);
	package.add("DamageArea", (void(*)(Mob*, int64, uint32))&Perl_Mob_DamageArea);
	package.add("DamageAreaBots", (void(*)(Mob*, int64))&Perl_Mob_DamageAreaBots);
	package.add("DamageAreaBots", (void(*)(Mob*, int64, uint32))&Perl_Mob_DamageAreaBots);
	package.add("DamageAreaBotsPercentage", (void(*)(Mob*, int64))&Perl_Mob_DamageAreaBotsPercentage);
	package.add("DamageAreaBotsPercentage", (void(*)(Mob*, int64, uint32))&Perl_Mob_DamageAreaBotsPercentage);
	package.add("DamageAreaClients", (void(*)(Mob*, int64))&Perl_Mob_DamageAreaClients);
	package.add("DamageAreaClients", (void(*)(Mob*, int64, uint32))&Perl_Mob_DamageAreaClients);
	package.add("DamageAreaClientsPercentage", (void(*)(Mob*, int64))&Perl_Mob_DamageAreaClientsPercentage);
	package.add("DamageAreaClientsPercentage", (void(*)(Mob*, int64, uint32))&Perl_Mob_DamageAreaClientsPercentage);
	package.add("DamageAreaNPCs", (void(*)(Mob*, int64))&Perl_Mob_DamageAreaNPCs);
	package.add("DamageAreaNPCs", (void(*)(Mob*, int64, uint32))&Perl_Mob_DamageAreaNPCs);
	package.add("DamageAreaNPCsPercentage", (void(*)(Mob*, int64))&Perl_Mob_DamageAreaNPCsPercentage);
	package.add("DamageAreaNPCsPercentage", (void(*)(Mob*, int64, uint32))&Perl_Mob_DamageAreaNPCsPercentage);
	package.add("DamageAreaPercentage", (void(*)(Mob*, int64))&Perl_Mob_DamageAreaPercentage);
	package.add("DamageAreaPercentage", (void(*)(Mob*, int64, uint32))&Perl_Mob_DamageAreaPercentage);
	package.add("DamageHateList", (void(*)(Mob*, int64))&Perl_Mob_DamageHateList);
	package.add("DamageHateList", (void(*)(Mob*, int64, uint32))&Perl_Mob_DamageHateList);
	package.add("DamageHateListBots", (void(*)(Mob*, int64))&Perl_Mob_DamageHateListBots);
	package.add("DamageHateListBots", (void(*)(Mob*, int64, uint32))&Perl_Mob_DamageHateListBots);
	package.add("DamageHateListBotsPercentage", (void(*)(Mob*, int64))&Perl_Mob_DamageHateListBotsPercentage);
	package.add("DamageHateListBotsPercentage", (void(*)(Mob*, int64, uint32))&Perl_Mob_DamageHateListBotsPercentage);
	package.add("DamageHateListClients", (void(*)(Mob*, int64))&Perl_Mob_DamageHateListClients);
	package.add("DamageHateListClients", (void(*)(Mob*, int64, uint32))&Perl_Mob_DamageHateListClients);
	package.add("DamageHateListClientsPercentage", (void(*)(Mob*, int64))&Perl_Mob_DamageHateListClientsPercentage);
	package.add("DamageHateListClientsPercentage", (void(*)(Mob*, int64, uint32))&Perl_Mob_DamageHateListClientsPercentage);
	package.add("DamageHateListNPCs", (void(*)(Mob*, int64))&Perl_Mob_DamageHateListNPCs);
	package.add("DamageHateListNPCs", (void(*)(Mob*, int64, uint32))&Perl_Mob_DamageHateListNPCs);
	package.add("DamageHateListNPCsPercentage", (void(*)(Mob*, int64))&Perl_Mob_DamageHateListNPCsPercentage);
	package.add("DamageHateListNPCsPercentage", (void(*)(Mob*, int64, uint32))&Perl_Mob_DamageHateListNPCsPercentage);
	package.add("DamageHateListPercentage", (void(*)(Mob*, int64))&Perl_Mob_DamageHateListPercentage);
	package.add("DamageHateListPercentage", (void(*)(Mob*, int64, uint32))&Perl_Mob_DamageHateListPercentage);
	package.add("DelGlobal", &Perl_Mob_DelGlobal);
	package.add("DeleteBucket", &Perl_Mob_DeleteBucket);
	package.add("DeleteEntityVariable", &Perl_Mob_DeleteEntityVariable);
	package.add("Depop", (void(*)(Mob*))&Perl_Mob_Depop);
	package.add("Depop", (void(*)(Mob*, bool))&Perl_Mob_Depop);
	package.add("DivineAura", &Perl_Mob_DivineAura);
	package.add("DoAnim", (void(*)(Mob*, int))&Perl_Mob_DoAnim);
	package.add("DoAnim", (void(*)(Mob*, int, int))&Perl_Mob_DoAnim);
	package.add("DoAnim", (void(*)(Mob*, int, int, bool))&Perl_Mob_DoAnim);
	package.add("DoAnim", (void(*)(Mob*, int, int, bool, int))&Perl_Mob_DoAnim);
	package.add("DoArcheryAttackDmg", &Perl_Mob_DoArcheryAttackDmg);
	package.add("DoKnockback", &Perl_Mob_DoKnockback);
	package.add("DoMeleeSkillAttackDmg", &Perl_Mob_DoMeleeSkillAttackDmg);
	package.add("DoSpecialAttackDamage", (void(*)(Mob*, Mob*, int, int))&Perl_Mob_DoSpecialAttackDamage);
	package.add("DoSpecialAttackDamage", (void(*)(Mob*, Mob*, int, int, int))&Perl_Mob_DoSpecialAttackDamage);
	package.add("DoSpecialAttackDamage", (void(*)(Mob*, Mob*, int, int, int, int))&Perl_Mob_DoSpecialAttackDamage);
	package.add("DoSpecialAttackDamage", (void(*)(Mob*, Mob*, int, int, int, int, int))&Perl_Mob_DoSpecialAttackDamage);
	package.add("DoThrowingAttackDmg", &Perl_Mob_DoThrowingAttackDmg);
	package.add("DontBuffMeBefore", &Perl_Mob_DontBuffMeBefore);
	package.add("DontDotMeBefore", &Perl_Mob_DontDotMeBefore);
	package.add("DontHealMeBefore", &Perl_Mob_DontHealMeBefore);
	package.add("DontRootMeBefore", &Perl_Mob_DontRootMeBefore);
	package.add("DontSnareMeBefore", &Perl_Mob_DontSnareMeBefore);
	package.add("DoubleAggro", &Perl_Mob_DoubleAggro);
	package.add("Emote", &Perl_Mob_Emote);
	package.add("EntityVariableExists", &Perl_Mob_EntityVariableExists);
	package.add("FaceTarget", (void(*)(Mob*))&Perl_Mob_FaceTarget);
	package.add("FaceTarget", (void(*)(Mob*, Mob*))&Perl_Mob_FaceTarget);
	package.add("FindBuff", (bool(*)(Mob*, uint16))&Perl_Mob_FindBuff);
	package.add("FindBuff", (bool(*)(Mob*, uint16, uint16))&Perl_Mob_FindBuff);
	package.add("FindBuffBySlot", &Perl_Mob_FindBuffBySlot);
	package.add("FindGroundZ", (float(*)(Mob*, float, float))&Perl_Mob_FindGroundZ);
	package.add("FindGroundZ", (float(*)(Mob*, float, float, float))&Perl_Mob_FindGroundZ);
	package.add("FindType", (bool(*)(Mob*, uint16_t))&Perl_Mob_FindType);
	package.add("FindType", (bool(*)(Mob*, uint16_t, bool))&Perl_Mob_FindType);
	package.add("FindType", (bool(*)(Mob*, uint16_t, bool, uint16_t))&Perl_Mob_FindType);
	package.add("GMMove", (void(*)(Mob*, float, float, float))&Perl_Mob_GMMove);
	package.add("GMMove", (void(*)(Mob*, float, float, float, float))&Perl_Mob_GMMove);
	package.add("GMMove", (void(*)(Mob*, float, float, float, float, bool))&Perl_Mob_GMMove);
	package.add("Gate", &Perl_Mob_Gate);
	package.add("GetAA", &Perl_Mob_GetAA);
	package.add("GetAABonuses", &Perl_Mob_GetAABonuses);
	package.add("GetAAByAAID", &Perl_Mob_GetAAByAAID);
	package.add("GetAC", &Perl_Mob_GetAC);
	package.add("GetAGI", &Perl_Mob_GetAGI);
	package.add("GetATK", &Perl_Mob_GetATK);
	package.add("GetActDoTDamage", (int64_t(*)(Mob*, uint16, int64, Mob*))&Perl_Mob_GetActDoTDamage);
	package.add("GetActDoTDamage", (int64_t(*)(Mob*, uint16, int64, Mob*, bool))&Perl_Mob_GetActDoTDamage);
	package.add("GetActReflectedSpellDamage", &Perl_Mob_GetActReflectedSpellDamage);
	package.add("GetActSpellCasttime", &Perl_Mob_GetActSpellCasttime);
	package.add("GetActSpellCost", &Perl_Mob_GetActSpellCost);
	package.add("GetActSpellDamage", (int64_t(*)(Mob*, uint16, int64))&Perl_Mob_GetActSpellDamage);
	package.add("GetActSpellDamage", (int64_t(*)(Mob*, uint16, int64, Mob*))&Perl_Mob_GetActSpellDamage);
	package.add("GetActSpellDuration", &Perl_Mob_GetActSpellDuration);
	package.add("GetActSpellHealing", (int64_t(*)(Mob*, uint16, int64))&Perl_Mob_GetActSpellHealing);
	package.add("GetActSpellHealing", (int64_t(*)(Mob*, uint16, int64, Mob*))&Perl_Mob_GetActSpellHealing);
	package.add("GetActSpellHealing", (int64_t(*)(Mob*, uint16, int64, Mob*, bool))&Perl_Mob_GetActSpellHealing);
	package.add("GetActSpellRange", &Perl_Mob_GetActSpellRange);
	package.add("GetAggroRange", &Perl_Mob_GetAggroRange);
	package.add("GetAllowBeneficial", &Perl_Mob_GetAllowBeneficial);
	package.add("GetAppearance", &Perl_Mob_GetAppearance);
	package.add("GetArchetypeName", &Perl_Mob_GetArchetypeName);
	package.add("GetArmorTint", &Perl_Mob_GetArmorTint);
	package.add("GetAssistRange", &Perl_Mob_GetAssistRange);
	package.add("GetBaseGender", &Perl_Mob_GetBaseGender);
	package.add("GetBaseRace", &Perl_Mob_GetBaseRace);
	package.add("GetBaseRaceName", &Perl_Mob_GetBaseRaceName);
	package.add("GetBaseSize", &Perl_Mob_GetBaseSize);
	package.add("GetBeard", &Perl_Mob_GetBeard);
	package.add("GetBeardColor", &Perl_Mob_GetBeardColor);
	package.add("GetBodyType", &Perl_Mob_GetBodyType);
	package.add("GetBucket", &Perl_Mob_GetBucket);
	package.add("GetBucketExpires", &Perl_Mob_GetBucketExpires);
	package.add("GetBucketKey", &Perl_Mob_GetBucketKey);
	package.add("GetBucketRemaining", &Perl_Mob_GetBucketRemaining);
	package.add("GetBuffs", &Perl_Mob_GetBuffs);
	package.add("GetBuffSlotFromType", &Perl_Mob_GetBuffSlotFromType);
	package.add("GetBuffSpellIDs", &Perl_Mob_GetBuffSpellIDs);
	package.add("GetBuffStatValueBySpell", &Perl_Mob_GetBuffStatValueBySpell);
	package.add("GetBuffStatValueBySlot", &Perl_Mob_GetBuffStatValueBySlot);
	package.add("GetCHA", &Perl_Mob_GetCHA);
	package.add("GetCR", &Perl_Mob_GetCR);
	package.add("GetCasterLevel", &Perl_Mob_GetCasterLevel);
	package.add("GetClass", &Perl_Mob_GetClass);
	package.add("GetClassLevelFactor", &Perl_Mob_GetClassLevelFactor);
	package.add("GetClassPlural", &Perl_Mob_GetClassPlural);
	package.add("GetClassName", &Perl_Mob_GetClassName);
	package.add("GetCleanName", &Perl_Mob_GetCleanName);
	package.add("GetCloseMobList", (perl::array(*)(Mob*))&Perl_Mob_GetCloseMobList);
	package.add("GetCloseMobList", (perl::array(*)(Mob*, float))&Perl_Mob_GetCloseMobList);
	package.add("GetCloseMobList", (perl::array(*)(Mob*, float, bool))&Perl_Mob_GetCloseMobList);
	package.add("GetConsiderColor", (std::string(*)(Mob*, Mob*))&Perl_Mob_GetConsiderColor);
	package.add("GetConsiderColor", (std::string(*)(Mob*, uint8))&Perl_Mob_GetConsiderColor);
	package.add("GetCorruption", &Perl_Mob_GetCorruption);
	package.add("GetDefaultRaceSize", (float(*)(Mob*))&Perl_Mob_GetDefaultRaceSize);
	package.add("GetDefaultRaceSize", (float(*)(Mob*, int))&Perl_Mob_GetDefaultRaceSize);
	package.add("GetDefaultRaceSize", (float(*)(Mob*, int, int))&Perl_Mob_GetDefaultRaceSize);
	package.add("GetDEX", &Perl_Mob_GetDEX);
	package.add("GetDR", &Perl_Mob_GetDR);
	package.add("GetDamageAmount", &Perl_Mob_GetDamageAmount);
	package.add("GetDeity", &Perl_Mob_GetDeity);
	package.add("GetDeityName", &Perl_Mob_GetDeityName);
	package.add("GetDisplayAC", &Perl_Mob_GetDisplayAC);
	package.add("GetDrakkinDetails", &Perl_Mob_GetDrakkinDetails);
	package.add("GetDrakkinHeritage", &Perl_Mob_GetDrakkinHeritage);
	package.add("GetDrakkinTattoo", &Perl_Mob_GetDrakkinTattoo);
	package.add("GetEntityVariable", &Perl_Mob_GetEntityVariable);
	package.add("GetEntityVariables", &Perl_Mob_GetEntityVariables);
	package.add("GetEquipment", &Perl_Mob_GetEquipment);
	package.add("GetEquipmentColor", &Perl_Mob_GetEquipmentColor);
	package.add("GetEquipmentMaterial", &Perl_Mob_GetEquipmentMaterial);
	package.add("GetExtraHaste", &Perl_Mob_GetExtraHaste);
	package.add("GetEyeColor1", &Perl_Mob_GetEyeColor1);
	package.add("GetEyeColor2", &Perl_Mob_GetEyeColor2);
	package.add("GetFR", &Perl_Mob_GetFR);
	package.add("GetFlurryChance", &Perl_Mob_GetFlurryChance);
	package.add("GetFollowID", &Perl_Mob_GetFollowID);
	package.add("GetGender", &Perl_Mob_GetGender);
	package.add("GetGlobal", &Perl_Mob_GetGlobal);
	package.add("GetHP", &Perl_Mob_GetHP);
	package.add("GetHPRatio", &Perl_Mob_GetHPRatio);
	package.add("GetHairColor", &Perl_Mob_GetHairColor);
	package.add("GetHairStyle", &Perl_Mob_GetHairStyle);
	package.add("GetHandToHandDamage", &Perl_Mob_GetHandToHandDamage);
	package.add("GetHandToHandDelay", &Perl_Mob_GetHandToHandDelay);
	package.add("GetHaste", &Perl_Mob_GetHaste);
	package.add("GetHateAmount", (int64_t(*)(Mob*, Mob*))&Perl_Mob_GetHateAmount);
	package.add("GetHateAmount", (int64_t(*)(Mob*, Mob*, bool))&Perl_Mob_GetHateAmount);
	package.add("GetHateClosest", (Mob*(*)(Mob*))&Perl_Mob_GetHateClosest);
	package.add("GetHateClosest", (Mob*(*)(Mob*, bool))&Perl_Mob_GetHateClosest);
	package.add("GetHateClosestBot", (Bot*(*)(Mob*))&Perl_Mob_GetHateClosestBot);
	package.add("GetHateClosestBot", (Bot*(*)(Mob*, bool))&Perl_Mob_GetHateClosestBot);
	package.add("GetHateClosestClient", (Client*(*)(Mob*))&Perl_Mob_GetHateClosestClient);
	package.add("GetHateClosestClient", (Client*(*)(Mob*, bool))&Perl_Mob_GetHateClosestClient);
	package.add("GetHateClosestNPC", (NPC*(*)(Mob*))&Perl_Mob_GetHateClosestNPC);
	package.add("GetHateClosestNPC", (NPC*(*)(Mob*, bool))&Perl_Mob_GetHateClosestNPC);
	package.add("GetHateDamageTop", &Perl_Mob_GetHateDamageTop);
	package.add("GetHateList", &Perl_Mob_GetHateList);
	package.add("GetHateListBots", (perl::array(*)(Mob*))&Perl_Mob_GetHateListBots);
	package.add("GetHateListBots", (perl::array(*)(Mob*, uint32))&Perl_Mob_GetHateListBots);
	package.add("GetHateListBotCount", &Perl_Mob_GetHateListBotCount);
	package.add("GetHateListClients", (perl::array(*)(Mob*))&Perl_Mob_GetHateListClients);
	package.add("GetHateListClients", (perl::array(*)(Mob*, uint32))&Perl_Mob_GetHateListClients);
	package.add("GetHateListClientCount", &Perl_Mob_GetHateListClientCount);
	package.add("GetHateListNPCs", (perl::array(*)(Mob*))&Perl_Mob_GetHateListNPCs);
	package.add("GetHateListNPCs", (perl::array(*)(Mob*, uint32))&Perl_Mob_GetHateListNPCs);
	package.add("GetHateListNPCCount", &Perl_Mob_GetHateListNPCCount);
	package.add("GetHateListByDistance", (perl::array(*)(Mob*))&Perl_Mob_GetHateListByDistance);
	package.add("GetHateListByDistance", (perl::array(*)(Mob*, uint32))&Perl_Mob_GetHateListByDistance);
	package.add("GetHateListCount", &Perl_Mob_GetHateListCount);
	package.add("GetHateRandom", &Perl_Mob_GetHateRandom);
	package.add("GetHateRandomBot", &Perl_Mob_GetHateRandomBot);
	package.add("GetHateRandomClient", &Perl_Mob_GetHateRandomClient);
	package.add("GetHateRandomNPC", &Perl_Mob_GetHateRandomNPC);
	package.add("GetHateTop", &Perl_Mob_GetHateTop);
	package.add("GetHateTopBot", &Perl_Mob_GetHateTopBot);
	package.add("GetHateTopClient", &Perl_Mob_GetHateTopClient);
	package.add("GetHateTopNPC", &Perl_Mob_GetHateTopNPC);
	package.add("GetHeading", &Perl_Mob_GetHeading);
	package.add("GetHelmTexture", &Perl_Mob_GetHelmTexture);
	package.add("GetHeroicStrikethrough", &Perl_Mob_GetHeroicStrikethrough);
	package.add("GetHerosForgeModel", &Perl_Mob_GetHerosForgeModel);
	package.add("GetID", &Perl_Mob_GetID);
	package.add("GetINT", &Perl_Mob_GetINT);
	package.add("GetInvisibleLevel", &Perl_Mob_GetInvisibleLevel);
	package.add("GetInvisibleUndeadLevel", &Perl_Mob_GetInvisibleUndeadLevel);
	package.add("GetInvul", &Perl_Mob_GetInvul);
	package.add("GetItemBonuses", &Perl_Mob_GetItemBonuses);
	package.add("GetItemHPBonuses", &Perl_Mob_GetItemHPBonuses);
	package.add("GetItemStat", &Perl_Mob_GetItemStat);
	package.add("GetLastName", &Perl_Mob_GetLastName);
	package.add("GetLevel", &Perl_Mob_GetLevel);
	package.add("GetLevelCon", &Perl_Mob_GetLevelCon);
	package.add("GetLevelHP", &Perl_Mob_GetLevelHP);
	package.add("GetLuclinFace", &Perl_Mob_GetLuclinFace);
	package.add("GetMR", &Perl_Mob_GetMR);
	package.add("GetMana", &Perl_Mob_GetMana);
	package.add("GetManaRatio", &Perl_Mob_GetManaRatio);
	package.add("GetMaxAGI", &Perl_Mob_GetMaxAGI);
	package.add("GetMaxCHA", &Perl_Mob_GetMaxCHA);
	package.add("GetMaxDEX", &Perl_Mob_GetMaxDEX);
	package.add("GetMaxHP", &Perl_Mob_GetMaxHP);
	package.add("GetMaxINT", &Perl_Mob_GetMaxINT);
	package.add("GetMaxMana", &Perl_Mob_GetMaxMana);
	package.add("GetMaxSTA", &Perl_Mob_GetMaxSTA);
	package.add("GetMaxSTR", &Perl_Mob_GetMaxSTR);
	package.add("GetMaxWIS", &Perl_Mob_GetMaxWIS);
	package.add("GetMeleeMitigation", &Perl_Mob_GetMeleeMitigation);
	package.add("GetMobTypeIdentifier", &Perl_Mob_GetMobTypeIdentifier);
	package.add("GetModSkillDmgTaken", &Perl_Mob_GetModSkillDmgTaken);
	package.add("GetModVulnerability", &Perl_Mob_GetModVulnerability);
	package.add("GetNPCTypeID", &Perl_Mob_GetNPCTypeID);
	package.add("GetName", &Perl_Mob_GetName);
	package.add("GetNimbusEffect1", &Perl_Mob_GetNimbusEffect1);
	package.add("GetNimbusEffect2", &Perl_Mob_GetNimbusEffect2);
	package.add("GetNimbusEffect3", &Perl_Mob_GetNimbusEffect3);
	package.add("GetOwner", &Perl_Mob_GetOwner);
	package.add("GetOwnerID", &Perl_Mob_GetOwnerID);
	package.add("GetPR", &Perl_Mob_GetPR);
	package.add("GetPausedTimers", &Perl_Mob_GetPausedTimers);
	package.add("GetPet", &Perl_Mob_GetPet);
	package.add("GetPetID", &Perl_Mob_GetPetID);
	package.add("GetPetOrder", &Perl_Mob_GetPetOrder);
	package.add("GetPetType", &Perl_Mob_GetPetType);
	package.add("GetPetTypeName", &Perl_Mob_GetPetTypeName);
	package.add("GetPhR", &Perl_Mob_GetPhR);
	package.add("GetRace", &Perl_Mob_GetRace);
	package.add("GetRaceName", &Perl_Mob_GetRaceName);
	package.add("GetRacePlural", &Perl_Mob_GetRacePlural);
	package.add("GetRemainingTimeMS", &Perl_Mob_GetRemainingTimeMS);
	package.add("GetResist", &Perl_Mob_GetResist);
	package.add("GetReverseFactionCon", &Perl_Mob_GetReverseFactionCon);
	package.add("GetRunAnimSpeed", &Perl_Mob_GetRunAnimSpeed);
	package.add("GetRunspeed", &Perl_Mob_GetRunspeed);
	package.add("GetSTA", &Perl_Mob_GetSTA);
	package.add("GetSTR", &Perl_Mob_GetSTR);
	package.add("GetSize", &Perl_Mob_GetSize);
	package.add("GetSkill", &Perl_Mob_GetSkill);
	package.add("GetSkillDmgAmt", &Perl_Mob_GetSkillDmgAmt);
	package.add("GetSkillDmgTaken", &Perl_Mob_GetSkillDmgTaken);
	package.add("GetSpecialAbility", &Perl_Mob_GetSpecialAbility);
	package.add("GetSpecialAbilityParam", &Perl_Mob_GetSpecialAbilityParam);
	package.add("GetSpecializeSkillValue", &Perl_Mob_GetSpecializeSkillValue);
	package.add("GetSpellBonuses", &Perl_Mob_GetSpellBonuses);
	package.add("GetSpellHPBonuses", &Perl_Mob_GetSpellHPBonuses);
	package.add("GetSpellIDFromSlot", &Perl_Mob_GetSpellIDFromSlot);
	package.add("GetSpellStat", (int(*)(Mob*, uint32, const char*))&Perl_Mob_GetSpellStat);
	package.add("GetSpellStat", (int(*)(Mob*, uint32, const char*, uint8))&Perl_Mob_GetSpellStat);
	package.add("GetTarget", &Perl_Mob_GetTarget);
	package.add("GetTexture", &Perl_Mob_GetTexture);
	package.add("GetTimerDurationMS", &Perl_Mob_GetTimerDurationMS);
	package.add("GetTimers", &Perl_Mob_GetTimers);
	package.add("GetUltimateOwner", &Perl_Mob_GetUltimateOwner);
	package.add("GetWIS", &Perl_Mob_GetWIS);
	package.add("GetWalkspeed", &Perl_Mob_GetWalkspeed);
	package.add("GetWaypointH", &Perl_Mob_GetWaypointH);
	package.add("GetWaypointID", &Perl_Mob_GetWaypointID);
	package.add("GetWaypointPause", &Perl_Mob_GetWaypointPause);
	package.add("GetWaypointX", &Perl_Mob_GetWaypointX);
	package.add("GetWaypointY", &Perl_Mob_GetWaypointY);
	package.add("GetWaypointZ", &Perl_Mob_GetWaypointZ);
	package.add("GetX", &Perl_Mob_GetX);
	package.add("GetY", &Perl_Mob_GetY);
	package.add("GetZ", &Perl_Mob_GetZ);
	package.add("GetZoneID", &Perl_Mob_GetZoneID);
	package.add("GoToBind", &Perl_Mob_GoToBind);
	package.add("HalveAggro", &Perl_Mob_HalveAggro);
	package.add("HasNPCSpecialAtk", &Perl_Mob_HasNPCSpecialAtk);
	package.add("HasOwner", &Perl_Mob_HasOwner);
	package.add("HasPet", &Perl_Mob_HasPet);
	package.add("HasProcs", &Perl_Mob_HasProcs);
	package.add("HasShieldEquipped", &Perl_Mob_HasShieldEquipped);
	package.add("HasSpellEffect", &Perl_Mob_HasSpellEffect);
	package.add("HasTimer", &Perl_Mob_HasTimer);
	package.add("HasTwoHandBluntEquipped", &Perl_Mob_HasTwoHandBluntEquipped);
	package.add("HasTwoHanderEquipped", &Perl_Mob_HasTwoHanderEquipped);
	package.add("HateSummon", &Perl_Mob_HateSummon);
	package.add("Heal", &Perl_Mob_RestoreHealth);
	package.add("HealDamage", (void(*)(Mob*, int64_t))&Perl_Mob_HealDamage);
	package.add("HealDamage", (void(*)(Mob*, int64_t, Mob*))&Perl_Mob_HealDamage);
	package.add("InterruptSpell", (void(*)(Mob*))&Perl_Mob_InterruptSpell);
	package.add("InterruptSpell", (void(*)(Mob*, uint16))&Perl_Mob_InterruptSpell);
	package.add("IsAIControlled", &Perl_Mob_IsAIControlled);
	package.add("IsAlwaysAggro", &Perl_Mob_IsAlwaysAggro);
	package.add("IsAmnesiad", &Perl_Mob_IsAmnesiad);
	package.add("IsAnimation", &Perl_Mob_IsAnimation);
	package.add("IsAttackAllowed", (bool(*)(Mob*, Mob*))&Perl_Mob_IsAttackAllowed);
	package.add("IsAttackAllowed", (bool(*)(Mob*, Mob*, bool))&Perl_Mob_IsAttackAllowed);
	package.add("IsAura", &Perl_Mob_IsAura);
	package.add("IsBeacon", &Perl_Mob_IsBeacon);
	package.add("IsBeneficialAllowed", &Perl_Mob_IsBeneficialAllowed);
	package.add("IsBerserk", &Perl_Mob_IsBerserk);
	package.add("IsBlind", &Perl_Mob_IsBlind);
	package.add("IsBoat", &Perl_Mob_IsBoat);
	package.add("IsBot", &Perl_Mob_IsBot);
	package.add("IsCasting", &Perl_Mob_IsCasting);
	package.add("IsCharmed", &Perl_Mob_IsCharmed);
	package.add("IsClient", &Perl_Mob_IsClient);
	package.add("IsControllableBoat", &Perl_Mob_IsControllableBoat);
	package.add("IsCorpse", &Perl_Mob_IsCorpse);
	package.add("IsDestructibleObject", &Perl_Mob_IsDestructibleObject);
	package.add("IsDoor", &Perl_Mob_IsDoor);
	package.add("IsEliteMaterialItem", &Perl_Mob_IsEliteMaterialItem);
	package.add("IsEncounter", &Perl_Mob_IsEncounter);
	package.add("IsEngaged", &Perl_Mob_IsEngaged);
	package.add("IsEnraged", &Perl_Mob_IsEnraged);
	package.add("IsFamiliar", &Perl_Mob_IsFamiliar);
	package.add("IsFeared", &Perl_Mob_IsFeared);
	package.add("IsFindable", &Perl_Mob_IsFindable);
	package.add("IsHorse", &Perl_Mob_IsHorse);
	package.add("IsImmuneToSpell", &Perl_Mob_IsImmuneToSpell);
	package.add("IsIntelligenceCasterClass", &Perl_Mob_IsIntelligenceCasterClass);
	package.add("IsInvisible", (bool(*)(Mob*))&Perl_Mob_IsInvisible);
	package.add("IsInvisible", (bool(*)(Mob*, Mob*))&Perl_Mob_IsInvisible);
	package.add("IsMeleeDisabled", &Perl_Mob_IsMeleeDisabled);
	package.add("IsMerc", &Perl_Mob_IsMerc);
	package.add("IsMezzed", &Perl_Mob_IsMezzed);
	package.add("IsMob", &Perl_Mob_IsMob);
	package.add("IsMoving", &Perl_Mob_IsMoving);
	package.add("IsNPC", &Perl_Mob_IsNPC);
	package.add("IsNPCCorpse", &Perl_Mob_IsNPCCorpse);
	package.add("IsObject", &Perl_Mob_IsObject);
	package.add("IsOfClientBot", &Perl_Mob_IsOfClientBot);
	package.add("IsOfClientBotMerc", &Perl_Mob_IsOfClientBotMerc);
	package.add("IsPausedTimer", &Perl_Mob_IsPausedTimer);
	package.add("IsPet", &Perl_Mob_IsPet);
	package.add("IsPetOwnerBot", &Perl_Mob_IsPetOwnerBot);
	package.add("IsPetOwnerClient", &Perl_Mob_IsPetOwnerClient);
	package.add("IsPetOwnerNPC", &Perl_Mob_IsPetOwnerNPC);
	package.add("IsPetOwnerOfClientBot", &Perl_Mob_IsPetOwnerOfClientBot);
	package.add("IsPlayerCorpse", &Perl_Mob_IsPlayerCorpse);
	package.add("IsPureMeleeClass", &Perl_Mob_IsPureMeleeClass);
	package.add("IsRoamer", &Perl_Mob_IsRoamer);
	package.add("IsRooted", &Perl_Mob_IsRooted);
	package.add("IsRunning", &Perl_Mob_IsRunning);
	package.add("IsSilenced", &Perl_Mob_IsSilenced);
	package.add("IsStunned", &Perl_Mob_IsStunned);
	package.add("IsTargetable", &Perl_Mob_IsTargetable);
	package.add("IsTargeted", &Perl_Mob_IsTargeted);
	package.add("IsTargetLockPet", &Perl_Mob_IsTargetLockPet);
	package.add("IsTemporaryPet", &Perl_Mob_IsTemporaryPet);
	package.add("IsTrackable", &Perl_Mob_IsTrackable);
	package.add("IsTrap", &Perl_Mob_IsTrap);
	package.add("IsWarriorClass", &Perl_Mob_IsWarriorClass);
	package.add("IsWisdomCasterClass", &Perl_Mob_IsWisdomCasterClass);
	package.add("Kill", &Perl_Mob_Kill);
	package.add("MakePet", (void(*)(Mob*, uint16, const char*))&Perl_Mob_MakePet);
	package.add("MakePet", (void(*)(Mob*, uint16, const char*, const char*))&Perl_Mob_MakePet);
	package.add("MakeTempPet", (void(*)(Mob*, uint16))&Perl_Mob_MakeTempPet);
	package.add("MakeTempPet", (void(*)(Mob*, uint16, const char*))&Perl_Mob_MakeTempPet);
	package.add("MakeTempPet", (void(*)(Mob*, uint16, const char*, uint32))&Perl_Mob_MakeTempPet);
	package.add("MakeTempPet", (void(*)(Mob*, uint16, const char*, uint32, Mob*))&Perl_Mob_MakeTempPet);
	package.add("MakeTempPet", (void(*)(Mob*, uint16, const char*, uint32, Mob*, bool))&Perl_Mob_MakeTempPet);
	package.add("MassGroupBuff", (void(*)(Mob*, Mob*, uint16))&Perl_Mob_MassGroupBuff);
	package.add("MassGroupBuff", (void(*)(Mob*, Mob*, uint16, bool))&Perl_Mob_MassGroupBuff);
	package.add("Mesmerize", &Perl_Mob_Mesmerize);
	package.add("Message", &Perl_Mob_Message);
	package.add("Message_StringID", (void(*)(Mob*, uint32, uint32))&Perl_Mob_Message_StringID);
	package.add("Message_StringID", (void(*)(Mob*, uint32, uint32, uint32))&Perl_Mob_Message_StringID);
	package.add("ModSkillDmgTaken", &Perl_Mob_ModSkillDmgTaken);
	package.add("ModVulnerability", &Perl_Mob_ModVulnerability);
	package.add("NPCSpecialAttacks", (void(*)(Mob*, const char*, int))&Perl_Mob_NPCSpecialAttacks);
	package.add("NPCSpecialAttacks", (void(*)(Mob*, const char*, int, bool))&Perl_Mob_NPCSpecialAttacks);
	package.add("NPCSpecialAttacks", (void(*)(Mob*, const char*, int, bool, bool))&Perl_Mob_NPCSpecialAttacks);
	package.add("NavigateTo", &Perl_Mob_NavigateTo);
	package.add("PauseTimer", &Perl_Mob_PauseTimer);
	package.add("ProcessSpecialAbilities", &Perl_Mob_ProcessSpecialAbilities);
	package.add("ProjectileAnim", (void(*)(Mob*, Mob*, int))&Perl_Mob_ProjectileAnim);
	package.add("ProjectileAnim", (void(*)(Mob*, Mob*, int, bool))&Perl_Mob_ProjectileAnim);
	package.add("ProjectileAnim", (void(*)(Mob*, Mob*, int, bool, float))&Perl_Mob_ProjectileAnim);
	package.add("ProjectileAnim", (void(*)(Mob*, Mob*, int, bool, float, float))&Perl_Mob_ProjectileAnim);
	package.add("ProjectileAnim", (void(*)(Mob*, Mob*, int, bool, float, float, float))&Perl_Mob_ProjectileAnim);
	package.add("ProjectileAnim", (void(*)(Mob*, Mob*, int, bool, float, float, float, float))&Perl_Mob_ProjectileAnim);
	package.add("ProjectileAnim", (void(*)(Mob*, Mob*, int, bool, float, float, float, float, const char*))&Perl_Mob_ProjectileAnim);
	package.add("RandomizeFeatures", (bool(*)(Mob*))&Perl_Mob_RandomizeFeatures);
	package.add("RandomizeFeatures", (bool(*)(Mob*, bool))&Perl_Mob_RandomizeFeatures);
	package.add("RandomizeFeatures", (bool(*)(Mob*, bool, bool))&Perl_Mob_RandomizeFeatures);
	package.add("RangedAttack", &Perl_Mob_RangedAttack);
	package.add("RemoveAllAppearanceEffects", &Perl_Mob_RemoveAllAppearanceEffects);
	package.add("RemoveAllNimbusEffects", &Perl_Mob_RemoveAllNimbusEffects);
	package.add("RemoveFromFeignMemory", &Perl_Mob_RemoveFromFeignMemory);
	package.add("RemoveNimbusEffect", &Perl_Mob_RemoveNimbusEffect);
	package.add("RemovePet", &Perl_Mob_RemovePet);
	package.add("ResistSpell", &Perl_Mob_ResistSpell);
	package.add("RestoreEndurance", &Perl_Mob_RestoreEndurance);
	package.add("RestoreHealth", &Perl_Mob_RestoreHealth);
	package.add("RestoreMana", &Perl_Mob_RestoreMana);
	package.add("ResumeTimer", &Perl_Mob_ResumeTimer);
	package.add("RogueAssassinate", &Perl_Mob_RogueAssassinate);
	package.add("RunTo", &Perl_Mob_RunTo);
	package.add("Say", &Perl_Mob_Say);
	package.add("SeeHide", &Perl_Mob_SeeHide);
	package.add("SeeImprovedHide", &Perl_Mob_SeeImprovedHide);
	package.add("SeeInvisible", &Perl_Mob_SeeInvisible);
	package.add("SeeInvisibleUndead", &Perl_Mob_SeeInvisibleUndead);
	package.add("SendAppearanceEffect", (void(*)(Mob*, int32))&Perl_Mob_SendAppearanceEffect);
	package.add("SendAppearanceEffect", (void(*)(Mob*, int32, int32))&Perl_Mob_SendAppearanceEffect);
	package.add("SendAppearanceEffect", (void(*)(Mob*, int32, int32, int32))&Perl_Mob_SendAppearanceEffect);
	package.add("SendAppearanceEffect", (void(*)(Mob*, int32, int32, int32, int32))&Perl_Mob_SendAppearanceEffect);
	package.add("SendAppearanceEffect", (void(*)(Mob*, int32, int32, int32, int32, int32))&Perl_Mob_SendAppearanceEffect);
	package.add("SendAppearanceEffect", (void(*)(Mob*, int32, int32, int32, int32, int32, perl::nullable<Client*>))&Perl_Mob_SendAppearanceEffect);
	package.add("SendAppearanceEffect", (void(*)(Mob*, int32, int32, int32, int32, int32, perl::nullable<Client*>, uint32))&Perl_Mob_SendAppearanceEffect);
	package.add("SendAppearanceEffect", (void(*)(Mob*, int32, int32, int32, int32, int32, perl::nullable<Client*>, uint32, uint32))&Perl_Mob_SendAppearanceEffect);
	package.add("SendAppearanceEffect", (void(*)(Mob*, int32, int32, int32, int32, int32, perl::nullable<Client*>, uint32, uint32, uint32))&Perl_Mob_SendAppearanceEffect);
	package.add("SendAppearanceEffect", (void(*)(Mob*, int32, int32, int32, int32, int32, perl::nullable<Client*>, uint32, uint32, uint32, uint32))&Perl_Mob_SendAppearanceEffect);
	package.add("SendAppearanceEffect", (void(*)(Mob*, int32, int32, int32, int32, int32, perl::nullable<Client*>, uint32, uint32, uint32, uint32, uint32))&Perl_Mob_SendAppearanceEffect);
	package.add("SendAppearanceEffect", (void(*)(Mob*, int32, int32, int32, int32, int32, perl::nullable<Client*>, uint32, uint32, uint32, uint32, uint32, uint32))&Perl_Mob_SendAppearanceEffect);
	package.add("SendAppearanceEffect", (void(*)(Mob*, int32, int32, int32, int32, int32, perl::nullable<Client*>, uint32, uint32, uint32, uint32, uint32, uint32, uint32))&Perl_Mob_SendAppearanceEffect);
	package.add("SendAppearanceEffect", (void(*)(Mob*, int32, int32, int32, int32, int32, perl::nullable<Client*>, uint32, uint32, uint32, uint32, uint32, uint32, uint32, uint32))&Perl_Mob_SendAppearanceEffect);
	package.add("SendAppearanceEffect", (void(*)(Mob*, int32, int32, int32, int32, int32, perl::nullable<Client*>, uint32, uint32, uint32, uint32, uint32, uint32, uint32, uint32, uint32))&Perl_Mob_SendAppearanceEffect);
	package.add("SendAppearanceEffect", (void(*)(Mob*, int32, int32, int32, int32, int32, perl::nullable<Client*>, uint32, uint32, uint32, uint32, uint32, uint32, uint32, uint32, uint32, uint32))&Perl_Mob_SendAppearanceEffect);
	package.add("SendAppearanceEffectActor", (void(*)(Mob*, int32, uint32))&Perl_Mob_SendAppearanceEffectActor);
	package.add("SendAppearanceEffectActor", (void(*)(Mob*, int32, uint32, int32))&Perl_Mob_SendAppearanceEffectActor);
	package.add("SendAppearanceEffectActor", (void(*)(Mob*, int32, uint32, int32, uint32))&Perl_Mob_SendAppearanceEffectActor);
	package.add("SendAppearanceEffectActor", (void(*)(Mob*, int32, uint32, int32, uint32, int32))&Perl_Mob_SendAppearanceEffectActor);
	package.add("SendAppearanceEffectActor", (void(*)(Mob*, int32, uint32, int32, uint32, int32, uint32))&Perl_Mob_SendAppearanceEffectActor);
	package.add("SendAppearanceEffectActor", (void(*)(Mob*, int32, uint32, int32, uint32, int32, uint32, int32))&Perl_Mob_SendAppearanceEffectActor);
	package.add("SendAppearanceEffectActor", (void(*)(Mob*, int32, uint32, int32, uint32, int32, uint32, int32, uint32))&Perl_Mob_SendAppearanceEffectActor);
	package.add("SendAppearanceEffectActor", (void(*)(Mob*, int32, uint32, int32, uint32, int32, uint32, int32, uint32, int32))&Perl_Mob_SendAppearanceEffectActor);
	package.add("SendAppearanceEffectActor", (void(*)(Mob*, int32, uint32, int32, uint32, int32, uint32, int32, uint32, int32, uint32))&Perl_Mob_SendAppearanceEffectActor);
	package.add("SendAppearanceEffectActor", (void(*)(Mob*, int32, uint32, int32, uint32, int32, uint32, int32, uint32, int32, uint32, Client*))&Perl_Mob_SendAppearanceEffectActor);
	package.add("SendAppearanceEffectGround", (void(*)(Mob*, int32))&Perl_Mob_SendAppearanceEffectGround);
	package.add("SendAppearanceEffectGround", (void(*)(Mob*, int32, int32))&Perl_Mob_SendAppearanceEffectGround);
	package.add("SendAppearanceEffectGround", (void(*)(Mob*, int32, int32, int32))&Perl_Mob_SendAppearanceEffectGround);
	package.add("SendAppearanceEffectGround", (void(*)(Mob*, int32, int32, int32, int32))&Perl_Mob_SendAppearanceEffectGround);
	package.add("SendAppearanceEffectGround", (void(*)(Mob*, int32, int32, int32, int32, int32))&Perl_Mob_SendAppearanceEffectGround);
	package.add("SendAppearanceEffectGround", (void(*)(Mob*, int32, int32, int32, int32, int32, Client*))&Perl_Mob_SendAppearanceEffectGround);
	package.add("SendIllusion", (void(*)(Mob*, uint16))&Perl_Mob_SendIllusion);
	package.add("SendIllusion", (void(*)(Mob*, uint16, uint8))&Perl_Mob_SendIllusion);
	package.add("SendIllusion", (void(*)(Mob*, uint16, uint8, uint8))&Perl_Mob_SendIllusion);
	package.add("SendIllusion", (void(*)(Mob*, uint16, uint8, uint8, uint8))&Perl_Mob_SendIllusion);
	package.add("SendIllusion", (void(*)(Mob*, uint16, uint8, uint8, uint8, uint8))&Perl_Mob_SendIllusion);
	package.add("SendIllusion", (void(*)(Mob*, uint16, uint8, uint8, uint8, uint8, uint8))&Perl_Mob_SendIllusion);
	package.add("SendIllusion", (void(*)(Mob*, uint16, uint8, uint8, uint8, uint8, uint8, uint8))&Perl_Mob_SendIllusion);
	package.add("SendIllusion", (void(*)(Mob*, uint16, uint8, uint8, uint8, uint8, uint8, uint8, uint8))&Perl_Mob_SendIllusion);
	package.add("SendIllusion", (void(*)(Mob*, uint16, uint8, uint8, uint8, uint8, uint8, uint8, uint8, uint8))&Perl_Mob_SendIllusion);
	package.add("SendIllusion", (void(*)(Mob*, uint16, uint8, uint8, uint8, uint8, uint8, uint8, uint8, uint8, uint32))&Perl_Mob_SendIllusion);
	package.add("SendIllusion", (void(*)(Mob*, uint16, uint8, uint8, uint8, uint8, uint8, uint8, uint8, uint8, uint32, uint32))&Perl_Mob_SendIllusion);
	package.add("SendIllusion", (void(*)(Mob*, uint16, uint8, uint8, uint8, uint8, uint8, uint8, uint8, uint8, uint32, uint32, uint32))&Perl_Mob_SendIllusion);
	package.add("SendIllusion", (void(*)(Mob*, uint16, uint8, uint8, uint8, uint8, uint8, uint8, uint8, uint8, uint32, uint32, uint32, float))&Perl_Mob_SendIllusion);
	package.add("SendIllusion", (void(*)(Mob*, uint16, uint8, uint8, uint8, uint8, uint8, uint8, uint8, uint8, uint32, uint32, uint32, float, Client*))&Perl_Mob_SendIllusion);
	package.add("SendIllusionPacket", (void(*)(Mob*, perl::reference))&Perl_Mob_SendIllusionPacket);
	package.add("SendTo", &Perl_Mob_SendTo);
	package.add("SendToFixZ", &Perl_Mob_SendToFixZ);
	package.add("SendWearChange", &Perl_Mob_SendWearChange);
	package.add("SetAA", (bool(*)(Mob*, int, int))&Perl_Mob_SetAA);
	package.add("SetAA", (bool(*)(Mob*, int, int, int))&Perl_Mob_SetAA);
	package.add("SetAllowBeneficial", &Perl_Mob_SetAllowBeneficial);
	package.add("SetAppearance", (void(*)(Mob*, int))&Perl_Mob_SetAppearance);
	package.add("SetAppearance", (void(*)(Mob*, int, bool))&Perl_Mob_SetAppearance);
	package.add("SetBodyType", (void(*)(Mob*, uint8))&Perl_Mob_SetBodyType);
	package.add("SetBodyType", (void(*)(Mob*, uint8, bool))&Perl_Mob_SetBodyType);
	package.add("SetBucket", (void(*)(Mob*, std::string, std::string))&Perl_Mob_SetBucket);
	package.add("SetBucket", (void(*)(Mob*, std::string, std::string, std::string))&Perl_Mob_SetBucket);
	package.add("SetBuffDuration", (void(*)(Mob*, int))&Perl_Mob_SetBuffDuration);
	package.add("SetBuffDuration", (void(*)(Mob*, int, int))&Perl_Mob_SetBuffDuration);
	package.add("SetBuffDuration", (void(*)(Mob*, int, int, int))&Perl_Mob_SetBuffDuration);
	package.add("SetCurrentWP", &Perl_Mob_SetCurrentWP);
	package.add("SetDeltas", &Perl_Mob_SetDeltas);
	package.add("SetDisableMelee", &Perl_Mob_SetDisableMelee);
	package.add("SetEntityVariable", &Perl_Mob_SetEntityVariable);
	package.add("SetExtraHaste", (void(*)(Mob*, int))&Perl_Mob_SetExtraHaste);
	package.add("SetExtraHaste", (void(*)(Mob*, int, bool))&Perl_Mob_SetExtraHaste);
	package.add("SetFlurryChance", &Perl_Mob_SetFlurryChance);
	package.add("SetFlyMode", &Perl_Mob_SetFlyMode);
	package.add("SetFollowID", &Perl_Mob_SetFollowID);
	package.add("SetGender", &Perl_Mob_SetGender);
	package.add("SetGlobal", (void(*)(Mob*, const char*, const char*, int, const char*))&Perl_Mob_SetGlobal);
	package.add("SetGlobal", (void(*)(Mob*, const char*, const char*, int, const char*, Mob*))&Perl_Mob_SetGlobal);
	package.add("SetHP", &Perl_Mob_SetHP);
	package.add("SetHate", (void(*)(Mob*, Mob*))&Perl_Mob_SetHate);
	package.add("SetHate", (void(*)(Mob*, Mob*, int64_t))&Perl_Mob_SetHate);
	package.add("SetHate", (void(*)(Mob*, Mob*, int64_t, int64_t))&Perl_Mob_SetHate);
	package.add("SetHeading", &Perl_Mob_SetHeading);
	package.add("SetInvisible", &Perl_Mob_SetInvisible);
	package.add("SetInvul", &Perl_Mob_SetInvul);
	package.add("SetLD", &Perl_Mob_SetLD);
	package.add("SetLevel", (void(*)(Mob*, uint8_t))&Perl_Mob_SetLevel);
	package.add("SetLevel", (void(*)(Mob*, uint8_t, bool))&Perl_Mob_SetLevel);
	package.add("SetMana", &Perl_Mob_SetMana);
	package.add("SetMaxHP", &Perl_Mob_SetMaxHP);
	package.add("SetOOCRegen", &Perl_Mob_SetOOCRegen);
	package.add("SetOwnerID", &Perl_Mob_SetOwnerID);
	package.add("SetPet", &Perl_Mob_SetPet);
	package.add("SetPetID", &Perl_Mob_SetPetID);
	package.add("SetPetOrder", &Perl_Mob_SetPetOrder);
	package.add("SetPetType", &Perl_Mob_SetPetType);
	package.add("SetRace", &Perl_Mob_SetRace);
	package.add("SetRunAnimSpeed", &Perl_Mob_SetRunAnimSpeed);
	package.add("SetRunning", &Perl_Mob_SetRunning);
	package.add("SetSeeInvisibleLevel", &Perl_Mob_SetSeeInvisibleLevel);
	package.add("SetSeeInvisibleUndeadLevel", &Perl_Mob_SetSeeInvisibleUndeadLevel);
	package.add("SetSlotTint", &Perl_Mob_SetSlotTint);
	package.add("SetSpecialAbility", &Perl_Mob_SetSpecialAbility);
	package.add("SetSpecialAbilityParam", &Perl_Mob_SetSpecialAbilityParam);
	package.add("SetTarget", &Perl_Mob_SetTarget);
	package.add("SetTargetable", &Perl_Mob_SetTargetable);
	package.add("SetTexture", &Perl_Mob_SetTexture);
	package.add("SetTimer", &Perl_Mob_SetTimer);
	package.add("SetTimerMS", &Perl_Mob_SetTimerMS);
	package.add("StopAllTimers", &Perl_Mob_StopAllTimers);
	package.add("StopTimer", &Perl_Mob_StopTimer);
	package.add("ShieldAbility", (void(*)(Mob*, uint32))&Perl_Mob_ShieldAbility);
	package.add("ShieldAbility", (void(*)(Mob*, uint32, int32))&Perl_Mob_ShieldAbility);
	package.add("ShieldAbility", (void(*)(Mob*, uint32, int32, int32))&Perl_Mob_ShieldAbility);
	package.add("ShieldAbility", (void(*)(Mob*, uint32, int32, int32, int32))&Perl_Mob_ShieldAbility);
	package.add("ShieldAbility", (void(*)(Mob*, uint32, int32, int32, int32, int32))&Perl_Mob_ShieldAbility);
	package.add("ShieldAbility", (void(*)(Mob*, uint32, int32, int32, int32, int32, bool))&Perl_Mob_ShieldAbility);
	package.add("ShieldAbility", (void(*)(Mob*, uint32, int32, int32, int32, int32, bool, bool))&Perl_Mob_ShieldAbility);
	package.add("Shout", &Perl_Mob_Shout);
	package.add("SignalClient", &Perl_Mob_SignalClient);
	package.add("SpellEffect", (void(*)(Mob*, uint32))&Perl_Mob_SpellEffect);
	package.add("SpellEffect", (void(*)(Mob*, uint32, uint32))&Perl_Mob_SpellEffect);
	package.add("SpellEffect", (void(*)(Mob*, uint32, uint32, uint32))&Perl_Mob_SpellEffect);
	package.add("SpellEffect", (void(*)(Mob*, uint32, uint32, uint32, bool))&Perl_Mob_SpellEffect);
	package.add("SpellEffect", (void(*)(Mob*, uint32, uint32, uint32, bool, uint32))&Perl_Mob_SpellEffect);
	package.add("SpellEffect", (void(*)(Mob*, uint32, uint32, uint32, bool, uint32, bool))&Perl_Mob_SpellEffect);
	package.add("SpellEffect", (void(*)(Mob*, uint32, uint32, uint32, bool, uint32, bool, Client*))&Perl_Mob_SpellEffect);
	package.add("SpellEffect", (void(*)(Mob*, uint32, uint32, uint32, bool, uint32, bool, perl::nullable<Client*>, uint32))&Perl_Mob_SpellEffect);
	package.add("SpellEffect", (void(*)(Mob*, uint32, uint32, uint32, bool, uint32, bool, perl::nullable<Client*>, uint32, uint32))&Perl_Mob_SpellEffect);
	package.add("SpellFinished", (void(*)(Mob*, uint16))&Perl_Mob_SpellFinished);
	package.add("SpellFinished", (void(*)(Mob*, uint16, Mob*))&Perl_Mob_SpellFinished);
	package.add("SpellFinished", (void(*)(Mob*, uint16, Mob*, int32))&Perl_Mob_SpellFinished);
	package.add("SpellFinished", (void(*)(Mob*, uint16, Mob*, int32, uint16))&Perl_Mob_SpellFinished);
	package.add("Spin", &Perl_Mob_Spin);
	package.add("StartEnrage", &Perl_Mob_StartEnrage);
	package.add("StopNavigation", &Perl_Mob_StopNavigation);
	package.add("Stun", &Perl_Mob_Stun);
	package.add("TarGlobal", &Perl_Mob_TarGlobal);
	package.add("TempName", (void(*)(Mob*))&Perl_Mob_TempName);
	package.add("TempName", (void(*)(Mob*, const char*))&Perl_Mob_TempName);
	package.add("ThrowingAttack", &Perl_Mob_ThrowingAttack);
	package.add("TryMoveAlong", (void(*)(Mob*, float, float))&Perl_Mob_TryMoveAlong);
	package.add("TryMoveAlong", (void(*)(Mob*, float, float, bool))&Perl_Mob_TryMoveAlong);
	package.add("TypesTempPet", (void(*)(Mob*, uint32))&Perl_Mob_TypesTempPet);
	package.add("TypesTempPet", (void(*)(Mob*, uint32, const char*))&Perl_Mob_TypesTempPet);
	package.add("TypesTempPet", (void(*)(Mob*, uint32, const char*, uint32))&Perl_Mob_TypesTempPet);
	package.add("TypesTempPet", (void(*)(Mob*, uint32, const char*, uint32, bool))&Perl_Mob_TypesTempPet);
	package.add("TypesTempPet", (void(*)(Mob*, uint32, const char*, uint32, bool, Mob*))&Perl_Mob_TypesTempPet);
	package.add("TypesTempPet", (void(*)(Mob*, uint32, const char*, uint32, bool, Mob*, bool))&Perl_Mob_TypesTempPet);
	package.add("WalkTo", &Perl_Mob_WalkTo);
	package.add("WearChange", (void(*)(Mob*, uint8, uint32))&Perl_Mob_WearChange);
	package.add("WearChange", (void(*)(Mob*, uint8, uint32, uint32))&Perl_Mob_WearChange);
	package.add("WearChange", (void(*)(Mob*, uint8, uint32, uint32, uint32))&Perl_Mob_WearChange);
	package.add("WipeHateList", &Perl_Mob_WipeHateList);
}

#endif //EMBPERL_XS_CLASSES
