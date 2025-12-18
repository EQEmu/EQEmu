#ifdef LUA_EQEMU

#include "lua_mob.h"

#include "zone/bot.h"
#include "zone/client.h"
#include "zone/dialogue_window.h"
#include "zone/lua_bot.h"
#include "zone/lua_buff.h"
#include "zone/lua_client.h"
#include "zone/lua_hate_list.h"
#include "zone/lua_item.h"
#include "zone/lua_iteminst.h"
#include "zone/lua_npc.h"
#include "zone/lua_stat_bonuses.h"
#include "zone/npc.h"

#include "lua.hpp"
#include "luabind/luabind.hpp"

struct SpecialAbilities { };

struct Lua_Mob_List {
	std::vector<Lua_Mob> entries;
};

const char *Lua_Mob::GetName() {
	Lua_Safe_Call_String();
	return self->GetName();
}

void Lua_Mob::Depop() {
	Lua_Safe_Call_Void();
	return self->Depop();
}

void Lua_Mob::Depop(bool start_spawn_timer) {
	Lua_Safe_Call_Void();
	return self->Depop(start_spawn_timer);
}

bool Lua_Mob::BehindMob() {
	Lua_Safe_Call_Bool();
	return self->BehindMob();
}

bool Lua_Mob::BehindMob(Lua_Mob other) {
	Lua_Safe_Call_Bool();
	return self->BehindMob(other);
}

bool Lua_Mob::BehindMob(Lua_Mob other, float x) {
	Lua_Safe_Call_Bool();
	return self->BehindMob(other, x);
}

bool Lua_Mob::BehindMob(Lua_Mob other, float x, float y) {
	Lua_Safe_Call_Bool();
	return self->BehindMob(other, x, y);
}

void Lua_Mob::SetLevel(int level) {
	Lua_Safe_Call_Void();
	self->SetLevel(level);
}

void Lua_Mob::SetLevel(int level, bool command) {
	Lua_Safe_Call_Void();
	self->SetLevel(level, command);
}

void Lua_Mob::SendWearChange(uint8 material_slot) {
	Lua_Safe_Call_Void();
	self->SendWearChange(material_slot);
}

bool Lua_Mob::IsMoving() {
	Lua_Safe_Call_Bool();
	return self->IsMoving();
}

void Lua_Mob::GotoBind() {
	Lua_Safe_Call_Void();
	self->GoToBind();
}

void Lua_Mob::Gate() {
	Lua_Safe_Call_Void();
	self->Gate();
}

bool Lua_Mob::Attack(Lua_Mob other) {
	Lua_Safe_Call_Bool();
	return self->Attack(other);
}

bool Lua_Mob::Attack(Lua_Mob other, int hand) {
	Lua_Safe_Call_Bool();
	return self->Attack(other, hand);
}

bool Lua_Mob::Attack(Lua_Mob other, int hand, bool from_riposte) {
	Lua_Safe_Call_Bool();
	return self->Attack(other, hand, from_riposte);
}

bool Lua_Mob::Attack(Lua_Mob other, int hand, bool from_riposte, bool is_strikethrough) {
	Lua_Safe_Call_Bool();
	return self->Attack(other, hand, from_riposte, is_strikethrough);
}

bool Lua_Mob::Attack(Lua_Mob other, int hand, bool from_riposte, bool is_strikethrough, bool is_from_spell) {
	Lua_Safe_Call_Bool();
	return self->Attack(other, hand, from_riposte, is_strikethrough, is_from_spell);
}

bool Lua_Mob::Attack(Lua_Mob other, int hand, bool from_riposte, bool is_strikethrough, bool is_from_spell, luabind::adl::object opts) {
	Lua_Safe_Call_Bool();

	ExtraAttackOptions options;
	if(luabind::type(opts) == LUA_TTABLE) {
		auto cur = opts["armor_pen_flat"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				options.armor_pen_flat = luabind::object_cast<int>(cur);
			} catch(luabind::cast_failed &) {
			}
		}

		cur = opts["crit_flat"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				options.crit_flat = luabind::object_cast<float>(cur);
			} catch(luabind::cast_failed &) {
			}
		}

		cur = opts["damage_flat"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				options.damage_flat = luabind::object_cast<int>(cur);
			} catch(luabind::cast_failed &) {
			}
		}

		cur = opts["hate_flat"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				options.hate_flat = luabind::object_cast<int>(cur);
			} catch(luabind::cast_failed &) {
			}
		}

		cur = opts["armor_pen_percent"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				options.armor_pen_percent = luabind::object_cast<float>(cur);
			} catch(luabind::cast_failed &) {
			}
		}

		cur = opts["crit_percent"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				options.crit_percent = luabind::object_cast<float>(cur);
			} catch(luabind::cast_failed &) {
			}
		}

		cur = opts["damage_percent"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				options.damage_percent = luabind::object_cast<float>(cur);
			} catch(luabind::cast_failed &) {
			}
		}

		cur = opts["hate_percent"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				options.hate_percent = luabind::object_cast<float>(cur);
			} catch(luabind::cast_failed &) {
			}
		}
	}

	return self->Attack(other, hand, from_riposte, is_strikethrough, is_from_spell, &options);
}

void Lua_Mob::Damage(Lua_Mob from, int64 damage, int spell_id, int attack_skill) {
	Lua_Safe_Call_Void();
	return self->Damage(from, damage, spell_id, static_cast<EQ::skills::SkillType>(attack_skill));
}

void Lua_Mob::Damage(Lua_Mob from, int64 damage, int spell_id, int attack_skill, bool avoidable) {
	Lua_Safe_Call_Void();
	return self->Damage(from, damage, spell_id, static_cast<EQ::skills::SkillType>(attack_skill), avoidable);
}

void Lua_Mob::Damage(Lua_Mob from, int64 damage, int spell_id, int attack_skill, bool avoidable, int buffslot) {
	Lua_Safe_Call_Void();
	return self->Damage(from, damage, spell_id, static_cast<EQ::skills::SkillType>(attack_skill), avoidable, buffslot);
}

void Lua_Mob::Damage(Lua_Mob from, int64 damage, int spell_id, int attack_skill, bool avoidable, int buffslot, bool buff_tic) {
	Lua_Safe_Call_Void();
	return self->Damage(from, damage, spell_id, static_cast<EQ::skills::SkillType>(attack_skill), avoidable, buffslot, buff_tic);
}

void Lua_Mob::RangedAttack(Lua_Mob other) {
	Lua_Safe_Call_Void();
	self->RangedAttack(other);
}

void Lua_Mob::ThrowingAttack(Lua_Mob other) {
	Lua_Safe_Call_Void();
	self->ThrowingAttack(other);
}

void Lua_Mob::HealDamage(uint64 amount) {
	Lua_Safe_Call_Void();
	self->HealDamage(amount);
}

void Lua_Mob::HealDamage(uint64 amount, Lua_Mob other) {
	Lua_Safe_Call_Void();
	self->HealDamage(amount, other);
}

uint32 Lua_Mob::GetLevelCon(int other) {
	Lua_Safe_Call_Int();
	return self->GetLevelCon(other);
}

uint32 Lua_Mob::GetLevelCon(int my, int other) {
	Lua_Safe_Call_Int();
	return self->GetLevelCon(my, other);
}

void Lua_Mob::SetHP(int64 hp) {
	Lua_Safe_Call_Void();
	self->SetHP(hp);
}

void Lua_Mob::DoAnim(int animation_id) {
	Lua_Safe_Call_Void();
	self->DoAnim(animation_id);
}

void Lua_Mob::DoAnim(int animation_id, int animation_speed) {
	Lua_Safe_Call_Void();
	self->DoAnim(animation_id, animation_speed);
}

void Lua_Mob::DoAnim(int animation_id, int animation_speed, bool ackreq) {
	Lua_Safe_Call_Void();
	self->DoAnim(animation_id, animation_speed, ackreq);
}

void Lua_Mob::DoAnim(int animation_id, int animation_speed, bool ackreq, int filter) {
	Lua_Safe_Call_Void();
	self->DoAnim(animation_id, animation_speed, ackreq, static_cast<eqFilterType>(filter));
}

void Lua_Mob::ChangeSize(double in_size) {
	Lua_Safe_Call_Void();
	self->ChangeSize(static_cast<float>(in_size));
}

void Lua_Mob::ChangeSize(double in_size, bool unrestricted) {
	Lua_Safe_Call_Void();
	self->ChangeSize(static_cast<float>(in_size), unrestricted);
}

void Lua_Mob::GMMove(double x, double y, double z) {
	Lua_Safe_Call_Void();
	self->GMMove(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
}

void Lua_Mob::GMMove(double x, double y, double z, double heading) {
	Lua_Safe_Call_Void();
	self->GMMove(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(heading));
}

void Lua_Mob::GMMove(double x, double y, double z, double heading, bool save_guard_spot) {
	Lua_Safe_Call_Void();
	self->GMMove(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(heading), save_guard_spot);
}

void Lua_Mob::TryMoveAlong(float distance, float angle) {
	Lua_Safe_Call_Void();
	self->TryMoveAlong(distance, angle);
}

void Lua_Mob::TryMoveAlong(float distance, float angle, bool send) {
	Lua_Safe_Call_Void();
	self->TryMoveAlong(distance, angle, send);
}

bool Lua_Mob::HasProcs() {
	Lua_Safe_Call_Bool();
	return self->HasProcs();
}

bool Lua_Mob::IsInvisible() {
	Lua_Safe_Call_Bool();
	return self->IsInvisible();
}

bool Lua_Mob::IsInvisible(Lua_Mob other) {
	Lua_Safe_Call_Bool();
	return self->IsInvisible(other);
}

void Lua_Mob::SetInvisible(int state) {
	Lua_Safe_Call_Void();
	self->SetInvisible(state);
}

bool Lua_Mob::FindBuff(uint16 spell_id) {
	Lua_Safe_Call_Bool();
	return self->FindBuff(spell_id);
}

bool Lua_Mob::FindBuff(uint16 spell_id, uint16 caster_id) {
	Lua_Safe_Call_Bool();
	return self->FindBuff(spell_id, caster_id);
}

uint16 Lua_Mob::FindBuffBySlot(int slot) {
	Lua_Safe_Call_Int();
	return self->FindBuffBySlot(slot);
}

bool Lua_Mob::FindType(int type) {
	Lua_Safe_Call_Bool();
	return self->FindType(type);
}

bool Lua_Mob::FindType(int type, bool offensive) {
	Lua_Safe_Call_Bool();
	return self->FindType(type, offensive);
}

bool Lua_Mob::FindType(int type, bool offensive, int threshold) {
	Lua_Safe_Call_Bool();
	return self->FindType(type, offensive, threshold);
}

int Lua_Mob::GetBuffSlotFromType(int slot) {
	Lua_Safe_Call_Bool();
	return self->GetBuffSlotFromType(slot);
}

int Lua_Mob::GetBaseRace() {
	Lua_Safe_Call_Int();
	return self->GetBaseRace();
}

int Lua_Mob::GetBaseGender() {
	Lua_Safe_Call_Int();
	return self->GetBaseGender();
}

int Lua_Mob::GetDeity() {
	Lua_Safe_Call_Int();
	return self->GetDeity();
}

int Lua_Mob::GetRace() {
	Lua_Safe_Call_Int();
	return self->GetRace();
}

const char *Lua_Mob::GetRaceName() {
	Lua_Safe_Call_String();
	return GetRaceIDName(self->GetRace());
}

const char* Lua_Mob::GetBaseRaceName() {
	Lua_Safe_Call_String();
	return GetRaceIDName(self->GetBaseRace());
}

int Lua_Mob::GetGender() {
	Lua_Safe_Call_Int();
	return self->GetGender();
}

int Lua_Mob::GetTexture() {
	Lua_Safe_Call_Int();
	return self->GetTexture();
}

int Lua_Mob::GetHelmTexture() {
	Lua_Safe_Call_Int();
	return self->GetHelmTexture();
}

int Lua_Mob::GetHairColor() {
	Lua_Safe_Call_Int();
	return self->GetHairColor();
}

int Lua_Mob::GetBeardColor() {
	Lua_Safe_Call_Int();
	return self->GetBeardColor();
}

int Lua_Mob::GetEyeColor1() {
	Lua_Safe_Call_Int();
	return self->GetEyeColor1();
}

int Lua_Mob::GetEyeColor2() {
	Lua_Safe_Call_Int();
	return self->GetEyeColor2();
}

int Lua_Mob::GetHairStyle() {
	Lua_Safe_Call_Int();
	return self->GetHairStyle();
}

int Lua_Mob::GetLuclinFace() {
	Lua_Safe_Call_Int();
	return self->GetLuclinFace();
}

int Lua_Mob::GetBeard() {
	Lua_Safe_Call_Int();
	return self->GetBeard();
}

int Lua_Mob::GetDrakkinHeritage() {
	Lua_Safe_Call_Int();
	return self->GetDrakkinHeritage();
}

int Lua_Mob::GetDrakkinTattoo() {
	Lua_Safe_Call_Int();
	return self->GetDrakkinTattoo();
}

int Lua_Mob::GetDrakkinDetails() {
	Lua_Safe_Call_Int();
	return self->GetDrakkinDetails();
}

int Lua_Mob::GetClass() {
	Lua_Safe_Call_Int();
	return self->GetClass();
}

const char *Lua_Mob::GetClassName() {
	Lua_Safe_Call_String();
	return GetClassIDName(self->GetClass());
}

int Lua_Mob::GetLevel() {
	Lua_Safe_Call_Int();
	return self->GetLevel();
}

const char *Lua_Mob::GetCleanName() {
	Lua_Safe_Call_String();
	return self->GetCleanName();
}

Lua_Mob Lua_Mob::GetTarget() {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetTarget());
}

void Lua_Mob::SetTarget(Lua_Mob t) {
	Lua_Safe_Call_Void();
	self->SetTarget(t);
}

double Lua_Mob::GetHPRatio() {
	Lua_Safe_Call_Real();
	return self->GetHPRatio();
}

bool Lua_Mob::IsWarriorClass() {
	Lua_Safe_Call_Bool();
	return self->IsWarriorClass();
}

int64 Lua_Mob::GetHP() {
	Lua_Safe_Call_Int();
	return self->GetHP();
}

int64 Lua_Mob::GetMaxHP() {
	Lua_Safe_Call_Int();
	return self->GetMaxHP();
}

int64 Lua_Mob::GetItemHPBonuses() {
	Lua_Safe_Call_Int();
	return self->GetItemHPBonuses();
}

int64 Lua_Mob::GetSpellHPBonuses() {
	Lua_Safe_Call_Int();
	return self->GetSpellHPBonuses();
}

double Lua_Mob::GetWalkspeed() {
	Lua_Safe_Call_Real();
	return self->GetWalkspeed();
}

double Lua_Mob::GetRunspeed() {
	Lua_Safe_Call_Real();
	return self->GetRunspeed();
}

int Lua_Mob::GetCasterLevel(int spell_id) {
	Lua_Safe_Call_Int();
	return self->GetCasterLevel(spell_id);
}

int Lua_Mob::GetMaxMana() {
	Lua_Safe_Call_Int();
	return self->GetMaxMana();
}

int Lua_Mob::GetMana() {
	Lua_Safe_Call_Int();
	return self->GetMana();
}

int Lua_Mob::SetMana(int mana) {
	Lua_Safe_Call_Int();
	return self->SetMana(mana);
}

double Lua_Mob::GetManaRatio() {
	Lua_Safe_Call_Real();
	return self->GetManaRatio();
}

int Lua_Mob::GetAC() {
	Lua_Safe_Call_Int();
	return self->GetAC();
}

int Lua_Mob::GetDisplayAC() {
	Lua_Safe_Call_Int();
	return self->GetDisplayAC();
}

int Lua_Mob::GetATK() {
	Lua_Safe_Call_Int();
	return self->GetATK();
}

int Lua_Mob::GetSTR() {
	Lua_Safe_Call_Int();
	return self->GetSTR();
}

int Lua_Mob::GetSTA() {
	Lua_Safe_Call_Int();
	return self->GetSTA();
}

int Lua_Mob::GetDEX() {
	Lua_Safe_Call_Int();
	return self->GetDEX();
}

int Lua_Mob::GetAGI() {
	Lua_Safe_Call_Int();
	return self->GetAGI();
}

int Lua_Mob::GetINT() {
	Lua_Safe_Call_Int();
	return self->GetINT();
}

int Lua_Mob::GetWIS() {
	Lua_Safe_Call_Int();
	return self->GetWIS();
}

int Lua_Mob::GetCHA() {
	Lua_Safe_Call_Int();
	return self->GetCHA();
}

int Lua_Mob::GetMR() {
	Lua_Safe_Call_Int();
	return self->GetMR();
}

int Lua_Mob::GetFR() {
	Lua_Safe_Call_Int();
	return self->GetFR();
}

int Lua_Mob::GetDR() {
	Lua_Safe_Call_Int();
	return self->GetDR();
}

int Lua_Mob::GetPR() {
	Lua_Safe_Call_Int();
	return self->GetPR();
}

int Lua_Mob::GetCR() {
	Lua_Safe_Call_Int();
	return self->GetCR();
}

int Lua_Mob::GetCorruption() {
	Lua_Safe_Call_Int();
	return self->GetCorrup();
}

int Lua_Mob::GetPhR() {
	Lua_Safe_Call_Int();
	return self->GetPhR();
}

int Lua_Mob::GetMaxSTR() {
	Lua_Safe_Call_Int();
	return self->GetMaxSTR();
}

int Lua_Mob::GetMaxSTA() {
	Lua_Safe_Call_Int();
	return self->GetMaxSTA();
}

int Lua_Mob::GetMaxDEX() {
	Lua_Safe_Call_Int();
	return self->GetMaxDEX();
}

int Lua_Mob::GetMaxAGI() {
	Lua_Safe_Call_Int();
	return self->GetMaxAGI();
}

int Lua_Mob::GetMaxINT() {
	Lua_Safe_Call_Int();
	return self->GetMaxINT();
}

int Lua_Mob::GetMaxWIS() {
	Lua_Safe_Call_Int();
	return self->GetMaxWIS();
}

int Lua_Mob::GetMaxCHA() {
	Lua_Safe_Call_Int();
	return self->GetMaxCHA();
}

double Lua_Mob::ResistSpell(int resist_type, int spell_id, Lua_Mob caster) {
	Lua_Safe_Call_Real();
	return self->ResistSpell(resist_type, spell_id, caster);
}

double Lua_Mob::ResistSpell(int resist_type, int spell_id, Lua_Mob caster, bool use_resist_override) {
	Lua_Safe_Call_Real();
	return self->ResistSpell(resist_type, spell_id, caster, use_resist_override);
}

double Lua_Mob::ResistSpell(int resist_type, int spell_id, Lua_Mob caster, bool use_resist_override, int resist_override) {
	Lua_Safe_Call_Real();
	return self->ResistSpell(resist_type, spell_id, caster, use_resist_override, resist_override);
}

double Lua_Mob::ResistSpell(int resist_type, int spell_id, Lua_Mob caster, bool use_resist_override, int resist_override,
	bool charisma_check) {
	Lua_Safe_Call_Real();
	return self->ResistSpell(resist_type, spell_id, caster, use_resist_override, resist_override, charisma_check);
}

int Lua_Mob::GetSpecializeSkillValue(int spell_id) {
	Lua_Safe_Call_Int();
	return self->GetSpecializeSkillValue(spell_id);
}

int Lua_Mob::GetNPCTypeID() {
	Lua_Safe_Call_Int();
	return self->GetNPCTypeID();
}

bool Lua_Mob::IsTargeted() {
	Lua_Safe_Call_Bool();
	return self->IsTargeted();
}

double Lua_Mob::GetX() {
	Lua_Safe_Call_Real();
	return self->GetX();
}

double Lua_Mob::GetY() {
	Lua_Safe_Call_Real();
	return self->GetY();
}

double Lua_Mob::GetZ() {
	Lua_Safe_Call_Real();
	return self->GetZ();
}

double Lua_Mob::GetHeading() {
	Lua_Safe_Call_Real();
	return self->GetHeading();
}

double Lua_Mob::GetWaypointX() {
	Lua_Safe_Call_Real();
	return self->GetCurrentWayPoint().x;
}

double Lua_Mob::GetWaypointY() {
	Lua_Safe_Call_Real();
	return self->GetCurrentWayPoint().y;
}

double Lua_Mob::GetWaypointZ() {
	Lua_Safe_Call_Real();
	return self->GetCurrentWayPoint().z;
}

double Lua_Mob::GetWaypointH() {
	Lua_Safe_Call_Real();
	return self->GetCurrentWayPoint().w;
}

double Lua_Mob::GetWaypointPause() {
	Lua_Safe_Call_Real();
	return self->GetCWPP();
}

int Lua_Mob::GetWaypointID() {
	Lua_Safe_Call_Int();
	return self->GetCWP();
}

void Lua_Mob::SetCurrentWP(int wp) {
	Lua_Safe_Call_Void();
	self->SetCurrentWP(wp);
}

double Lua_Mob::GetSize() {
	Lua_Safe_Call_Real();
	return self->GetSize();
}

void Lua_Mob::Message(uint32 type, const char *message) {
	Lua_Safe_Call_Void();

	// auto inject saylinks
	if (RuleB(Chat, QuestDialogueUsesDialogueWindow) && self->IsClient()) {
		std::string window_markdown = message;
		DialogueWindow::Render(self->CastToClient(), window_markdown);
	}
	else if (RuleB(Chat, AutoInjectSaylinksToClientMessage)) {
		std::string new_message = EQ::SayLinkEngine::InjectSaylinksIfNotExist(message);
		self->Message(type, new_message.c_str());
	}
	else {
		self->Message(type, message);
	}
}

void Lua_Mob::MessageString(uint32 type, uint32 string_id, uint32 distance) {
	Lua_Safe_Call_Void();
	self->MessageString(type, string_id, distance);
}

void Lua_Mob::Say(const char *message) {
	Lua_Safe_Call_Void();
	self->Say(message);
}

void Lua_Mob::Say(const char* message, uint8 language_id) {
	Lua_Safe_Call_Void();
	entity_list.ChannelMessage(self, ChatChannel_Say, language_id, message); // these run through the client channels and probably shouldn't for NPCs, but oh well
}

void Lua_Mob::QuestSay(Lua_Client client, const char *message) {
	Lua_Safe_Call_Void();
	Journal::Options journal_opts;
	journal_opts.speak_mode = Journal::SpeakMode::Say;
	journal_opts.journal_mode = RuleB(NPC, EnableNPCQuestJournal) ? Journal::Mode::Log2 : Journal::Mode::None;
	journal_opts.language = Language::CommonTongue;
	journal_opts.message_type = Chat::NPCQuestSay;
	journal_opts.target_spawn_id = 0;
	self->QuestJournalledSay(client, message, journal_opts);
}

void Lua_Mob::QuestSay(Lua_Client client, const char *message, luabind::adl::object opts) {
	Lua_Safe_Call_Void();

	Journal::Options journal_opts;
	// defaults
	journal_opts.speak_mode = Journal::SpeakMode::Say;
	journal_opts.journal_mode = Journal::Mode::Log2;
	journal_opts.language = Language::CommonTongue;
	journal_opts.message_type = Chat::NPCQuestSay;
	journal_opts.target_spawn_id = 0;

	if (luabind::type(opts) == LUA_TTABLE) {
		auto cur = opts["speak_mode"];
		if (luabind::type(cur) != LUA_TNIL) {
			try {
				journal_opts.speak_mode = static_cast<Journal::SpeakMode>(luabind::object_cast<int>(cur));
			} catch (luabind::cast_failed &) {
			}
		}

		cur = opts["journal_mode"];
		if (luabind::type(cur) != LUA_TNIL) {
			try {
				journal_opts.journal_mode = static_cast<Journal::Mode>(luabind::object_cast<int>(cur));
			} catch (luabind::cast_failed &) {
			}
		}

		cur = opts["language"];
		if (luabind::type(cur) != LUA_TNIL) {
			try {
				journal_opts.language = luabind::object_cast<int>(cur);
			} catch (luabind::cast_failed &) {
			}
		}

		cur = opts["message_type"];
		if (luabind::type(cur) != LUA_TNIL) {
			try {
				journal_opts.message_type = luabind::object_cast<int>(cur);
			} catch (luabind::cast_failed &) {
			}
		}
	}

	// if rule disables it, we override provided
	if (!RuleB(NPC, EnableNPCQuestJournal))
		journal_opts.journal_mode = Journal::Mode::None;

	self->QuestJournalledSay(client, message, journal_opts);
}

void Lua_Mob::Shout(const char *message) {
	Lua_Safe_Call_Void();
	self->Shout(message);
}

void Lua_Mob::Shout(const char* message, uint8 language_id) {
	Lua_Safe_Call_Void();
	entity_list.ChannelMessage(self, ChatChannel_Shout, language_id, message);
}

void Lua_Mob::Emote(const char *message) {
	Lua_Safe_Call_Void();
	self->Emote(message);
}

void Lua_Mob::InterruptSpell() {
	Lua_Safe_Call_Void();
	self->InterruptSpell();
}

void Lua_Mob::InterruptSpell(int spell_id) {
	Lua_Safe_Call_Void();
	self->InterruptSpell(spell_id);
}

bool Lua_Mob::CastSpell(int spell_id, int target_id) {
	Lua_Safe_Call_Bool();
	return self->CastSpell(spell_id, target_id);
}

bool Lua_Mob::CastSpell(int spell_id, int target_id, int slot) {
	Lua_Safe_Call_Bool();

	int cast_slot = 0;
	if (slot >= 0 || slot <= 23 || slot == 255) {
		cast_slot = slot;
	}

	return self->CastSpell(spell_id, target_id, static_cast<EQ::spells::CastingSlot>(cast_slot));
}

bool Lua_Mob::CastSpell(int spell_id, int target_id, int slot, int cast_time) {
	Lua_Safe_Call_Bool();
	return self->CastSpell(spell_id, target_id, static_cast<EQ::spells::CastingSlot>(slot), cast_time);
}

bool Lua_Mob::CastSpell(int spell_id, int target_id, int slot, int cast_time, int mana_cost) {
	Lua_Safe_Call_Bool();
	return self->CastSpell(spell_id, target_id, static_cast<EQ::spells::CastingSlot>(slot), cast_time, mana_cost);
}

bool Lua_Mob::CastSpell(int spell_id, int target_id, int slot, int cast_time, int mana_cost, int item_slot) {
	Lua_Safe_Call_Bool();
	return self->CastSpell(spell_id, target_id, static_cast<EQ::spells::CastingSlot>(slot), cast_time, mana_cost, nullptr, static_cast<uint32>(item_slot));
}

bool Lua_Mob::CastSpell(int spell_id, int target_id, int slot, int cast_time, int mana_cost, int item_slot, int timer,
	int timer_duration) {
	Lua_Safe_Call_Bool();
	return self->CastSpell(spell_id, target_id, static_cast<EQ::spells::CastingSlot>(slot), cast_time, mana_cost, nullptr, static_cast<uint32>(item_slot),
		static_cast<uint32>(timer), static_cast<uint32>(timer_duration));
}

bool Lua_Mob::CastSpell(int spell_id, int target_id, int slot, int cast_time, int mana_cost, int item_slot, int timer,
	int timer_duration, int resist_adjust) {
	Lua_Safe_Call_Bool();
	int16 res = resist_adjust;

	return self->CastSpell(spell_id, target_id, static_cast<EQ::spells::CastingSlot>(slot), cast_time, mana_cost, nullptr, static_cast<uint32>(item_slot),
		static_cast<uint32>(timer), static_cast<uint32>(timer_duration), &res);
}

bool Lua_Mob::SpellFinished(int spell_id, Lua_Mob target) {
	Lua_Safe_Call_Bool();
	return self->SpellFinished(spell_id, target);
}

bool Lua_Mob::SpellFinished(int spell_id, Lua_Mob target, int slot) {
	Lua_Safe_Call_Bool();
	return self->SpellFinished(spell_id, target, static_cast<EQ::spells::CastingSlot>(slot));
}

bool Lua_Mob::SpellFinished(int spell_id, Lua_Mob target, int slot, int mana_used) {
	Lua_Safe_Call_Bool();
	return self->SpellFinished(spell_id, target, static_cast<EQ::spells::CastingSlot>(slot), mana_used);
}

bool Lua_Mob::SpellFinished(int spell_id, Lua_Mob target, int slot, int mana_used, uint32 inventory_slot) {
	Lua_Safe_Call_Bool();
	return self->SpellFinished(spell_id, target, static_cast<EQ::spells::CastingSlot>(slot), mana_used, inventory_slot);
}

bool Lua_Mob::SpellFinished(int spell_id, Lua_Mob target, int slot, int mana_used, uint32 inventory_slot, int resist_adjust) {
	Lua_Safe_Call_Bool();
	return self->SpellFinished(spell_id, target, static_cast<EQ::spells::CastingSlot>(slot), mana_used, inventory_slot, resist_adjust);
}

bool Lua_Mob::SpellFinished(int spell_id, Lua_Mob target, int slot, int mana_used, uint32 inventory_slot, int resist_adjust, bool proc) {
	Lua_Safe_Call_Bool();
	return self->SpellFinished(spell_id, target, static_cast<EQ::spells::CastingSlot>(slot), mana_used, inventory_slot, resist_adjust, proc);
}

void Lua_Mob::SendBeginCast(int spell_id, int cast_time) {
	Lua_Safe_Call_Void();
	self->SendBeginCast(spell_id, cast_time);
}

void Lua_Mob::SpellEffect(Lua_Mob caster, int spell_id, double partial) {
	Lua_Safe_Call_Void();
	self->SpellEffect(caster, spell_id, static_cast<float>(partial));
}

Lua_Mob Lua_Mob::GetPet() {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetPet());
}

Lua_Mob Lua_Mob::GetOwner() {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetOwner());
}

Lua_HateList Lua_Mob::GetHateList() {
	Lua_Safe_Call_Class(Lua_HateList);
	Lua_HateList ret;

	auto h_list = self->GetHateList();
	auto iter = h_list.begin();
	while(iter != h_list.end()) {
		Lua_HateEntry e(*iter);
		ret.entries.push_back(e);
		++iter;
	}

	return ret;
}

Lua_HateList Lua_Mob::GetShuffledHateList() {
	Lua_Safe_Call_Class(Lua_HateList);
	Lua_HateList ret;

	auto h_list = self->GetHateList();
	auto iter = h_list.begin();
	while(iter != h_list.end()) {
		Lua_HateEntry e(*iter);
		ret.entries.push_back(e);
		++iter;
	}

	zone->random.Shuffle(ret.entries.begin(), ret.entries.end());

	return ret;
}

Lua_Mob Lua_Mob::GetHateTop() {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetHateTop());
}

Lua_Bot Lua_Mob::GetHateTopBot() {
	Lua_Safe_Call_Class(Lua_Bot);
	return Lua_Bot(self->GetHateTopBot());
}

Lua_Client Lua_Mob::GetHateTopClient() {
	Lua_Safe_Call_Class(Lua_Client);
	return Lua_Client(self->GetHateTopClient());
}

Lua_NPC Lua_Mob::GetHateTopNPC() {
	Lua_Safe_Call_Class(Lua_NPC);
	return Lua_NPC(self->GetHateTopNPC());
}

Lua_Mob Lua_Mob::GetHateDamageTop(Lua_Mob other) {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetHateDamageTop(other));
}

Lua_Mob Lua_Mob::GetHateRandom() {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetHateRandom());
}

void Lua_Mob::AddToHateList(Lua_Mob other) {
	Lua_Safe_Call_Void();
	self->AddToHateList(other);
}

void Lua_Mob::AddToHateList(Lua_Mob other, int64 hate) {
	Lua_Safe_Call_Void();
	self->AddToHateList(other, hate);
}

void Lua_Mob::AddToHateList(Lua_Mob other, int64 hate, int64 damage) {
	Lua_Safe_Call_Void();
	self->AddToHateList(other, hate, damage);
}

void Lua_Mob::AddToHateList(Lua_Mob other, int64 hate, int64 damage, bool yell_for_help) {
	Lua_Safe_Call_Void();
	self->AddToHateList(other, hate, damage, yell_for_help);
}

void Lua_Mob::AddToHateList(Lua_Mob other, int64 hate, int64 damage, bool yell_for_help, bool frenzy) {
	Lua_Safe_Call_Void();
	self->AddToHateList(other, hate, damage, yell_for_help, frenzy);
}

void Lua_Mob::AddToHateList(Lua_Mob other, int64 hate, int64 damage, bool yell_for_help, bool frenzy, bool buff_tic) {
	Lua_Safe_Call_Void();
	self->AddToHateList(other, hate, damage, yell_for_help, frenzy, buff_tic);
}

void Lua_Mob::SetHate(Lua_Mob other) {
	Lua_Safe_Call_Void();
	self->SetHateAmountOnEnt(other);
}

void Lua_Mob::SetHate(Lua_Mob other, int64 hate) {
	Lua_Safe_Call_Void();
	self->SetHateAmountOnEnt(other, hate);
}

void Lua_Mob::SetHate(Lua_Mob other, int64 hate, int64 damage) {
	Lua_Safe_Call_Void();
	self->SetHateAmountOnEnt(other, hate, damage);
}

void Lua_Mob::HalveAggro(Lua_Mob other) {
	Lua_Safe_Call_Void();
	self->HalveAggro(other);
}

void Lua_Mob::DoubleAggro(Lua_Mob other) {
	Lua_Safe_Call_Void();
	self->DoubleAggro(other);
}

int64 Lua_Mob::GetHateAmount(Lua_Mob target) {
	Lua_Safe_Call_Int();
	return self->GetHateAmount(target);
}

int64 Lua_Mob::GetHateAmount(Lua_Mob target, bool is_damage) {
	Lua_Safe_Call_Int();
	return self->GetHateAmount(target, is_damage);
}

uint64 Lua_Mob::GetDamageAmount(Lua_Mob target) {
	Lua_Safe_Call_Int();
	return self->GetDamageAmount(target);
}

void Lua_Mob::WipeHateList() {
	Lua_Safe_Call_Void();
	self->WipeHateList();
}

bool Lua_Mob::CheckAggro(Lua_Mob other) {
	Lua_Safe_Call_Bool();
	return self->CheckAggro(other);
}

void Lua_Mob::Stun(int duration) {
	Lua_Safe_Call_Void();
	self->Stun(duration);
}

void Lua_Mob::UnStun() {
	Lua_Safe_Call_Void();
	self->UnStun();
}

bool Lua_Mob::IsStunned() {
	Lua_Safe_Call_Bool();
	return self->IsStunned();
}

void Lua_Mob::Spin() {
	Lua_Safe_Call_Void();
	self->Spin();
}

void Lua_Mob::Kill() {
	Lua_Safe_Call_Void();
	self->Kill();
}

bool Lua_Mob::CanThisClassDoubleAttack() {
	Lua_Safe_Call_Bool();
	return self->CanThisClassDoubleAttack();
}

bool Lua_Mob::CanThisClassDualWield() {
	Lua_Safe_Call_Bool();
	return self->CanThisClassDualWield();
}

bool Lua_Mob::CanThisClassRiposte() {
	Lua_Safe_Call_Bool();
	return self->CanThisClassRiposte();
}

bool Lua_Mob::CanThisClassDodge() {
	Lua_Safe_Call_Bool();
	return self->CanThisClassDodge();
}

bool Lua_Mob::CanThisClassParry() {
	Lua_Safe_Call_Bool();
	return self->CanThisClassParry();
}

bool Lua_Mob::CanThisClassBlock() {
	Lua_Safe_Call_Bool();
	return self->CanThisClassBlock();
}

void Lua_Mob::SetInvul(bool value) {
	Lua_Safe_Call_Void();
	self->SetInvul(value);
}

bool Lua_Mob::GetInvul() {
	Lua_Safe_Call_Bool();
	return self->GetInvul();
}

void Lua_Mob::SetExtraHaste(int haste) {
	Lua_Safe_Call_Void();
	self->SetExtraHaste(haste);
}

void Lua_Mob::SetExtraHaste(int haste, bool need_to_save) {
	Lua_Safe_Call_Void();
	self->SetExtraHaste(haste, need_to_save);
}

int Lua_Mob::GetHaste() {
	Lua_Safe_Call_Int();
	return self->GetHaste();
}

int Lua_Mob::GetHandToHandDamage() {
	Lua_Safe_Call_Int();
	return self->GetHandToHandDamage();
}

int Lua_Mob::GetHandToHandDelay() {
	Lua_Safe_Call_Int();
	return self->GetHandToHandDelay();
}

void Lua_Mob::Mesmerize() {
	Lua_Safe_Call_Void();
	self->Mesmerize();
}

bool Lua_Mob::IsMezzed() {
	Lua_Safe_Call_Bool();
	return self->IsMezzed();
}

bool Lua_Mob::IsEnraged() {
	Lua_Safe_Call_Bool();
	return self->IsEnraged();
}

int Lua_Mob::GetReverseFactionCon(Lua_Mob other) {
	Lua_Safe_Call_Int();
	return self->GetReverseFactionCon(other);
}

bool Lua_Mob::IsAIControlled() {
	Lua_Safe_Call_Bool();
	return self->IsAIControlled();
}

float Lua_Mob::GetAggroRange() {
	Lua_Safe_Call_Real();
	return self->GetAggroRange();
}

float Lua_Mob::GetAssistRange() {
	Lua_Safe_Call_Real();
	return self->GetAssistRange();
}

void Lua_Mob::SetPetOrder(uint8 pet_order) {
	Lua_Safe_Call_Void();
	self->SetPetOrder(pet_order);
}

uint8 Lua_Mob::GetPetOrder() {
	Lua_Safe_Call_Int();
	return self->GetPetOrder();
}

bool Lua_Mob::IsRoamer() {
	Lua_Safe_Call_Bool();
	return self->IsRoamer();
}

bool Lua_Mob::IsRooted() {
	Lua_Safe_Call_Bool();
	return self->IsRooted();
}

bool Lua_Mob::IsEngaged() {
	Lua_Safe_Call_Bool();
	return self->IsEngaged();
}

void Lua_Mob::FaceTarget(Lua_Mob target) {
	Lua_Safe_Call_Void();
	self->FaceTarget(target);
}

void Lua_Mob::SetHeading(double in) {
	Lua_Safe_Call_Void();
	self->SetHeading(static_cast<float>(in));
}

double Lua_Mob::CalculateHeadingToTarget(double in_x, double in_y) {
	Lua_Safe_Call_Real();
	return self->CalculateHeadingToTarget(static_cast<float>(in_x), static_cast<float>(in_y));
}

void Lua_Mob::RunTo(double x, double y, double z) {
	Lua_Safe_Call_Void();
	self->RunTo(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
}

void Lua_Mob::WalkTo(double x, double y, double z) {
	Lua_Safe_Call_Void();
	self->WalkTo(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
}

void Lua_Mob::NavigateTo(double x, double y, double z) {
	Lua_Safe_Call_Void();
	self->NavigateTo(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
}

void Lua_Mob::StopNavigation() {
	Lua_Safe_Call_Void();
	self->StopNavigation();
}

float Lua_Mob::CalculateDistance(double x, double y, double z) {
	Lua_Safe_Call_Real();
	return self->CalculateDistance(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
}

float Lua_Mob::CalculateDistance(Lua_Mob mob) {
	Lua_Safe_Call_Real();
	return self->CalculateDistance(mob);
}

void Lua_Mob::SendTo(double x, double y, double z) {
	Lua_Safe_Call_Void();
	self->SendTo(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
}

void Lua_Mob::SendToFixZ(double x, double y, double z) {
	Lua_Safe_Call_Void();
	self->SendToFixZ(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
}

void Lua_Mob::NPCSpecialAttacks(const char *parse, int perm) {
	Lua_Safe_Call_Void();
	self->NPCSpecialAttacks(parse, perm);
}

void Lua_Mob::NPCSpecialAttacks(const char *parse, int perm, bool reset) {
	Lua_Safe_Call_Void();
	self->NPCSpecialAttacks(parse, perm, reset);
}

void Lua_Mob::NPCSpecialAttacks(const char *parse, int perm, bool reset, bool remove) {
	Lua_Safe_Call_Void();
	self->NPCSpecialAttacks(parse, perm, reset, remove);
}

int Lua_Mob::GetResist(int type) {
	Lua_Safe_Call_Int();
	return self->GetResist(type);
}

bool Lua_Mob::Charmed() {
	Lua_Safe_Call_Bool();
	return self->Charmed();
}

int Lua_Mob::CheckAggroAmount(int spell_id) {
	Lua_Safe_Call_Int();
	return self->CheckAggroAmount(spell_id, nullptr);
}

int Lua_Mob::CheckAggroAmount(int spell_id, bool is_proc) {
	Lua_Safe_Call_Int();
	return self->CheckAggroAmount(spell_id, nullptr, is_proc);
}

int Lua_Mob::CheckHealAggroAmount(int spell_id) {
	Lua_Safe_Call_Int();
	return self->CheckHealAggroAmount(spell_id, nullptr);
}

int Lua_Mob::CheckHealAggroAmount(int spell_id, uint32 heal_possible) {
	Lua_Safe_Call_Int();
	return self->CheckHealAggroAmount(spell_id, nullptr, heal_possible);
}

int Lua_Mob::GetAA(int id) {
	Lua_Safe_Call_Int();
	return self->GetAA(id);
}

int Lua_Mob::GetAAByAAID(int id) {
	Lua_Safe_Call_Int();
	return self->GetAAByAAID(id);
}

bool Lua_Mob::SetAA(int rank_id, int new_value) {
	Lua_Safe_Call_Bool();
	return self->SetAA(rank_id, new_value);
}

bool Lua_Mob::SetAA(int rank_id, int new_value, int charges) {
	Lua_Safe_Call_Bool();
	return self->SetAA(rank_id, new_value, charges);
}

bool Lua_Mob::DivineAura() {
	Lua_Safe_Call_Bool();
	return self->DivineAura();
}

void Lua_Mob::SetOOCRegen(int64 new_ooc_regen) {
	Lua_Safe_Call_Void();
	self->SetOOCRegen(new_ooc_regen);
}

void Lua_Mob::Signal(int signal_id) {
	Lua_Safe_Call_Void();

	if (self->IsClient()) {
		self->CastToClient()->Signal(signal_id);
	} else if (self->IsNPC()) {
		self->CastToNPC()->SignalNPC(signal_id);
	} else if (self->IsBot()) {
		self->CastToBot()->Signal(signal_id);
	}
}

bool Lua_Mob::CombatRange(Lua_Mob other) {
	Lua_Safe_Call_Bool();
	return self->CombatRange(other);
}

void Lua_Mob::DoSpecialAttackDamage(Lua_Mob other, int skill, int max_damage) {
	Lua_Safe_Call_Void();
	self->DoSpecialAttackDamage(other, static_cast<EQ::skills::SkillType>(skill), max_damage);
}

void Lua_Mob::DoSpecialAttackDamage(Lua_Mob other, int skill, int max_damage, int min_damage) {
	Lua_Safe_Call_Void();
	self->DoSpecialAttackDamage(other, static_cast<EQ::skills::SkillType>(skill), max_damage, min_damage);
}

void Lua_Mob::DoSpecialAttackDamage(Lua_Mob other, int skill, int max_damage, int min_damage, int hate_override) {
	Lua_Safe_Call_Void();
	self->DoSpecialAttackDamage(other, static_cast<EQ::skills::SkillType>(skill), max_damage, min_damage, hate_override);
}

void Lua_Mob::DoSpecialAttackDamage(Lua_Mob other, int skill, int max_damage, int min_damage, int hate_override, int reuse_time) {
	Lua_Safe_Call_Void();
	self->DoSpecialAttackDamage(other, static_cast<EQ::skills::SkillType>(skill), max_damage, min_damage, hate_override, reuse_time);
}

void Lua_Mob::DoThrowingAttackDmg(Lua_Mob other) {
	Lua_Safe_Call_Void();
	self->DoThrowingAttackDmg(other);
}

void Lua_Mob::DoThrowingAttackDmg(Lua_Mob other, Lua_ItemInst range_weapon) {
	Lua_Safe_Call_Void();
	self->DoThrowingAttackDmg(other, range_weapon);
}

void Lua_Mob::DoThrowingAttackDmg(Lua_Mob other, Lua_ItemInst range_weapon, Lua_Item item) {
	Lua_Safe_Call_Void();
	self->DoThrowingAttackDmg(other, range_weapon, item);
}

void Lua_Mob::DoThrowingAttackDmg(Lua_Mob other, Lua_ItemInst range_weapon, Lua_Item item, int weapon_damage) {
	Lua_Safe_Call_Void();
	self->DoThrowingAttackDmg(other, range_weapon, item, weapon_damage);
}

void Lua_Mob::DoThrowingAttackDmg(Lua_Mob other, Lua_ItemInst range_weapon, Lua_Item item, int weapon_damage, int chance_mod) {
	Lua_Safe_Call_Void();
	self->DoThrowingAttackDmg(other, range_weapon, item, weapon_damage, chance_mod);
}

void Lua_Mob::DoThrowingAttackDmg(Lua_Mob other, Lua_ItemInst range_weapon, Lua_Item item, int weapon_damage, int chance_mod,
								  int focus) {
	Lua_Safe_Call_Void();
	self->DoThrowingAttackDmg(other, range_weapon, item, weapon_damage, chance_mod, focus);
}

void Lua_Mob::DoMeleeSkillAttackDmg(Lua_Mob other, int weapon_damage, int skill) {
	Lua_Safe_Call_Void();
	self->DoMeleeSkillAttackDmg(other, weapon_damage, static_cast<EQ::skills::SkillType>(skill));
}

void Lua_Mob::DoMeleeSkillAttackDmg(Lua_Mob other, int weapon_damage, int skill, int chance_mod) {
	Lua_Safe_Call_Void();
	self->DoMeleeSkillAttackDmg(other, weapon_damage, static_cast<EQ::skills::SkillType>(skill), chance_mod);
}

void Lua_Mob::DoMeleeSkillAttackDmg(Lua_Mob other, int weapon_damage, int skill, int chance_mod, int focus) {
	Lua_Safe_Call_Void();
	self->DoMeleeSkillAttackDmg(other, weapon_damage, static_cast<EQ::skills::SkillType>(skill), chance_mod, focus);
}

void Lua_Mob::DoMeleeSkillAttackDmg(Lua_Mob other, int weapon_damage, int skill, int chance_mod, int focus, bool can_riposte) {
	Lua_Safe_Call_Void();
	self->DoMeleeSkillAttackDmg(other, weapon_damage, static_cast<EQ::skills::SkillType>(skill), chance_mod, focus, can_riposte);
}

void Lua_Mob::DoArcheryAttackDmg(Lua_Mob other) {
	Lua_Safe_Call_Void();
	self->DoArcheryAttackDmg(other);
}

void Lua_Mob::DoArcheryAttackDmg(Lua_Mob other, Lua_ItemInst range_weapon) {
	Lua_Safe_Call_Void();
	self->DoArcheryAttackDmg(other, range_weapon);
}

void Lua_Mob::DoArcheryAttackDmg(Lua_Mob other, Lua_ItemInst range_weapon, Lua_ItemInst ammo) {
	Lua_Safe_Call_Void();
	self->DoArcheryAttackDmg(other, range_weapon, ammo);
}

void Lua_Mob::DoArcheryAttackDmg(Lua_Mob other, Lua_ItemInst range_weapon, Lua_ItemInst ammo, int weapon_damage) {
	Lua_Safe_Call_Void();
	self->DoArcheryAttackDmg(other, range_weapon, ammo, weapon_damage);
}

void Lua_Mob::DoArcheryAttackDmg(Lua_Mob other, Lua_ItemInst range_weapon, Lua_ItemInst ammo, int weapon_damage, int chance_mod) {
	Lua_Safe_Call_Void();
	self->DoArcheryAttackDmg(other, range_weapon, ammo, weapon_damage, chance_mod);
}

void Lua_Mob::DoArcheryAttackDmg(Lua_Mob other, Lua_ItemInst range_weapon, Lua_ItemInst ammo, int weapon_damage, int chance_mod,
								 int focus) {
	Lua_Safe_Call_Void();
	self->DoArcheryAttackDmg(other, range_weapon, ammo, weapon_damage, chance_mod, focus);
}

bool Lua_Mob::CheckLoS(Lua_Mob other) {
	Lua_Safe_Call_Bool();
	return self->CheckLosFN(other);
}

bool Lua_Mob::CheckLoSToLoc(double x, double y, double z) {
	Lua_Safe_Call_Bool();
	return self->CheckLosFN(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), 6.0f);
}

bool Lua_Mob::CheckLoSToLoc(double x, double y, double z, double mob_size) {
	Lua_Safe_Call_Bool();
	return self->CheckLosFN(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(mob_size));
}

double Lua_Mob::FindGroundZ(double x, double y) {
	Lua_Safe_Call_Real();
	return self->GetGroundZ(static_cast<float>(x), static_cast<float>(y));
}

double Lua_Mob::FindGroundZ(double x, double y, double z) {
	Lua_Safe_Call_Real();
	return self->GetGroundZ(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
}

void Lua_Mob::ProjectileAnimation(Lua_Mob to, int item_id) {
	Lua_Safe_Call_Void();
	self->ProjectileAnimation(to, item_id);
}

void Lua_Mob::ProjectileAnimation(Lua_Mob to, int item_id, bool is_arrow) {
	Lua_Safe_Call_Void();
	self->ProjectileAnimation(to, item_id, is_arrow);
}

void Lua_Mob::ProjectileAnimation(Lua_Mob to, int item_id, bool is_arrow, double speed) {
	Lua_Safe_Call_Void();
	self->ProjectileAnimation(to, item_id, is_arrow, static_cast<float>(speed));
}

void Lua_Mob::ProjectileAnimation(Lua_Mob to, int item_id, bool is_arrow, double speed, double angle) {
	Lua_Safe_Call_Void();
	self->ProjectileAnimation(to, item_id, is_arrow, static_cast<float>(speed), static_cast<float>(angle));
}

void Lua_Mob::ProjectileAnimation(Lua_Mob to, int item_id, bool is_arrow, double speed, double angle, double tilt) {
	Lua_Safe_Call_Void();
	self->ProjectileAnimation(to, item_id, is_arrow, static_cast<float>(speed), static_cast<float>(angle), static_cast<float>(tilt));
}

void Lua_Mob::ProjectileAnimation(Lua_Mob to, int item_id, bool is_arrow, double speed, double angle, double tilt, double arc) {
	Lua_Safe_Call_Void();
	self->ProjectileAnimation(to, item_id, is_arrow, static_cast<float>(speed), static_cast<float>(angle), static_cast<float>(tilt),
		static_cast<float>(arc));
}

bool Lua_Mob::HasNPCSpecialAtk(const char *parse) {
	Lua_Safe_Call_Bool();
	return self->HasNPCSpecialAtk(parse);
}

void Lua_Mob::SendAppearanceEffect(uint32 parm1, uint32 parm2, uint32 parm3, uint32 parm4, uint32 parm5) {
	Lua_Safe_Call_Void();
	self->SendAppearanceEffect(parm1, parm2, parm3, parm4, parm5);
}

void Lua_Mob::SendAppearanceEffect(uint32 parm1, uint32 parm2, uint32 parm3, uint32 parm4, uint32 parm5, Lua_Client specific_target) {
	Lua_Safe_Call_Void();
	self->SendAppearanceEffect(parm1, parm2, parm3, parm4, parm5, specific_target);
}

void Lua_Mob::SetFlyMode(int in) {
	Lua_Safe_Call_Void();
	self->SetFlyMode(static_cast<GravityBehavior>(in));
}

void Lua_Mob::SetTexture(uint8 texture) {
	Lua_Safe_Call_Void();
	self->SendIllusionPacket(
		AppearanceStruct{
			.race_id = self->GetRace(),
			.texture = texture
		}
	);
}

void Lua_Mob::SetRace(uint16 race_id) {
	Lua_Safe_Call_Void();
	self->SendIllusionPacket(
		AppearanceStruct{
			.race_id = race_id
		}
	);
}

void Lua_Mob::SetGender(uint8 gender_id) {
	Lua_Safe_Call_Void();
	self->SendIllusionPacket(
		AppearanceStruct{
			.gender_id = gender_id,
			.race_id = self->GetRace(),
		}
	);
}

void Lua_Mob::SendIllusionPacket(luabind::adl::object illusion) {
	Lua_Safe_Call_Void();

	if(luabind::type(illusion) != LUA_TTABLE) {
		return;
	}

	uint16     race                    = self->GetRace();
	uint8      gender                  = self->GetGender();
	uint8      texture                 = self->GetTexture();
	uint8      helmtexture             = self->GetHelmTexture();
	uint8      haircolor               = self->GetHairColor();
	uint8      beardcolor              = self->GetBeardColor();
	uint8      eyecolor1               = self->GetEyeColor1();
	uint8      eyecolor2               = self->GetEyeColor2();
	uint8      hairstyle               = self->GetHairStyle();
	uint8      luclinface              = self->GetLuclinFace();
	uint8      beard                   = self->GetBeard();
	uint8      aa_title                = 255;
	uint32     drakkin_heritage        = self->GetDrakkinHeritage();
	uint32     drakkin_tattoo          = self->GetDrakkinTattoo();
	uint32     drakkin_details         = self->GetDrakkinDetails();
	float      size                    = self->GetSize();
	bool       send_appearance_effects = true;
	Lua_Client target                  = Lua_Client();

	auto cur = illusion["race"];
	if (luabind::type(cur) != LUA_TNIL) {
		try {
			race = luabind::object_cast<uint16>(cur);
		} catch (luabind::cast_failed &) {
		}
	}

	cur = illusion["gender"];
	if (luabind::type(cur) != LUA_TNIL) {
		try {
			gender = luabind::object_cast<uint8>(cur);
		} catch (luabind::cast_failed &) {
		}
	}

	cur = illusion["texture"];
	if (luabind::type(cur) != LUA_TNIL) {
		try {
			texture = luabind::object_cast<uint8>(cur);
		} catch (luabind::cast_failed &) {
		}
	}

	cur = illusion["helmtexture"];
	if (luabind::type(cur) != LUA_TNIL) {
		try {
			helmtexture = luabind::object_cast<uint8>(cur);
		} catch (luabind::cast_failed &) {
		}
	}

	cur = illusion["haircolor"];
	if (luabind::type(cur) != LUA_TNIL) {
		try {
			haircolor = luabind::object_cast<uint8>(cur);
		} catch (luabind::cast_failed &) {
		}
	}

	cur = illusion["beardcolor"];
	if (luabind::type(cur) != LUA_TNIL) {
		try {
			beardcolor = luabind::object_cast<uint8>(cur);
		} catch (luabind::cast_failed &) {
		}
	}

	cur = illusion["eyecolor1"];
	if (luabind::type(cur) != LUA_TNIL) {
		try {
			eyecolor1 = luabind::object_cast<uint8>(cur);
		} catch (luabind::cast_failed &) {
		}
	}

	cur = illusion["eyecolor2"];
	if (luabind::type(cur) != LUA_TNIL) {
		try {
			eyecolor2 = luabind::object_cast<uint8>(cur);
		} catch (luabind::cast_failed &) {
		}
	}

	cur = illusion["hairstyle"];
	if (luabind::type(cur) != LUA_TNIL) {
		try {
			hairstyle = luabind::object_cast<uint8>(cur);
		} catch (luabind::cast_failed &) {
		}
	}

	cur = illusion["luclinface"];
	if (luabind::type(cur) != LUA_TNIL) {
		try {
			luclinface = luabind::object_cast<uint8>(cur);
		} catch (luabind::cast_failed &) {
		}
	}

	cur = illusion["beard"];
	if (luabind::type(cur) != LUA_TNIL) {
		try {
			beard = luabind::object_cast<uint8>(cur);
		} catch (luabind::cast_failed &) {
		}
	}

	cur = illusion["aa_title"];
	if (luabind::type(cur) != LUA_TNIL) {
		try {
			aa_title = luabind::object_cast<uint8>(cur);
		} catch (luabind::cast_failed &) {
		}
	}

	cur = illusion["drakkin_heritage"];
	if (luabind::type(cur) != LUA_TNIL) {
		try {
			drakkin_heritage = luabind::object_cast<uint32>(cur);
		} catch (luabind::cast_failed &) {
		}
	}

	cur = illusion["drakkin_tattoo"];
	if (luabind::type(cur) != LUA_TNIL) {
		try {
			drakkin_tattoo = luabind::object_cast<uint32>(cur);
		} catch (luabind::cast_failed &) {
		}
	}

	cur = illusion["drakkin_details"];
	if (luabind::type(cur) != LUA_TNIL) {
		try {
			drakkin_details = luabind::object_cast<uint32>(cur);
		} catch (luabind::cast_failed &) {
		}
	}

	cur = illusion["size"];
	if (luabind::type(cur) != LUA_TNIL) {
		try {
			size = luabind::object_cast<float>(cur);
		} catch (luabind::cast_failed &) {
		}
	}

	cur = illusion["send_appearance_effects"];
	if (luabind::type(cur) != LUA_TNIL) {
		try {
			send_appearance_effects = luabind::object_cast<bool>(cur);
		} catch (luabind::cast_failed &) {
		}
	}

	cur = illusion["target"];
	if (luabind::type(cur) != LUA_TNIL) {
		try {
			target = luabind::object_cast<Lua_Client>(cur);
		} catch (luabind::cast_failed &) {
		}
	}

	self->SendIllusionPacket(
		AppearanceStruct{
			.aa_title = aa_title,
			.beard = beard,
			.beard_color = beardcolor,
			.drakkin_details = drakkin_details,
			.drakkin_heritage = drakkin_heritage,
			.drakkin_tattoo = drakkin_tattoo,
			.eye_color_one = eyecolor1,
			.eye_color_two = eyecolor2,
			.face = luclinface,
			.gender_id = gender,
			.hair = hairstyle,
			.hair_color = haircolor,
			.helmet_texture = helmtexture,
			.race_id = race,
			.send_effects = send_appearance_effects,
			.size = size,
			.target = target,
			.texture = texture,
		}
	);
}

void Lua_Mob::ChangeRace(int in) {
	Lua_Safe_Call_Void();
	self->ChangeRace(in);
}

void Lua_Mob::ChangeGender(int in) {
	Lua_Safe_Call_Void();
	self->ChangeGender(in);
}

void Lua_Mob::ChangeTexture(int in) {
	Lua_Safe_Call_Void();
	self->ChangeTexture(in);
}

void Lua_Mob::ChangeHelmTexture(int in) {
	Lua_Safe_Call_Void();
	self->ChangeHelmTexture(in);
}

void Lua_Mob::ChangeHairColor(int in) {
	Lua_Safe_Call_Void();
	self->ChangeHairColor(in);
}

void Lua_Mob::ChangeBeardColor(int in) {
	Lua_Safe_Call_Void();
	self->ChangeBeardColor(in);
}

void Lua_Mob::ChangeEyeColor1(int in) {
	Lua_Safe_Call_Void();
	self->ChangeEyeColor1(in);
}

void Lua_Mob::ChangeEyeColor2(int in) {
	Lua_Safe_Call_Void();
	self->ChangeEyeColor2(in);
}

void Lua_Mob::ChangeHairStyle(int in) {
	Lua_Safe_Call_Void();
	self->ChangeHairStyle(in);
}

void Lua_Mob::ChangeLuclinFace(int in) {
	Lua_Safe_Call_Void();
	self->ChangeLuclinFace(in);
}

void Lua_Mob::ChangeBeard(int in) {
	Lua_Safe_Call_Void();
	self->ChangeBeard(in);
}

void Lua_Mob::ChangeDrakkinHeritage(int in) {
	Lua_Safe_Call_Void();
	self->ChangeDrakkinHeritage(in);
}

void Lua_Mob::ChangeDrakkinTattoo(int in) {
	Lua_Safe_Call_Void();
	self->ChangeDrakkinTattoo(in);
}

void Lua_Mob::ChangeDrakkinDetails(int in) {
	Lua_Safe_Call_Void();
	self->ChangeDrakkinDetails(in);
}

void Lua_Mob::CameraEffect(uint32 duration, float intensity) {
	Lua_Safe_Call_Void();
	self->CameraEffect(duration, intensity);
}

void Lua_Mob::CameraEffect(uint32 duration, float intensity, Lua_Client c) {
	Lua_Safe_Call_Void();
	self->CameraEffect(duration, intensity, c);
}

void Lua_Mob::CameraEffect(uint32 duration, float intensity, Lua_Client c, bool global) {
	Lua_Safe_Call_Void();
	self->CameraEffect(duration, intensity, c, global);
}

void Lua_Mob::SendSpellEffect(uint32 effect_id, uint32 duration, uint32 finish_delay, bool zone_wide, uint32 unk020) {
	Lua_Safe_Call_Void();
	self->SendSpellEffect(effect_id, duration, finish_delay, zone_wide, unk020);
}

void Lua_Mob::SendSpellEffect(uint32 effect_id, uint32 duration, uint32 finish_delay, bool zone_wide, uint32 unk020, bool perm_effect) {
	Lua_Safe_Call_Void();
	self->SendSpellEffect(effect_id, duration, finish_delay, zone_wide, unk020, perm_effect);
}

void Lua_Mob::SendSpellEffect(uint32 effect_id, uint32 duration, uint32 finish_delay, bool zone_wide, uint32 unk020, bool perm_effect,
							  Lua_Client c) {
	Lua_Safe_Call_Void();
	self->SendSpellEffect(effect_id, duration, finish_delay, zone_wide, unk020, perm_effect, c);
}

void Lua_Mob::TempName() {
	Lua_Safe_Call_Void();
	self->TempName();
}

void Lua_Mob::TempName(const char *newname) {
	Lua_Safe_Call_Void();
	self->TempName(newname);
}

std::string Lua_Mob::GetGlobal(const char *varname) {
	Lua_Safe_Call_String();
	return self->GetGlobal(varname);
}

void Lua_Mob::SetGlobal(const char *varname, const char *newvalue, int options, const char *duration) {
	Lua_Safe_Call_Void();
	self->SetGlobal(varname, newvalue, options, duration);
}

void Lua_Mob::SetGlobal(const char *varname, const char *newvalue, int options, const char *duration, Lua_Mob other) {
	Lua_Safe_Call_Void();
	self->SetGlobal(varname, newvalue, options, duration, other);
}

void Lua_Mob::TarGlobal(const char *varname, const char *value, const char *duration, int npc_id, int char_id, int zone_id) {
	Lua_Safe_Call_Void();
	self->TarGlobal(varname, value, duration, npc_id, char_id, zone_id);
}

void Lua_Mob::DelGlobal(const char *varname) {
	Lua_Safe_Call_Void();
	self->DelGlobal(varname);
}

void Lua_Mob::SetSlotTint(int material_slot, int red_tint, int green_tint, int blue_tint) {
	Lua_Safe_Call_Void();
	self->SetSlotTint(material_slot, red_tint, green_tint, blue_tint);
}

void Lua_Mob::WearChange(uint8 material_slot, uint32 texture) {
	Lua_Safe_Call_Void();
	self->WearChange(material_slot, texture);
}

void Lua_Mob::WearChange(uint8 material_slot, uint32 texture, uint32 color) {
	Lua_Safe_Call_Void();
	self->WearChange(material_slot, texture, color);
}

void Lua_Mob::WearChange(uint8 material_slot, uint32 texture, uint32 color, uint32 heros_forge_model) {
	Lua_Safe_Call_Void();
	self->WearChange(material_slot, texture, color, heros_forge_model);
}

void Lua_Mob::DoKnockback(Lua_Mob caster, uint32 push_back, uint32 push_up) {
	Lua_Safe_Call_Void();
	self->DoKnockback(caster, push_back, push_up);
}

void Lua_Mob::AddNimbusEffect(int effect_id) {
	Lua_Safe_Call_Void();
	self->AddNimbusEffect(effect_id);
}

void Lua_Mob::RemoveNimbusEffect(int effect_id) {
	Lua_Safe_Call_Void();
	self->RemoveNimbusEffect(effect_id);
}

bool Lua_Mob::IsRunning() {
	Lua_Safe_Call_Bool();
	return self->IsRunning();
}

void Lua_Mob::SetRunning(bool running) {
	Lua_Safe_Call_Void();
	self->SetRunning(running);
}

void Lua_Mob::SetBodyType(uint8 new_body, bool overwrite_orig) {
	Lua_Safe_Call_Void();
	self->SetBodyType(new_body, overwrite_orig);
}

void Lua_Mob::SetTargetable(bool on) {
	Lua_Safe_Call_Void();
	self->SetTargetable(on);
}

void Lua_Mob::ModSkillDmgTaken(int skill, int value) {
	Lua_Safe_Call_Void();
	self->ModSkillDmgTaken(static_cast<EQ::skills::SkillType>(skill), value);
}

int Lua_Mob::GetModSkillDmgTaken(int skill) {
	Lua_Safe_Call_Int();
	return self->GetModSkillDmgTaken(static_cast<EQ::skills::SkillType>(skill));
}

int Lua_Mob::GetSkillDmgTaken(int skill) {
	Lua_Safe_Call_Int();
	return self->GetSkillDmgTaken(static_cast<EQ::skills::SkillType>(skill));
}

int Lua_Mob::GetFcDamageAmtIncoming(Lua_Mob caster, int32 spell_id)
{
	Lua_Safe_Call_Int();
	return self->GetFcDamageAmtIncoming(caster, spell_id);
}

int Lua_Mob::GetSkillDmgAmt(int skill_id)
{
	Lua_Safe_Call_Int();
	return self->GetSkillDmgAmt(skill_id);
}

void Lua_Mob::SetAllowBeneficial(bool value) {
	Lua_Safe_Call_Void();
	self->SetAllowBeneficial(value);
}

bool Lua_Mob::GetAllowBeneficial() {
	Lua_Safe_Call_Bool();
	return self->GetAllowBeneficial();
}

bool Lua_Mob::IsBeneficialAllowed(Lua_Mob target) {
	Lua_Safe_Call_Bool();
	return self->IsBeneficialAllowed(target);
}

void Lua_Mob::ModVulnerability(int resist, int value) {
	Lua_Safe_Call_Void();
	self->ModVulnerability(resist, value);
}

int Lua_Mob::GetModVulnerability(int resist) {
	Lua_Safe_Call_Int();
	return self->GetModVulnerability(resist);
}

void Lua_Mob::SetDisableMelee(bool disable) {
	Lua_Safe_Call_Void();
	self->SetDisableMelee(disable);
}

bool Lua_Mob::IsMeleeDisabled() {
	Lua_Safe_Call_Bool();
	return IsMeleeDisabled();
}

void Lua_Mob::SetFlurryChance(int value) {
	Lua_Safe_Call_Void();
	self->SetFlurryChance(value);
}

int Lua_Mob::GetFlurryChance() {
	Lua_Safe_Call_Int();
	return self->GetFlurryChance();
}

int Lua_Mob::GetSkill(int skill) {
	Lua_Safe_Call_Int();
	return self->GetSkill(static_cast<EQ::skills::SkillType>(skill));
}

int Lua_Mob::GetSpecialAbility(int ability) {
	Lua_Safe_Call_Int();
	return self->GetSpecialAbility(ability);
}

int Lua_Mob::GetSpecialAbilityParam(int ability, int param) {
	Lua_Safe_Call_Int();
	return self->GetSpecialAbilityParam(ability, param);
}

void Lua_Mob::SetSpecialAbility(int ability, int level) {
	Lua_Safe_Call_Void();
	self->SetSpecialAbility(ability, level);
}

void Lua_Mob::SetSpecialAbilityParam(int ability, int param, int value) {
	Lua_Safe_Call_Void();
	self->SetSpecialAbilityParam(ability, param, value);
}

void Lua_Mob::ClearSpecialAbilities() {
	Lua_Safe_Call_Void();
	self->ClearSpecialAbilities();
}

void Lua_Mob::ProcessSpecialAbilities(std::string str) {
	Lua_Safe_Call_Void();
	self->ProcessSpecialAbilities(str);
}

uint32 Lua_Mob::GetAppearance() {
	Lua_Safe_Call_Int();
	return self->GetAppearance();
}

void Lua_Mob::SetAppearance(int app) {
	Lua_Safe_Call_Void();
	self->SetAppearance(static_cast<EmuAppearance>(app));
}

void Lua_Mob::SetAppearance(int app, bool ignore_self) {
	Lua_Safe_Call_Void();
	self->SetAppearance(static_cast<EmuAppearance>(app), ignore_self);
}

void Lua_Mob::SetDestructibleObject(bool set) {
	Lua_Safe_Call_Void();
	self->SetDestructibleObject(set);
}

bool Lua_Mob::IsImmuneToSpell(int spell_id, Lua_Mob caster) {
	Lua_Safe_Call_Bool();
	return self->IsImmuneToSpell(spell_id, caster);
}

void Lua_Mob::BuffFadeBySpellID(int spell_id) {
	Lua_Safe_Call_Void();
	self->BuffFadeBySpellID(spell_id);
}

void Lua_Mob::BuffFadeByEffect(int effect_id) {
	Lua_Safe_Call_Void();
	self->BuffFadeByEffect(effect_id);
}

void Lua_Mob::BuffFadeByEffect(int effect_id, int skipslot) {
	Lua_Safe_Call_Void();
	self->BuffFadeByEffect(effect_id, skipslot);
}

void Lua_Mob::BuffFadeAll() {
	Lua_Safe_Call_Void();
	self->BuffFadeAll();
}

void Lua_Mob::BuffFadeBySlot(int slot) {
	Lua_Safe_Call_Void();
	self->BuffFadeBySlot(slot);
}

void Lua_Mob::BuffFadeBySlot(int slot, bool recalc_bonuses) {
	Lua_Safe_Call_Void();
	self->BuffFadeBySlot(slot, recalc_bonuses);
}

int Lua_Mob::CanBuffStack(int spell_id, int caster_level) {
	Lua_Safe_Call_Int();
	return self->CanBuffStack(spell_id, caster_level);
}

int Lua_Mob::CanBuffStack(int spell_id, int caster_level, bool fail_if_overwrite) {
	Lua_Safe_Call_Int();
	return self->CanBuffStack(spell_id, caster_level, fail_if_overwrite);
}

void Lua_Mob::SetPseudoRoot(bool in) {
	Lua_Safe_Call_Void();
	self->SetPseudoRoot(in);
}

bool Lua_Mob::IsFeared() {
	Lua_Safe_Call_Bool();
	return self->IsFeared();
}

bool Lua_Mob::IsBlind() {
	Lua_Safe_Call_Bool();
	return self->IsBlind();
}

uint8 Lua_Mob::SeeInvisible() {
	Lua_Safe_Call_Int();
	return self->SeeInvisible();
}

uint8 Lua_Mob::SeeInvisibleUndead() {
	Lua_Safe_Call_Bool();
	return self->SeeInvisibleUndead();
}

bool Lua_Mob::SeeHide() {
	Lua_Safe_Call_Bool();
	return self->SeeHide();
}

bool Lua_Mob::SeeImprovedHide() {
	Lua_Safe_Call_Bool();
	return self->SeeImprovedHide();
}

uint8 Lua_Mob::GetNimbusEffect1() {
	Lua_Safe_Call_Int();
	return self->GetNimbusEffect1();
}

uint8 Lua_Mob::GetNimbusEffect2() {
	Lua_Safe_Call_Int();
	return self->GetNimbusEffect2();
}

uint8 Lua_Mob::GetNimbusEffect3() {
	Lua_Safe_Call_Int();
	return self->GetNimbusEffect3();
}

bool Lua_Mob::IsTargetable() {
	Lua_Safe_Call_Bool();
	return self->IsTargetable();
}

bool Lua_Mob::HasShieldEquipped() {
	Lua_Safe_Call_Bool();
	return self->HasShieldEquipped();
}

bool Lua_Mob::HasTwoHandBluntEquipped() {
	Lua_Safe_Call_Bool();
	return self->HasTwoHandBluntEquipped();
}

bool Lua_Mob::HasTwoHanderEquipped() {
	Lua_Safe_Call_Bool();
	return self->HasTwoHanderEquipped();
}

uint32 Lua_Mob::GetHerosForgeModel(uint8 material_slot) {
	Lua_Safe_Call_Int();
	return self->GetHerosForgeModel(material_slot);
}

uint32 Lua_Mob::IsEliteMaterialItem(uint8 material_slot) {
	Lua_Safe_Call_Int();
	return self->IsEliteMaterialItem(material_slot);
}

float Lua_Mob::GetBaseSize() {
	Lua_Safe_Call_Real();
	return self->GetBaseSize();
}

bool Lua_Mob::HasOwner() {
	Lua_Safe_Call_Bool();
	return self->HasOwner();
}

bool Lua_Mob::IsPet() {
	Lua_Safe_Call_Bool();
	return self->IsPet();
}

bool Lua_Mob::HasPet() {
	Lua_Safe_Call_Bool();
	return self->HasPet();
}

void Lua_Mob::RemovePet() {
	Lua_Safe_Call_Void();
	return self->SetPet(nullptr);
}

void Lua_Mob::SetPet(Lua_Mob new_pet) {
	Lua_Safe_Call_Void();
	return self->SetPet(new_pet);
}

bool Lua_Mob::IsSilenced() {
	Lua_Safe_Call_Bool();
	return self->IsSilenced();
}

bool Lua_Mob::IsAmnesiad() {
	Lua_Safe_Call_Bool();
	return self->IsAmnesiad();
}

int32 Lua_Mob::GetMeleeMitigation() {
	Lua_Safe_Call_Int();
	return self->GetMeleeMitigation();
}

int Lua_Mob::GetWeaponDamageBonus(Lua_Item weapon, bool offhand) {
	Lua_Safe_Call_Int();
	return self->GetWeaponDamageBonus(weapon, offhand);
}

const int Lua_Mob::GetItemStat(uint32 item_id, std::string identifier) {
	Lua_Safe_Call_Int();
	return self->GetItemStat(item_id, identifier);
}

Lua_StatBonuses Lua_Mob::GetItemBonuses()
{
	Lua_Safe_Call_Class(Lua_StatBonuses);
	return self->GetItemBonusesPtr();
}

Lua_StatBonuses Lua_Mob::GetSpellBonuses()
{
	Lua_Safe_Call_Class(Lua_StatBonuses);
	return self->GetSpellBonusesPtr();
}

Lua_StatBonuses Lua_Mob::GetAABonuses()
{
	Lua_Safe_Call_Class(Lua_StatBonuses);
	return self->GetAABonusesPtr();
}

int16 Lua_Mob::GetMeleeDamageMod_SE(uint16 skill)
{
	Lua_Safe_Call_Int();
	return self->GetMeleeDamageMod_SE(skill);
}

int16 Lua_Mob::GetMeleeMinDamageMod_SE(uint16 skill)
{
	Lua_Safe_Call_Int();
	return self->GetMeleeMinDamageMod_SE(skill);
}

bool Lua_Mob::IsCasting() {
	Lua_Safe_Call_Bool();
	return self->IsCasting();
}

int Lua_Mob::AttackAnimation(int Hand, Lua_ItemInst weapon) {
	Lua_Safe_Call_Int();
	return (int)self->AttackAnimation(Hand, weapon);
}

int Lua_Mob::GetWeaponDamage(Lua_Mob against, Lua_ItemInst weapon) {
	Lua_Safe_Call_Int();
	return self->GetWeaponDamage(against, weapon);
}

bool Lua_Mob::IsBerserk() {
	Lua_Safe_Call_Bool();
	return self->IsBerserk();
}

bool Lua_Mob::TryFinishingBlow(Lua_Mob defender, int64 &damage) {
	Lua_Safe_Call_Bool();
	return self->TryFinishingBlow(defender, damage);
}

int Lua_Mob::GetBodyType()
{
	Lua_Safe_Call_Int();
	return (int)self->GetBodyType();
}

int Lua_Mob::GetOrigBodyType()
{
	Lua_Safe_Call_Int();
	return (int)self->GetOrigBodyType();
}

void Lua_Mob::CheckNumHitsRemaining(int type, int32 buff_slot, uint16 spell_id)
{
	Lua_Safe_Call_Void();
	self->CheckNumHitsRemaining((NumHit)type, buff_slot, spell_id);
}

void Lua_Mob::DeleteBucket(std::string bucket_name)
{
	Lua_Safe_Call_Void();
	self->DeleteBucket(bucket_name);
}

std::string Lua_Mob::GetBucket(std::string bucket_name)
{
	Lua_Safe_Call_String();
	return self->GetBucket(bucket_name);
}

std::string Lua_Mob::GetBucketExpires(std::string bucket_name)
{
	Lua_Safe_Call_String();
	return self->GetBucketExpires(bucket_name);
}

std::string Lua_Mob::GetBucketKey()
{
	Lua_Safe_Call_String();
	return {};
}

std::string Lua_Mob::GetBucketRemaining(std::string bucket_name)
{
	Lua_Safe_Call_String();
	return self->GetBucketRemaining(bucket_name);
}

void Lua_Mob::SetBucket(std::string bucket_name, std::string bucket_value)
{
	Lua_Safe_Call_Void();
	self->SetBucket(bucket_name, bucket_value);
}

void Lua_Mob::SetBucket(std::string bucket_name, std::string bucket_value, std::string expiration)
{
	Lua_Safe_Call_Void();
	self->SetBucket(bucket_name, bucket_value, expiration);
}

bool Lua_Mob::IsHorse()
{
	Lua_Safe_Call_Bool();
	return self->IsHorse();
}

Lua_Mob Lua_Mob::GetHateClosest() {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetHateClosest());
}

Lua_Mob Lua_Mob::GetHateClosest(bool skip_mezzed) {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetHateClosest(skip_mezzed));
}

Lua_Bot Lua_Mob::GetHateClosestBot() {
	Lua_Safe_Call_Class(Lua_Bot);
	return Lua_Bot(self->GetHateClosestBot());
}

Lua_Bot Lua_Mob::GetHateClosestBot(bool skip_mezzed) {
	Lua_Safe_Call_Class(Lua_Bot);
	return Lua_Bot(self->GetHateClosestBot());
}

Lua_Client Lua_Mob::GetHateClosestClient() {
	Lua_Safe_Call_Class(Lua_Client);
	return Lua_Client(self->GetHateClosestClient());
}

Lua_Client Lua_Mob::GetHateClosestClient(bool skip_mezzed) {
	Lua_Safe_Call_Class(Lua_Client);
	return Lua_Client(self->GetHateClosestClient(skip_mezzed));
}

Lua_NPC Lua_Mob::GetHateClosestNPC() {
	Lua_Safe_Call_Class(Lua_NPC);
	return Lua_NPC(self->GetHateClosestNPC());
}

Lua_NPC Lua_Mob::GetHateClosestNPC(bool skip_mezzed) {
	Lua_Safe_Call_Class(Lua_NPC);
	return Lua_NPC(self->GetHateClosestNPC(skip_mezzed));
}

Lua_HateList Lua_Mob::GetHateListByDistance() {
	Lua_Safe_Call_Class(Lua_HateList);
	Lua_HateList ret;

	auto h_list = self->GetFilteredHateList();
	for (auto h : h_list) {
		Lua_HateEntry e(h);
		ret.entries.push_back(e);
	}

	return ret;
}

Lua_HateList Lua_Mob::GetHateListByDistance(uint32 distance) {
	Lua_Safe_Call_Class(Lua_HateList);
	Lua_HateList ret;

	auto h_list = self->GetFilteredHateList(EntityFilterType::All, distance);
	for (auto h : h_list) {
		Lua_HateEntry e(h);
		ret.entries.push_back(e);
	}

	return ret;
}

const char *Lua_Mob::GetLastName() {
	Lua_Safe_Call_String();
	return self->GetLastName();
}

bool Lua_Mob::CanClassEquipItem(uint32 item_id) {
	Lua_Safe_Call_Bool();
	return self->CanClassEquipItem(item_id);
}

bool Lua_Mob::CanRaceEquipItem(uint32 item_id) {
	Lua_Safe_Call_Bool();
	return self->CanRaceEquipItem(item_id);
}

void Lua_Mob::RemoveAllNimbusEffects() {
	Lua_Safe_Call_Void();
	self->RemoveAllNimbusEffects();
}

Lua_Bot Lua_Mob::GetHateRandomBot() {
	Lua_Safe_Call_Class(Lua_Bot);
	return Lua_Bot(self->GetHateRandomBot());
}

Lua_Client Lua_Mob::GetHateRandomClient() {
	Lua_Safe_Call_Class(Lua_Client);
	return Lua_Client(self->GetHateRandomClient());
}

Lua_NPC Lua_Mob::GetHateRandomNPC() {
	Lua_Safe_Call_Class(Lua_NPC);
	return Lua_NPC(self->GetHateRandomNPC());
}

uint8 Lua_Mob::GetInvisibleLevel()
{
	Lua_Safe_Call_Int();
	return self->GetInvisibleLevel();
}

uint8 Lua_Mob::GetInvisibleUndeadLevel()
{
	Lua_Safe_Call_Int();
	return self->GetInvisibleUndeadLevel();
}

void Lua_Mob::SetSeeInvisibleLevel(uint8 invisible_level)
{
	Lua_Safe_Call_Void();
	self->SetInnateSeeInvisible(invisible_level);
	self->CalcSeeInvisibleLevel();
}

void Lua_Mob::SetSeeInvisibleUndeadLevel(uint8 invisible_level)
{
	Lua_Safe_Call_Void();
	self->SetSeeInvisibleUndead(invisible_level);
}

void Lua_Mob::ApplySpellBuff(int spell_id) {
	Lua_Safe_Call_Void();
	self->ApplySpellBuff(spell_id);
}

void Lua_Mob::ApplySpellBuff(int spell_id, int duration) {
	Lua_Safe_Call_Void();
	self->ApplySpellBuff(spell_id, duration);
}

void Lua_Mob::ApplySpellBuff(int spell_id, int duration, int level) {
	Lua_Safe_Call_Void();
	self->ApplySpellBuff(spell_id, level);
}

int Lua_Mob::GetBuffStatValueBySlot(uint8 slot, const char* identifier) {
	Lua_Safe_Call_Int();
	return self->GetBuffStatValueBySlot(slot, identifier);
}

int Lua_Mob::GetBuffStatValueBySpell(int spell_id, const char* identifier) {
	Lua_Safe_Call_Int();
	return self->GetBuffStatValueBySpell(spell_id, identifier);
}

void Lua_Mob::SetBuffDuration(int spell_id) {
	Lua_Safe_Call_Void();
	self->SetBuffDuration(spell_id);
}

void Lua_Mob::SetBuffDuration(int spell_id, int duration) {
	Lua_Safe_Call_Void();
	self->SetBuffDuration(spell_id, duration);
}

void Lua_Mob::SetBuffDuration(int spell_id, int duration, int level) {
	Lua_Safe_Call_Void();
	self->SetBuffDuration(spell_id, duration, level);
}

Lua_Mob Lua_Mob::GetUltimateOwner() {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetUltimateOwner());
}

bool Lua_Mob::RandomizeFeatures() {
	Lua_Safe_Call_Bool();
	return self->RandomizeFeatures();
}

bool Lua_Mob::RandomizeFeatures(bool send_illusion) {
	Lua_Safe_Call_Bool();
	return self->RandomizeFeatures(send_illusion);
}

bool Lua_Mob::RandomizeFeatures(bool send_illusion, bool save_variables) {
	Lua_Safe_Call_Bool();
	return self->RandomizeFeatures(send_illusion, save_variables);
}

void Lua_Mob::CloneAppearance(Lua_Mob other) {
	Lua_Safe_Call_Void();
	self->CloneAppearance(other);
}

void Lua_Mob::CloneAppearance(Lua_Mob other, bool clone_name) {
	Lua_Safe_Call_Void();
	self->CloneAppearance(other, clone_name);
}

uint16 Lua_Mob::GetOwnerID() {
	Lua_Safe_Call_Int();
	return self->GetOwnerID();
}

void Lua_Mob::DamageHateList(int64 damage) {
	Lua_Safe_Call_Void();
	self->DamageHateList(damage);
}

void Lua_Mob::DamageHateList(int64 damage, uint32 distance) {
	Lua_Safe_Call_Void();
	self->DamageHateList(damage, distance);
}

void Lua_Mob::DamageHateListClients(int64 damage) {
	Lua_Safe_Call_Void();
	self->DamageHateList(damage, 0, EntityFilterType::Clients);
}

void Lua_Mob::DamageHateListClients(int64 damage, uint32 distance) {
	Lua_Safe_Call_Void();
	self->DamageHateList(damage, distance, EntityFilterType::Clients);
}

void Lua_Mob::DamageHateListNPCs(int64 damage) {
	Lua_Safe_Call_Void();
	self->DamageHateList(damage, 0, EntityFilterType::NPCs);
}

void Lua_Mob::DamageHateListNPCs(int64 damage, uint32 distance) {
	Lua_Safe_Call_Void();
	self->DamageHateList(damage, distance, EntityFilterType::NPCs);
}

void Lua_Mob::DamageHateListPercentage(int64 damage) {
	Lua_Safe_Call_Void();
	self->DamageHateList(damage, 0, EntityFilterType::All, true);
}

void Lua_Mob::DamageHateListPercentage(int64 damage, uint32 distance) {
	Lua_Safe_Call_Void();
	self->DamageHateList(damage, distance, EntityFilterType::All, true);
}

void Lua_Mob::DamageHateListClientsPercentage(int64 damage) {
	Lua_Safe_Call_Void();
	self->DamageHateList(damage, 0, EntityFilterType::Clients, true);
}

void Lua_Mob::DamageHateListClientsPercentage(int64 damage, uint32 distance) {
	Lua_Safe_Call_Void();
	self->DamageHateList(damage, distance, EntityFilterType::Clients, true);
}

void Lua_Mob::DamageHateListNPCsPercentage(int64 damage) {
	Lua_Safe_Call_Void();
	self->DamageHateList(damage, 0, EntityFilterType::NPCs, true);
}

void Lua_Mob::DamageHateListNPCsPercentage(int64 damage, uint32 distance) {
	Lua_Safe_Call_Void();
	self->DamageHateList(damage, distance, EntityFilterType::NPCs, true);
}

Lua_HateList Lua_Mob::GetHateListClients() {
	Lua_Safe_Call_Class(Lua_HateList);
	Lua_HateList ret;

	auto h_list = self->GetFilteredHateList(EntityFilterType::Clients);
	for (auto h : h_list) {
		Lua_HateEntry e(h);
		ret.entries.push_back(e);
	}

	return ret;
}

Lua_HateList Lua_Mob::GetHateListClients(uint32 distance) {
	Lua_Safe_Call_Class(Lua_HateList);
	Lua_HateList ret;

	auto h_list = self->GetFilteredHateList(EntityFilterType::Clients, distance);
	for (auto h : h_list) {
		Lua_HateEntry e(h);
		ret.entries.push_back(e);
	}

	return ret;
}

Lua_HateList Lua_Mob::GetHateListNPCs() {
	Lua_Safe_Call_Class(Lua_HateList);
	Lua_HateList ret;

	auto h_list = self->GetFilteredHateList(EntityFilterType::NPCs);
	for (auto h : h_list) {
		Lua_HateEntry e(h);
		ret.entries.push_back(e);
	}

	return ret;
}

Lua_HateList Lua_Mob::GetHateListNPCs(uint32 distance) {
	Lua_Safe_Call_Class(Lua_HateList);
	Lua_HateList ret;

	auto h_list = self->GetFilteredHateList(EntityFilterType::NPCs, distance);
	for (auto h : h_list) {
		Lua_HateEntry e(h);
		ret.entries.push_back(e);
	}

	return ret;
}

void Lua_Mob::DamageArea(int64 damage) {
	Lua_Safe_Call_Void();
	self->DamageArea(damage);
}

void Lua_Mob::DamageArea(int64 damage, uint32 distance) {
	Lua_Safe_Call_Void();
	self->DamageArea(damage, distance);
}

void Lua_Mob::DamageAreaPercentage(int64 damage) {
	Lua_Safe_Call_Void();
	self->DamageArea(damage, 0, EntityFilterType::All, true);
}

void Lua_Mob::DamageAreaPercentage(int64 damage, uint32 distance) {
	Lua_Safe_Call_Void();
	self->DamageArea(damage, distance, EntityFilterType::All, true);
}

void Lua_Mob::DamageAreaClients(int64 damage) {
	Lua_Safe_Call_Void();
	self->DamageArea(damage, 0, EntityFilterType::Clients);
}

void Lua_Mob::DamageAreaClients(int64 damage, uint32 distance) {
	Lua_Safe_Call_Void();
	self->DamageArea(damage, distance, EntityFilterType::Clients);
}

void Lua_Mob::DamageAreaClientsPercentage(int64 damage) {
	Lua_Safe_Call_Void();
	self->DamageArea(damage, 0, EntityFilterType::Clients, true);
}

void Lua_Mob::DamageAreaClientsPercentage(int64 damage, uint32 distance) {
	Lua_Safe_Call_Void();
	self->DamageArea(damage, distance, EntityFilterType::Clients, true);
}

void Lua_Mob::DamageAreaNPCs(int64 damage) {
	Lua_Safe_Call_Void();
	self->DamageArea(damage, 0, EntityFilterType::NPCs);
}

void Lua_Mob::DamageAreaNPCs(int64 damage, uint32 distance) {
	Lua_Safe_Call_Void();
	self->DamageArea(damage, distance, EntityFilterType::NPCs);
}

void Lua_Mob::DamageAreaNPCsPercentage(int64 damage) {
	Lua_Safe_Call_Void();
	self->DamageArea(damage, 0, EntityFilterType::NPCs, true);
}

void Lua_Mob::DamageAreaNPCsPercentage(int64 damage, uint32 distance) {
	Lua_Safe_Call_Void();
	self->DamageArea(damage, distance, EntityFilterType::NPCs, true);
}

std::string Lua_Mob::GetEntityVariable(std::string variable_name) {
	Lua_Safe_Call_String();
	return self->GetEntityVariable(variable_name);
}

luabind::object Lua_Mob::GetEntityVariables(lua_State* L) {
	auto t = luabind::newtable(L);
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto l = self->GetEntityVariables();
		int i = 1;
		for (const auto& v : l) {
			t[i] = v;
			i++;
		}
	}

	return t;
}

bool Lua_Mob::ClearEntityVariables() {
	Lua_Safe_Call_Bool();
	return self->ClearEntityVariables();
}

bool Lua_Mob::DeleteEntityVariable(std::string variable_name) {
	Lua_Safe_Call_Bool();
	return self->DeleteEntityVariable(variable_name);
}

void Lua_Mob::SetEntityVariable(std::string variable_name, std::string variable_value) {
	Lua_Safe_Call_Void();
	self->SetEntityVariable(variable_name, variable_value);
}

bool Lua_Mob::EntityVariableExists(std::string variable_name) {
	Lua_Safe_Call_Bool();
	return self->EntityVariableExists(variable_name);
}

void Lua_Mob::SendPayload(int payload_id) {
	Lua_Safe_Call_Void();

	if (self->IsClient()) {
		self->CastToClient()->SendPayload(payload_id);
	} else if (self->IsNPC()) {
		self->CastToNPC()->SendPayload(payload_id);
	} else if (self->IsBot()) {
		self->CastToBot()->SendPayload(payload_id);
	}
}

void Lua_Mob::SendPayload(int payload_id, std::string payload_value) {
	Lua_Safe_Call_Void();

	if (self->IsClient()) {
		self->CastToClient()->SendPayload(payload_id, payload_value);
	} else if (self->IsNPC()) {
		self->CastToNPC()->SendPayload(payload_id, payload_value);
	} else if (self->IsBot()) {
		self->CastToBot()->SendPayload(payload_id, payload_value);
	}
}

void Lua_Mob::CopyHateList(Lua_Mob to) {
	Lua_Safe_Call_Void();
	self->CopyHateList(to);
}

bool Lua_Mob::IsAttackAllowed(Lua_Mob target) {
	Lua_Safe_Call_Bool();
	return self->IsAttackAllowed(target);
}

bool Lua_Mob::IsAttackAllowed(Lua_Mob target, bool is_spell_attack) {
	Lua_Safe_Call_Bool();
	return self->IsAttackAllowed(target, is_spell_attack);
}

uint32 Lua_Mob::BuffCount() {
	Lua_Safe_Call_Int();
	return self->BuffCount();
}

uint32 Lua_Mob::BuffCount(bool is_beneficial) {
	Lua_Safe_Call_Int();
	return self->BuffCount(is_beneficial);
}

uint32 Lua_Mob::BuffCount(bool is_beneficial, bool is_detrimental) {
	Lua_Safe_Call_Int();
	return self->BuffCount(is_beneficial, is_detrimental);
}

void Lua_Mob::DamageAreaBots(int64 damage) {
	Lua_Safe_Call_Void();
	self->DamageArea(damage, 0, EntityFilterType::Bots);
}

void Lua_Mob::DamageAreaBots(int64 damage, uint32 distance) {
	Lua_Safe_Call_Void();
	self->DamageArea(damage, distance, EntityFilterType::Bots);
}

void Lua_Mob::DamageAreaBotsPercentage(int64 damage) {
	Lua_Safe_Call_Void();
	self->DamageArea(damage, 0, EntityFilterType::Bots, true);
}

void Lua_Mob::DamageAreaBotsPercentage(int64 damage, uint32 distance) {
	Lua_Safe_Call_Void();
	self->DamageArea(damage, distance, EntityFilterType::Bots, true);
}

void Lua_Mob::DamageHateListBots(int64 damage) {
	Lua_Safe_Call_Void();
	self->DamageHateList(damage, 0, EntityFilterType::Bots);
}

void Lua_Mob::DamageHateListBots(int64 damage, uint32 distance) {
	Lua_Safe_Call_Void();
	self->DamageHateList(damage, distance, EntityFilterType::Bots);
}

void Lua_Mob::DamageHateListBotsPercentage(int64 damage) {
	Lua_Safe_Call_Void();
	self->DamageHateList(damage, 0, EntityFilterType::Bots, true);
}

void Lua_Mob::DamageHateListBotsPercentage(int64 damage, uint32 distance) {
	Lua_Safe_Call_Void();
	self->DamageHateList(damage, distance, EntityFilterType::Bots, true);
}

Lua_HateList Lua_Mob::GetHateListBots() {
	Lua_Safe_Call_Class(Lua_HateList);
	Lua_HateList ret;

	auto h_list = self->GetFilteredHateList(EntityFilterType::Bots);
	for (auto h : h_list) {
		Lua_HateEntry e(h);
		ret.entries.push_back(e);
	}

	return ret;
}

Lua_HateList Lua_Mob::GetHateListBots(uint32 distance) {
	Lua_Safe_Call_Class(Lua_HateList);
	Lua_HateList ret;

	auto h_list = self->GetFilteredHateList(EntityFilterType::Bots, distance);
	for (auto h : h_list) {
		Lua_HateEntry e(h);
		ret.entries.push_back(e);
	}

	return ret;
}

bool Lua_Mob::IsFindable() {
	Lua_Safe_Call_Bool();
	return self->IsFindable();
}


bool Lua_Mob::IsTrackable() {
	Lua_Safe_Call_Bool();
	return self->IsTrackable();
}

float Lua_Mob::GetDefaultRaceSize() {
	Lua_Safe_Call_Real();
	return self->GetDefaultRaceSize();
}

float Lua_Mob::GetDefaultRaceSize(int race_id) {
	Lua_Safe_Call_Real();
	return self->GetDefaultRaceSize(race_id);
}

float Lua_Mob::GetDefaultRaceSize(int race_id, int gender_id) {
	Lua_Safe_Call_Real();
	return self->GetDefaultRaceSize(race_id, gender_id);
}

float Lua_Mob::GetActSpellRange(uint16 spell_id, float range) {
	Lua_Safe_Call_Real();
	return self->GetActSpellRange(spell_id, range);
}

int64 Lua_Mob::GetActSpellDamage(uint16 spell_id, int64 value) {
	Lua_Safe_Call_Int();
	return self->GetActSpellDamage(spell_id, value);
}

int64 Lua_Mob::GetActSpellDamage(uint16 spell_id, int64 value, Lua_Mob target) {
	Lua_Safe_Call_Int();
	return self->GetActSpellDamage(spell_id, value, target);
}

int64 Lua_Mob::GetActDoTDamage(uint16 spell_id, int64 value, Lua_Mob target) {
	Lua_Safe_Call_Int();
	return self->GetActDoTDamage(spell_id, value, target);
}

int64 Lua_Mob::GetActDoTDamage(uint16 spell_id, int64 value, Lua_Mob target, bool from_buff_tic) {
	Lua_Safe_Call_Int();
	return self->GetActDoTDamage(spell_id, value, target, from_buff_tic);
}

int64 Lua_Mob::GetActSpellHealing(uint16 spell_id, int64 value) {
	Lua_Safe_Call_Int();
	return self->GetActSpellHealing(spell_id, value);
}

int64 Lua_Mob::GetActSpellHealing(uint16 spell_id, int64 value, Lua_Mob target) {
	Lua_Safe_Call_Int();
	return self->GetActSpellHealing(spell_id, value, target);
}

int64 Lua_Mob::GetActSpellHealing(uint16 spell_id, int64 value, Lua_Mob target, bool from_buff_tic) {
	Lua_Safe_Call_Int();
	return self->GetActSpellHealing(spell_id, value, target, from_buff_tic);
}

int Lua_Mob::GetActSpellCost(uint16 spell_id, int cost) {
	Lua_Safe_Call_Int();
	return self->GetActSpellCost(spell_id, cost);
}

int Lua_Mob::GetActSpellDuration(uint16 spell_id, int duration) {
	Lua_Safe_Call_Int();
	return self->GetActSpellDuration(spell_id, duration);
}

int Lua_Mob::GetActSpellCasttime(uint16 spell_id, uint32 cast_time) {
	Lua_Safe_Call_Int();
	return self->GetActSpellCasttime(spell_id, cast_time);
}

int64 Lua_Mob::GetActReflectedSpellDamage(uint16 spell_id, int64 value, int effectiveness) {
	Lua_Safe_Call_Int();
	return self->GetActReflectedSpellDamage(spell_id, value, effectiveness);
}

uint32 Lua_Mob::GetRemainingTimeMS(const char* timer_name) {
	Lua_Safe_Call_Int();
	return quest_manager.getremainingtimeMS(timer_name, self);
}

uint32 Lua_Mob::GetTimerDurationMS(const char* timer_name) {
	Lua_Safe_Call_Int();
	return quest_manager.gettimerdurationMS(timer_name, self);
}

bool Lua_Mob::HasTimer(const char* timer_name) {
	Lua_Safe_Call_Bool();
	return quest_manager.hastimer(timer_name, self);
}

bool Lua_Mob::IsPausedTimer(const char* timer_name) {
	Lua_Safe_Call_Bool();
	return quest_manager.ispausedtimer(timer_name, self);
}

void Lua_Mob::PauseTimer(const char* timer_name) {
	Lua_Safe_Call_Void();
	quest_manager.pausetimer(timer_name, self);
}

void Lua_Mob::ResumeTimer(const char* timer_name) {
	Lua_Safe_Call_Void();
	quest_manager.resumetimer(timer_name, self);
}

void Lua_Mob::SetTimer(const char* timer_name, int seconds) {
	Lua_Safe_Call_Void();
	quest_manager.settimer(timer_name, seconds, self);
}

void Lua_Mob::SetTimerMS(const char* timer_name, int milliseconds) {
	Lua_Safe_Call_Void();
	quest_manager.settimerMS(timer_name, milliseconds, self);
}

void Lua_Mob::StopAllTimers() {
	Lua_Safe_Call_Void();
	quest_manager.stopalltimers(self);
}

void Lua_Mob::StopTimer(const char* timer_name) {
	Lua_Safe_Call_Void();
	quest_manager.stoptimer(timer_name, self);
}

luabind::object Lua_Mob::GetBuffSpellIDs(lua_State* L) {
	auto t = luabind::newtable(L);
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto l = self->GetBuffSpellIDs();
		int i = 1;
		for (const auto& v : l) {
			t[i] = v;
			i++;
		}
	}

	return t;
}

bool Lua_Mob::HasSpellEffect(int effect_id) {
	Lua_Safe_Call_Bool();
	return self->HasSpellEffect(effect_id);
}

Lua_Mob_List Lua_Mob::GetCloseMobList() {
	Lua_Safe_Call_Class(Lua_Mob_List);

	Lua_Mob_List ret;

	const auto& l = entity_list.GetCloseMobList(self);

	ret.entries.reserve(l.size());

	for (const auto& e : l) {
		ret.entries.emplace_back(Lua_Mob(e.second));
	}

	return ret;
}

Lua_Mob_List Lua_Mob::GetCloseMobList(float distance) {
	Lua_Safe_Call_Class(Lua_Mob_List);

	Lua_Mob_List ret;

	const auto& l = entity_list.GetCloseMobList(self);

	ret.entries.reserve(l.size());

	for (const auto& e : l) {
		if (self->CalculateDistance(e.second) <= distance) {
			ret.entries.emplace_back(Lua_Mob(e.second));
		}
	}

	return ret;
}

Lua_Mob_List Lua_Mob::GetCloseMobList(float distance, bool ignore_self) {
	Lua_Safe_Call_Class(Lua_Mob_List);

	Lua_Mob_List ret;

	const auto& l = entity_list.GetCloseMobList(self);

	ret.entries.reserve(l.size());

	for (const auto& e : l) {
		if (ignore_self && e.second == self) {
			continue;
		}

		if (self->CalculateDistance(e.second) <= distance) {
			ret.entries.emplace_back(Lua_Mob(e.second));
		}
	}

	return ret;
}

std::string Lua_Mob::GetClassPlural()
{
	Lua_Safe_Call_String();
	return self->GetClassPlural();
}

std::string Lua_Mob::GetRacePlural()
{
	Lua_Safe_Call_String();
	return self->GetRacePlural();
}

bool Lua_Mob::IsTemporaryPet()
{
	Lua_Safe_Call_Bool();
	return self->IsTempPet();
}

uint32 Lua_Mob::GetMobTypeIdentifier()
{
	Lua_Safe_Call_Int();
	return self->GetMobTypeIdentifier();
}

uint32 Lua_Mob::GetHateListCount()
{
	Lua_Safe_Call_Int();
	return self->GetHateListCount();
}

uint32 Lua_Mob::GetHateListBotCount()
{
	Lua_Safe_Call_Int();
	return self->GetHateListCount(HateListCountType::Bot);
}

uint32 Lua_Mob::GetHateListClientCount()
{
	Lua_Safe_Call_Int();
	return self->GetHateListCount(HateListCountType::Client);
}

uint32 Lua_Mob::GetHateListNPCCount()
{
	Lua_Safe_Call_Int();
	return self->GetHateListCount(HateListCountType::NPC);
}

bool Lua_Mob::IsAnimation()
{
	Lua_Safe_Call_Bool();
	return self->IsAnimation();
}

bool Lua_Mob::IsCharmed()
{
	Lua_Safe_Call_Bool();
	return self->IsCharmed();
}

bool Lua_Mob::IsFamiliar()
{
	Lua_Safe_Call_Bool();
	return self->IsFamiliar();
}

bool Lua_Mob::IsTargetLockPet()
{
	Lua_Safe_Call_Bool();
	return self->IsTargetLockPet();
}

bool Lua_Mob::IsPetOwnerBot()
{
	Lua_Safe_Call_Bool();
	return self->IsPetOwnerBot();
}

bool Lua_Mob::IsPetOwnerClient()
{
	Lua_Safe_Call_Bool();
	return self->IsPetOwnerClient();
}

bool Lua_Mob::IsPetOwnerNPC()
{
	Lua_Safe_Call_Bool();
	return self->IsPetOwnerNPC();
}

bool Lua_Mob::IsPetOwnerOfClientBot()
{
	Lua_Safe_Call_Bool();
	return self->IsPetOwnerOfClientBot();
}

bool Lua_Mob::IsDestructibleObject()
{
	Lua_Safe_Call_Bool();
	return self->IsDestructibleObject();
}

bool Lua_Mob::IsBoat()
{
	Lua_Safe_Call_Bool();
	return self->IsBoat();
}

bool Lua_Mob::IsControllableBoat()
{
	Lua_Safe_Call_Bool();
	return self->IsControllableBoat();
}

int Lua_Mob::GetHeroicStrikethrough()
{
	Lua_Safe_Call_Int();
	return self->GetHeroicStrikethrough();
}

bool Lua_Mob::IsAlwaysAggro()
{
	Lua_Safe_Call_Bool();
	return self->AlwaysAggro();
}

std::string Lua_Mob::GetDeityName()
{
	Lua_Safe_Call_String();
	return Deity::GetName(self->GetDeity());
}

luabind::object Lua_Mob::GetBuffs(lua_State* L)
{
	auto t = luabind::newtable(L);
	if (d_) {
		auto     self    = reinterpret_cast<NativeType *>(d_);
		auto     l       = self->GetBuffs();
		int      i       = 1;
		for (int slot_id = 0; slot_id < self->GetMaxBuffSlots(); slot_id++) {
			t[i] = Lua_Buff(&l[slot_id]);
			i++;
		}
	}

	return t;
}

void Lua_Mob::RestoreEndurance()
{
	Lua_Safe_Call_Void();
	self->RestoreEndurance();
}

void Lua_Mob::RestoreHealth()
{
	Lua_Safe_Call_Void();
	self->RestoreHealth();
}

void Lua_Mob::RestoreMana()
{
	Lua_Safe_Call_Void();
	self->RestoreMana();
}

std::string Lua_Mob::GetArchetypeName()
{
	Lua_Safe_Call_String();
	return self->GetArchetypeName();
}

bool Lua_Mob::IsIntelligenceCasterClass()
{
	Lua_Safe_Call_Bool();
	return self->IsIntelligenceCasterClass();
}

bool Lua_Mob::IsPureMeleeClass()
{
	Lua_Safe_Call_Bool();
	return self->IsPureMeleeClass();
}

bool Lua_Mob::IsWisdomCasterClass()
{
	Lua_Safe_Call_Bool();
	return self->IsWisdomCasterClass();
}

std::string Lua_Mob::GetConsiderColor(Lua_Mob other)
{
	Lua_Safe_Call_String();
	return EQ::constants::GetConsiderColorName(self->GetLevelCon(other.GetLevel()));
}

std::string Lua_Mob::GetConsiderColor(uint8 other_level)
{
	Lua_Safe_Call_String();
	return EQ::constants::GetConsiderColorName(self->GetLevelCon(other_level));
}

int Lua_Mob::GetExtraHaste()
{
	Lua_Safe_Call_Int();
	return self->GetExtraHaste();
}

void Lua_Mob::AreaAttack(float distance)
{
	Lua_Safe_Call_Void();
	entity_list.AEAttack(self, distance);
}

void Lua_Mob::AreaAttack(float distance, int16 slot_id)
{
	Lua_Safe_Call_Void();
	entity_list.AEAttack(self, distance, slot_id);
}

void Lua_Mob::AreaAttack(float distance, int16 slot_id, int count)
{
	Lua_Safe_Call_Void();
	entity_list.AEAttack(self, distance, slot_id, count);
}

void Lua_Mob::AreaAttack(float distance, int16 slot_id, int count, bool is_from_spell)
{
	Lua_Safe_Call_Void();
	entity_list.AEAttack(self, distance, slot_id, count, is_from_spell);
}

void Lua_Mob::AreaAttack(float distance, int16 slot_id, int count, bool is_from_spell, int attack_rounds)
{
	Lua_Safe_Call_Void();
	entity_list.AEAttack(self, distance, slot_id, count, is_from_spell, attack_rounds);
}

void Lua_Mob::AreaSpell(Lua_Mob center, uint16 spell_id)
{
	Lua_Safe_Call_Void();
	entity_list.AESpell(self, center, spell_id);
}

void Lua_Mob::AreaSpell(Lua_Mob center, uint16 spell_id, bool affect_caster)
{
	Lua_Safe_Call_Void();
	entity_list.AESpell(self, center, spell_id, affect_caster);
}

void Lua_Mob::AreaSpell(Lua_Mob center, uint16 spell_id, bool affect_caster, int16 resist_adjust)
{
	Lua_Safe_Call_Void();
	entity_list.AESpell(self, center, spell_id, affect_caster, resist_adjust);
}

void Lua_Mob::AreaSpell(Lua_Mob center, uint16 spell_id, bool affect_caster, int16 resist_adjust, int max_targets)
{
	Lua_Safe_Call_Void();
	entity_list.AESpell(self, center, spell_id, affect_caster, resist_adjust, &max_targets);
}

void Lua_Mob::MassGroupBuff(Lua_Mob center, uint16 spell_id)
{
	Lua_Safe_Call_Void();
	entity_list.MassGroupBuff(self, center, spell_id);
}

void Lua_Mob::MassGroupBuff(Lua_Mob center, uint16 spell_id, bool affect_caster)
{
	Lua_Safe_Call_Void();
	entity_list.MassGroupBuff(self, center, spell_id, affect_caster);
}

void Lua_Mob::BuffFadeBeneficial()
{
	Lua_Safe_Call_Void();
	self->BuffFadeBeneficial();
}

void Lua_Mob::BuffFadeDetrimental()
{
	Lua_Safe_Call_Void();
	self->BuffFadeDetrimental();
}

void Lua_Mob::BuffFadeDetrimentalByCaster(Lua_Mob caster)
{
	Lua_Safe_Call_Void();
	self->BuffFadeDetrimentalByCaster(caster);
}

void Lua_Mob::BuffFadeNonPersistDeath()
{
	Lua_Safe_Call_Void();
	self->BuffFadeNonPersistDeath();
}

void Lua_Mob::BuffFadeSongs()
{
	Lua_Safe_Call_Void();
	self->BuffFadeSongs();
}

luabind::object Lua_Mob::GetPausedTimers(lua_State* L) {
	auto t = luabind::newtable(L);
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto l = quest_manager.GetPausedTimers(self);
		int i = 1;
		for (const auto& v : l) {
			t[i] = v;
			i++;
		}
	}

	return t;
}

luabind::object Lua_Mob::GetTimers(lua_State* L) {
	auto t = luabind::newtable(L);
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto l = quest_manager.GetTimers(self);
		int i = 1;
		for (const auto& v : l) {
			t[i] = v;
			i++;
		}
	}

	return t;
}

uint8 Lua_Mob::GetPetType()
{
	Lua_Safe_Call_Int();
	return self->GetPetType();
}

std::string Lua_Mob::GetPetTypeName()
{
	Lua_Safe_Call_String();
	return PetType::GetName(self->GetPetType());
}

void Lua_Mob::SetPetType(uint8 pet_type)
{
	Lua_Safe_Call_Void();
	self->SetPetType(pet_type);
}

luabind::scope lua_register_mob() {
	return luabind::class_<Lua_Mob, Lua_Entity>("Mob")
	.def(luabind::constructor<>())
	.def("AddNimbusEffect", (void(Lua_Mob::*)(int))&Lua_Mob::AddNimbusEffect)
	.def("AddToHateList", (void(Lua_Mob::*)(Lua_Mob))&Lua_Mob::AddToHateList)
	.def("AddToHateList", (void(Lua_Mob::*)(Lua_Mob,int64))&Lua_Mob::AddToHateList)
	.def("AddToHateList", (void(Lua_Mob::*)(Lua_Mob,int64,int64))&Lua_Mob::AddToHateList)
	.def("AddToHateList", (void(Lua_Mob::*)(Lua_Mob,int64,int64,bool))&Lua_Mob::AddToHateList)
	.def("AddToHateList", (void(Lua_Mob::*)(Lua_Mob,int64,int64,bool,bool))&Lua_Mob::AddToHateList)
	.def("AddToHateList", (void(Lua_Mob::*)(Lua_Mob,int64,int64,bool,bool,bool))&Lua_Mob::AddToHateList)
	.def("ApplySpellBuff", (void(Lua_Mob::*)(int))&Lua_Mob::ApplySpellBuff)
	.def("ApplySpellBuff", (void(Lua_Mob::*)(int,int))&Lua_Mob::ApplySpellBuff)
	.def("ApplySpellBuff", (void(Lua_Mob::*)(int,int,int))&Lua_Mob::ApplySpellBuff)
	.def("AreaAttack", (void(Lua_Mob::*)(float))&Lua_Mob::AreaAttack)
	.def("AreaAttack", (void(Lua_Mob::*)(float, int16))&Lua_Mob::AreaAttack)
	.def("AreaAttack", (void(Lua_Mob::*)(float, int16, int))&Lua_Mob::AreaAttack)
	.def("AreaAttack", (void(Lua_Mob::*)(float, int16, int, bool))&Lua_Mob::AreaAttack)
	.def("AreaAttack", (void(Lua_Mob::*)(float, int16, int, bool, int))&Lua_Mob::AreaAttack)
	.def("AreaSpell", (void(Lua_Mob::*)(Lua_Mob, uint16))&Lua_Mob::AreaSpell)
	.def("AreaSpell", (void(Lua_Mob::*)(Lua_Mob, uint16, bool))&Lua_Mob::AreaSpell)
	.def("AreaSpell", (void(Lua_Mob::*)(Lua_Mob, uint16, bool, int16))&Lua_Mob::AreaSpell)
	.def("AreaSpell", (void(Lua_Mob::*)(Lua_Mob, uint16, bool, int16, int))&Lua_Mob::AreaSpell)
	.def("Attack", (bool(Lua_Mob::*)(Lua_Mob))&Lua_Mob::Attack)
	.def("Attack", (bool(Lua_Mob::*)(Lua_Mob,int))&Lua_Mob::Attack)
	.def("Attack", (bool(Lua_Mob::*)(Lua_Mob,int,bool))&Lua_Mob::Attack)
	.def("Attack", (bool(Lua_Mob::*)(Lua_Mob,int,bool,bool))&Lua_Mob::Attack)
	.def("Attack", (bool(Lua_Mob::*)(Lua_Mob,int,bool,bool,bool))&Lua_Mob::Attack)
	.def("Attack", (bool(Lua_Mob::*)(Lua_Mob,int,bool,bool,bool,luabind::adl::object))&Lua_Mob::Attack)
	.def("AttackAnimation", &Lua_Mob::AttackAnimation)
	.def("BehindMob", (bool(Lua_Mob::*)(Lua_Mob))&Lua_Mob::BehindMob)
	.def("BehindMob", (bool(Lua_Mob::*)(Lua_Mob,float))&Lua_Mob::BehindMob)
	.def("BehindMob", (bool(Lua_Mob::*)(Lua_Mob,float,float))&Lua_Mob::BehindMob)
	.def("BehindMob", (bool(Lua_Mob::*)(void))&Lua_Mob::BehindMob)
	.def("BuffCount", (uint32(Lua_Mob::*)(void))&Lua_Mob::BuffCount)
	.def("BuffCount", (uint32(Lua_Mob::*)(bool))&Lua_Mob::BuffCount)
	.def("BuffCount", (uint32(Lua_Mob::*)(bool,bool))&Lua_Mob::BuffCount)
	.def("BuffFadeAll", (void(Lua_Mob::*)(void))&Lua_Mob::BuffFadeAll)
	.def("BuffFadeBeneficial", (void(Lua_Mob::*)(void))&Lua_Mob::BuffFadeBeneficial)
	.def("BuffFadeByEffect", (void(Lua_Mob::*)(int))&Lua_Mob::BuffFadeByEffect)
	.def("BuffFadeByEffect", (void(Lua_Mob::*)(int,int))&Lua_Mob::BuffFadeByEffect)
	.def("BuffFadeBySlot", (void(Lua_Mob::*)(int))&Lua_Mob::BuffFadeBySlot)
	.def("BuffFadeBySlot", (void(Lua_Mob::*)(int,bool))&Lua_Mob::BuffFadeBySlot)
	.def("BuffFadeBySpellID", (void(Lua_Mob::*)(int))&Lua_Mob::BuffFadeBySpellID)
	.def("BuffFadeDetrimental", (void(Lua_Mob::*)(void))&Lua_Mob::BuffFadeDetrimental)
	.def("BuffFadeDetrimentalByCaster", (void(Lua_Mob::*)(Lua_Mob))&Lua_Mob::BuffFadeDetrimentalByCaster)
	.def("BuffFadeNonPersistDeath", (void(Lua_Mob::*)(void))&Lua_Mob::BuffFadeNonPersistDeath)
	.def("BuffFadeSongs", (void(Lua_Mob::*)(void))&Lua_Mob::BuffFadeSongs)
	.def("CalculateDistance", (float(Lua_Mob::*)(double,double,double))&Lua_Mob::CalculateDistance)
	.def("CalculateDistance", (float(Lua_Mob::*)(Lua_Mob))&Lua_Mob::CalculateDistance)
	.def("CalculateHeadingToTarget", (double(Lua_Mob::*)(double,double))&Lua_Mob::CalculateHeadingToTarget)
	.def("CameraEffect", (void(Lua_Mob::*)(uint32,float))&Lua_Mob::CameraEffect)
	.def("CameraEffect", (void(Lua_Mob::*)(uint32,float,Lua_Client))&Lua_Mob::CameraEffect)
	.def("CameraEffect", (void(Lua_Mob::*)(uint32,float,Lua_Client,bool))&Lua_Mob::CameraEffect)
	.def("CanBuffStack", (int(Lua_Mob::*)(int,int))&Lua_Mob::CanBuffStack)
	.def("CanBuffStack", (int(Lua_Mob::*)(int,int,bool))&Lua_Mob::CanBuffStack)
	.def("CanClassEquipItem", &Lua_Mob::CanClassEquipItem)
	.def("CanRaceEquipItem", &Lua_Mob::CanRaceEquipItem)
	.def("CanThisClassBlock", (bool(Lua_Mob::*)(void))&Lua_Mob::CanThisClassBlock)
	.def("CanThisClassDodge", (bool(Lua_Mob::*)(void))&Lua_Mob::CanThisClassDodge)
	.def("CanThisClassDoubleAttack", (bool(Lua_Mob::*)(void))&Lua_Mob::CanThisClassDoubleAttack)
	.def("CanThisClassDualWield", (bool(Lua_Mob::*)(void))&Lua_Mob::CanThisClassDualWield)
	.def("CanThisClassParry", (bool(Lua_Mob::*)(void))&Lua_Mob::CanThisClassParry)
	.def("CanThisClassRiposte", (bool(Lua_Mob::*)(void))&Lua_Mob::CanThisClassRiposte)
	.def("CastSpell", (bool(Lua_Mob::*)(int,int))&Lua_Mob::CastSpell)
	.def("CastSpell", (bool(Lua_Mob::*)(int,int,int))&Lua_Mob::CastSpell)
	.def("CastSpell", (bool(Lua_Mob::*)(int,int,int,int))&Lua_Mob::CastSpell)
	.def("CastSpell", (bool(Lua_Mob::*)(int,int,int,int,int))&Lua_Mob::CastSpell)
	.def("CastSpell", (bool(Lua_Mob::*)(int,int,int,int,int,int))&Lua_Mob::CastSpell)
	.def("CastSpell", (bool(Lua_Mob::*)(int,int,int,int,int,int,int,int))&Lua_Mob::CastSpell)
	.def("CastSpell", (bool(Lua_Mob::*)(int,int,int,int,int,int,int,int,int))&Lua_Mob::CastSpell)
	.def("ChangeBeard", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeBeard)
	.def("ChangeBeardColor", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeBeardColor)
	.def("ChangeDrakkinDetails", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeDrakkinDetails)
	.def("ChangeDrakkinHeritage", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeDrakkinHeritage)
	.def("ChangeDrakkinTattoo", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeDrakkinTattoo)
	.def("ChangeEyeColor1", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeEyeColor1)
	.def("ChangeEyeColor2", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeEyeColor2)
	.def("ChangeGender", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeGender)
	.def("ChangeHairColor", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeHairColor)
	.def("ChangeHairStyle", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeHairStyle)
	.def("ChangeHelmTexture", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeHelmTexture)
	.def("ChangeLuclinFace", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeLuclinFace)
	.def("ChangeRace", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeRace)
	.def("ChangeSize", (void(Lua_Mob::*)(double))&Lua_Mob::ChangeSize)
	.def("ChangeSize", (void(Lua_Mob::*)(double,bool))&Lua_Mob::ChangeSize)
	.def("ChangeTexture", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeTexture)
	.def("Charmed", (bool(Lua_Mob::*)(void))&Lua_Mob::Charmed)
	.def("CheckAggro", (bool(Lua_Mob::*)(Lua_Mob))&Lua_Mob::CheckAggro)
	.def("CheckAggroAmount", (int(Lua_Mob::*)(int))&Lua_Mob::CheckAggroAmount)
	.def("CheckAggroAmount", (int(Lua_Mob::*)(int,bool))&Lua_Mob::CheckAggroAmount)
	.def("CheckHealAggroAmount", (int(Lua_Mob::*)(int))&Lua_Mob::CheckHealAggroAmount)
	.def("CheckHealAggroAmount", (int(Lua_Mob::*)(int,uint32))&Lua_Mob::CheckHealAggroAmount)
	.def("CheckLoS", (bool(Lua_Mob::*)(Lua_Mob))&Lua_Mob::CheckLoS)
	.def("CheckLoSToLoc", (bool(Lua_Mob::*)(double,double,double))&Lua_Mob::CheckLoSToLoc)
	.def("CheckLoSToLoc", (bool(Lua_Mob::*)(double,double,double,double))&Lua_Mob::CheckLoSToLoc)
	.def("CheckNumHitsRemaining", &Lua_Mob::CheckNumHitsRemaining)
	.def("ClearEntityVariables", (bool(Lua_Mob::*)(void))&Lua_Mob::ClearEntityVariables)
	.def("ClearSpecialAbilities", (void(Lua_Mob::*)(void))&Lua_Mob::ClearSpecialAbilities)
	.def("CloneAppearance", (void(Lua_Mob::*)(Lua_Mob))&Lua_Mob::CloneAppearance)
	.def("CloneAppearance", (void(Lua_Mob::*)(Lua_Mob,bool))&Lua_Mob::CloneAppearance)
	.def("CombatRange", (bool(Lua_Mob::*)(Lua_Mob))&Lua_Mob::CombatRange)
	.def("CopyHateList", (void(Lua_Mob::*)(Lua_Mob))&Lua_Mob::CopyHateList)
	.def("Damage", (void(Lua_Mob::*)(Lua_Mob,int64,int,int))&Lua_Mob::Damage)
	.def("Damage", (void(Lua_Mob::*)(Lua_Mob,int64,int,int,bool))&Lua_Mob::Damage)
	.def("Damage", (void(Lua_Mob::*)(Lua_Mob,int64,int,int,bool,int))&Lua_Mob::Damage)
	.def("Damage", (void(Lua_Mob::*)(Lua_Mob,int64,int,int,bool,int,bool))&Lua_Mob::Damage)
	.def("DamageArea", (void(Lua_Mob::*)(int64))&Lua_Mob::DamageArea)
	.def("DamageArea", (void(Lua_Mob::*)(int64,uint32))&Lua_Mob::DamageArea)
	.def("DamageAreaPercentage", (void(Lua_Mob::*)(int64))&Lua_Mob::DamageAreaPercentage)
	.def("DamageAreaPercentage", (void(Lua_Mob::*)(int64,uint32))&Lua_Mob::DamageAreaPercentage)
	.def("DamageAreaBots", (void(Lua_Mob::*)(int64))&Lua_Mob::DamageAreaBots)
	.def("DamageAreaBots", (void(Lua_Mob::*)(int64,uint32))&Lua_Mob::DamageAreaBots)
	.def("DamageAreaBotsPercentage", (void(Lua_Mob::*)(int64))&Lua_Mob::DamageAreaBotsPercentage)
	.def("DamageAreaBotsPercentage", (void(Lua_Mob::*)(int64,uint32))&Lua_Mob::DamageAreaBotsPercentage)
	.def("DamageAreaClients", (void(Lua_Mob::*)(int64))&Lua_Mob::DamageAreaClients)
	.def("DamageAreaClients", (void(Lua_Mob::*)(int64,uint32))&Lua_Mob::DamageAreaClients)
	.def("DamageAreaClientsPercentage", (void(Lua_Mob::*)(int64))&Lua_Mob::DamageAreaClientsPercentage)
	.def("DamageAreaClientsPercentage", (void(Lua_Mob::*)(int64,uint32))&Lua_Mob::DamageAreaClientsPercentage)
	.def("DamageAreaNPCs", (void(Lua_Mob::*)(int64))&Lua_Mob::DamageAreaNPCs)
	.def("DamageAreaNPCs", (void(Lua_Mob::*)(int64,uint32))&Lua_Mob::DamageAreaNPCs)
	.def("DamageAreaNPCsPercentage", (void(Lua_Mob::*)(int64))&Lua_Mob::DamageAreaNPCsPercentage)
	.def("DamageAreaNPCsPercentage", (void(Lua_Mob::*)(int64,uint32))&Lua_Mob::DamageAreaNPCsPercentage)
	.def("DamageHateList", (void(Lua_Mob::*)(int64))&Lua_Mob::DamageHateList)
	.def("DamageHateList", (void(Lua_Mob::*)(int64,uint32))&Lua_Mob::DamageHateList)
	.def("DamageHateListBots", (void(Lua_Mob::*)(int64))&Lua_Mob::DamageHateListBots)
	.def("DamageHateListBots", (void(Lua_Mob::*)(int64,uint32))&Lua_Mob::DamageHateListBots)
	.def("DamageHateListBotsPercentage", (void(Lua_Mob::*)(int64))&Lua_Mob::DamageHateListBots)
	.def("DamageHateListBotsPercentage", (void(Lua_Mob::*)(int64,uint32))&Lua_Mob::DamageHateListBots)
	.def("DamageHateListClients", (void(Lua_Mob::*)(int64))&Lua_Mob::DamageHateListClients)
	.def("DamageHateListClients", (void(Lua_Mob::*)(int64,uint32))&Lua_Mob::DamageHateListClients)
	.def("DamageHateListClientsPercentage", (void(Lua_Mob::*)(int64))&Lua_Mob::DamageHateListClientsPercentage)
	.def("DamageHateListClientsPercentage", (void(Lua_Mob::*)(int64,uint32))&Lua_Mob::DamageHateListClientsPercentage)
	.def("DamageHateListNPCs", (void(Lua_Mob::*)(int64))&Lua_Mob::DamageHateListNPCs)
	.def("DamageHateListNPCs", (void(Lua_Mob::*)(int64,uint32))&Lua_Mob::DamageHateListNPCs)
	.def("DamageHateListNPCsPercentage", (void(Lua_Mob::*)(int64))&Lua_Mob::DamageHateListNPCsPercentage)
	.def("DamageHateListNPCsPercentage", (void(Lua_Mob::*)(int64,uint32))&Lua_Mob::DamageHateListNPCsPercentage)
	.def("DamageHateListPercentage", (void(Lua_Mob::*)(int64))&Lua_Mob::DamageHateListPercentage)
	.def("DamageHateListPercentage", (void(Lua_Mob::*)(int64,uint32))&Lua_Mob::DamageHateListPercentage)
	.def("DelGlobal", (void(Lua_Mob::*)(const char*))&Lua_Mob::DelGlobal)
	.def("DeleteBucket", (void(Lua_Mob::*)(std::string))&Lua_Mob::DeleteBucket)
	.def("DeleteEntityVariable", (bool(Lua_Mob::*)(std::string))&Lua_Mob::DeleteEntityVariable)
	.def("Depop", (void(Lua_Mob::*)(bool))&Lua_Mob::Depop)
	.def("Depop", (void(Lua_Mob::*)(void))&Lua_Mob::Depop)
	.def("DivineAura", (bool(Lua_Mob::*)(void))&Lua_Mob::DivineAura)
	.def("DoAnim", (void(Lua_Mob::*)(int))&Lua_Mob::DoAnim)
	.def("DoAnim", (void(Lua_Mob::*)(int,int))&Lua_Mob::DoAnim)
	.def("DoAnim", (void(Lua_Mob::*)(int,int,bool))&Lua_Mob::DoAnim)
	.def("DoAnim", (void(Lua_Mob::*)(int,int,bool,int))&Lua_Mob::DoAnim)
	.def("DoArcheryAttackDmg", (void(Lua_Mob::*)(Lua_Mob))&Lua_Mob::DoArcheryAttackDmg)
	.def("DoArcheryAttackDmg", (void(Lua_Mob::*)(Lua_Mob,Lua_ItemInst))&Lua_Mob::DoArcheryAttackDmg)
	.def("DoArcheryAttackDmg", (void(Lua_Mob::*)(Lua_Mob,Lua_ItemInst,Lua_ItemInst))&Lua_Mob::DoArcheryAttackDmg)
	.def("DoArcheryAttackDmg", (void(Lua_Mob::*)(Lua_Mob,Lua_ItemInst,Lua_ItemInst,int))&Lua_Mob::DoArcheryAttackDmg)
	.def("DoArcheryAttackDmg", (void(Lua_Mob::*)(Lua_Mob,Lua_ItemInst,Lua_ItemInst,int,int))&Lua_Mob::DoArcheryAttackDmg)
	.def("DoArcheryAttackDmg", (void(Lua_Mob::*)(Lua_Mob,Lua_ItemInst,Lua_ItemInst,int,int,int))&Lua_Mob::DoArcheryAttackDmg)
	.def("DoKnockback", (void(Lua_Mob::*)(Lua_Mob,uint32,uint32))&Lua_Mob::DoKnockback)
	.def("DoMeleeSkillAttackDmg", (void(Lua_Mob::*)(Lua_Mob,int,int))&Lua_Mob::DoMeleeSkillAttackDmg)
	.def("DoMeleeSkillAttackDmg", (void(Lua_Mob::*)(Lua_Mob,int,int,int))&Lua_Mob::DoMeleeSkillAttackDmg)
	.def("DoMeleeSkillAttackDmg", (void(Lua_Mob::*)(Lua_Mob,int,int,int,int))&Lua_Mob::DoMeleeSkillAttackDmg)
	.def("DoMeleeSkillAttackDmg", (void(Lua_Mob::*)(Lua_Mob,int,int,int,int,bool))&Lua_Mob::DoMeleeSkillAttackDmg)
	.def("DoSpecialAttackDamage", (void(Lua_Mob::*)(Lua_Mob,int,int))&Lua_Mob::DoSpecialAttackDamage)
	.def("DoSpecialAttackDamage", (void(Lua_Mob::*)(Lua_Mob,int,int,int))&Lua_Mob::DoSpecialAttackDamage)
	.def("DoSpecialAttackDamage", (void(Lua_Mob::*)(Lua_Mob,int,int,int,int))&Lua_Mob::DoSpecialAttackDamage)
	.def("DoSpecialAttackDamage", (void(Lua_Mob::*)(Lua_Mob,int,int,int,int,int))&Lua_Mob::DoSpecialAttackDamage)
	.def("DoThrowingAttackDmg", (void(Lua_Mob::*)(Lua_Mob))&Lua_Mob::DoThrowingAttackDmg)
	.def("DoThrowingAttackDmg", (void(Lua_Mob::*)(Lua_Mob,Lua_ItemInst))&Lua_Mob::DoThrowingAttackDmg)
	.def("DoThrowingAttackDmg", (void(Lua_Mob::*)(Lua_Mob,Lua_ItemInst,Lua_Item))&Lua_Mob::DoThrowingAttackDmg)
	.def("DoThrowingAttackDmg", (void(Lua_Mob::*)(Lua_Mob,Lua_ItemInst,Lua_Item,int))&Lua_Mob::DoThrowingAttackDmg)
	.def("DoThrowingAttackDmg", (void(Lua_Mob::*)(Lua_Mob,Lua_ItemInst,Lua_Item,int,int))&Lua_Mob::DoThrowingAttackDmg)
	.def("DoThrowingAttackDmg", (void(Lua_Mob::*)(Lua_Mob,Lua_ItemInst,Lua_Item,int,int,int))&Lua_Mob::DoThrowingAttackDmg)
	.def("DoubleAggro", &Lua_Mob::DoubleAggro)
	.def("Emote", &Lua_Mob::Emote)
	.def("EntityVariableExists", &Lua_Mob::EntityVariableExists)
	.def("FaceTarget", (void(Lua_Mob::*)(Lua_Mob))&Lua_Mob::FaceTarget)
	.def("FindBuff", (bool(Lua_Mob::*)(uint16))&Lua_Mob::FindBuff)
	.def("FindBuff", (bool(Lua_Mob::*)(uint16,uint16))&Lua_Mob::FindBuff)
	.def("FindBuffBySlot", (uint16(Lua_Mob::*)(int))&Lua_Mob::FindBuffBySlot)
	.def("FindGroundZ", (double(Lua_Mob::*)(double,double))&Lua_Mob::FindGroundZ)
	.def("FindGroundZ", (double(Lua_Mob::*)(double,double,double))&Lua_Mob::FindGroundZ)
	.def("FindType", (bool(Lua_Mob::*)(int))&Lua_Mob::FindType)
	.def("FindType", (bool(Lua_Mob::*)(int,bool))&Lua_Mob::FindType)
	.def("FindType", (bool(Lua_Mob::*)(int,bool,int))&Lua_Mob::FindType)
	.def("GMMove", (void(Lua_Mob::*)(double,double,double))&Lua_Mob::GMMove)
	.def("GMMove", (void(Lua_Mob::*)(double,double,double,double))&Lua_Mob::GMMove)
	.def("GMMove", (void(Lua_Mob::*)(double,double,double,double,bool))&Lua_Mob::GMMove)
	.def("GetAA", (int(Lua_Mob::*)(int))&Lua_Mob::GetAA)
	.def("GetAABonuses", &Lua_Mob::GetAABonuses)
	.def("GetAAByAAID", (int(Lua_Mob::*)(int))&Lua_Mob::GetAAByAAID)
	.def("GetAC", &Lua_Mob::GetAC)
	.def("GetAGI", &Lua_Mob::GetAGI)
	.def("GetATK", &Lua_Mob::GetATK)
	.def("GetActDoTDamage", (int64(Lua_Mob::*)(uint16,int64,Lua_Mob))&Lua_Mob::GetActDoTDamage)
	.def("GetActDoTDamage", (int64(Lua_Mob::*)(uint16,int64,Lua_Mob,bool))&Lua_Mob::GetActDoTDamage)
	.def("GetActReflectedSpellDamage", &Lua_Mob::GetActReflectedSpellDamage)
	.def("GetActSpellCasttime", &Lua_Mob::GetActSpellCasttime)
	.def("GetActSpellCost", &Lua_Mob::GetActSpellCost)
	.def("GetActSpellDuration", &Lua_Mob::GetActSpellDuration)
	.def("GetActSpellDamage", (int64(Lua_Mob::*)(uint16,int64))&Lua_Mob::GetActSpellDamage)
	.def("GetActSpellDamage", (int64(Lua_Mob::*)(uint16,int64,Lua_Mob))&Lua_Mob::GetActSpellDamage)
	.def("GetActSpellHealing", (int64(Lua_Mob::*)(uint16,int64))&Lua_Mob::GetActSpellHealing)
	.def("GetActSpellHealing", (int64(Lua_Mob::*)(uint16,int64,Lua_Mob))&Lua_Mob::GetActSpellHealing)
	.def("GetActSpellHealing", (int64(Lua_Mob::*)(uint16,int64,Lua_Mob,bool))&Lua_Mob::GetActSpellHealing)
	.def("GetActSpellRange", &Lua_Mob::GetActSpellRange)
	.def("GetAggroRange", (float(Lua_Mob::*)(void))&Lua_Mob::GetAggroRange)
	.def("GetAllowBeneficial", (bool(Lua_Mob::*)(void))&Lua_Mob::GetAllowBeneficial)
	.def("GetAppearance", (uint32(Lua_Mob::*)(void))&Lua_Mob::GetAppearance)
	.def("GetArchetypeName", &Lua_Mob::GetArchetypeName)
	.def("GetAssistRange", (float(Lua_Mob::*)(void))&Lua_Mob::GetAssistRange)
	.def("GetBaseGender", &Lua_Mob::GetBaseGender)
	.def("GetBaseRace", &Lua_Mob::GetBaseRace)
	.def("GetBaseSize", (double(Lua_Mob::*)(void))&Lua_Mob::GetBaseSize)
	.def("GetBeard", &Lua_Mob::GetBeard)
	.def("GetBeardColor", &Lua_Mob::GetBeardColor)
	.def("GetBodyType", &Lua_Mob::GetBodyType)
	.def("GetBucket", (std::string(Lua_Mob::*)(std::string))&Lua_Mob::GetBucket)
	.def("GetBucketExpires", (std::string(Lua_Mob::*)(std::string))&Lua_Mob::GetBucketExpires)
	.def("GetBucketKey", (std::string(Lua_Mob::*)(void))&Lua_Mob::GetBucketKey)
	.def("GetBucketRemaining", (std::string(Lua_Mob::*)(std::string))&Lua_Mob::GetBucketRemaining)
	.def("GetBuffs", &Lua_Mob::GetBuffs)
	.def("GetBuffSlotFromType", &Lua_Mob::GetBuffSlotFromType)
	.def("GetBuffSpellIDs", &Lua_Mob::GetBuffSpellIDs)
	.def("GetBuffStatValueBySlot", (void(Lua_Mob::*)(uint8, const char*))& Lua_Mob::GetBuffStatValueBySlot)
	.def("GetBuffStatValueBySpell", (void(Lua_Mob::*)(int, const char*))&Lua_Mob::GetBuffStatValueBySpell)
	.def("GetCHA", &Lua_Mob::GetCHA)
	.def("GetCR", &Lua_Mob::GetCR)
	.def("GetCasterLevel", &Lua_Mob::GetCasterLevel)
	.def("GetClass", &Lua_Mob::GetClass)
	.def("GetClassName", &Lua_Mob::GetClassName)
	.def("GetBaseRaceName", &Lua_Mob::GetBaseRaceName)
	.def("GetClassPlural", &Lua_Mob::GetClassPlural)
	.def("GetCleanName", &Lua_Mob::GetCleanName)
	.def("GetCloseMobList", (Lua_Mob_List(Lua_Mob::*)(void))&Lua_Mob::GetCloseMobList)
	.def("GetCloseMobList", (Lua_Mob_List(Lua_Mob::*)(float))&Lua_Mob::GetCloseMobList)
	.def("GetCloseMobList", (Lua_Mob_List(Lua_Mob::*)(float,bool))&Lua_Mob::GetCloseMobList)
	.def("GetConsiderColor", (std::string(Lua_Mob::*)(Lua_Mob))&Lua_Mob::GetConsiderColor)
	.def("GetConsiderColor", (std::string(Lua_Mob::*)(uint8))&Lua_Mob::GetConsiderColor)
	.def("GetCorruption", &Lua_Mob::GetCorruption)
	.def("GetDEX", &Lua_Mob::GetDEX)
	.def("GetDR", &Lua_Mob::GetDR)
	.def("GetDamageAmount", (uint32(Lua_Mob::*)(Lua_Mob))&Lua_Mob::GetDamageAmount)
	.def("GetDefaultRaceSize", (float(Lua_Mob::*)(void))&Lua_Mob::GetDefaultRaceSize)
	.def("GetDefaultRaceSize", (float(Lua_Mob::*)(int))&Lua_Mob::GetDefaultRaceSize)
	.def("GetDefaultRaceSize", (float(Lua_Mob::*)(int,int))&Lua_Mob::GetDefaultRaceSize)
	.def("GetDeity", &Lua_Mob::GetDeity)
	.def("GetDeityName", &Lua_Mob::GetDeityName)
	.def("GetDisplayAC", &Lua_Mob::GetDisplayAC)
	.def("GetDrakkinDetails", &Lua_Mob::GetDrakkinDetails)
	.def("GetDrakkinHeritage", &Lua_Mob::GetDrakkinHeritage)
	.def("GetDrakkinTattoo", &Lua_Mob::GetDrakkinTattoo)
	.def("GetEntityVariable", &Lua_Mob::GetEntityVariable)
	.def("GetEntityVariables",&Lua_Mob::GetEntityVariables)
	.def("GetExtraHaste",&Lua_Mob::GetExtraHaste)
	.def("GetEyeColor1", &Lua_Mob::GetEyeColor1)
	.def("GetEyeColor2", &Lua_Mob::GetEyeColor2)
	.def("GetFR", &Lua_Mob::GetFR)
	.def("GetFcDamageAmtIncoming", &Lua_Mob::GetFcDamageAmtIncoming)
	.def("GetFlurryChance", (int(Lua_Mob::*)(void))&Lua_Mob::GetFlurryChance)
	.def("GetGender", &Lua_Mob::GetGender)
	.def("GetGlobal", (std::string(Lua_Mob::*)(const char*))&Lua_Mob::GetGlobal)
	.def("GetHP", &Lua_Mob::GetHP)
	.def("GetHPRatio", &Lua_Mob::GetHPRatio)
	.def("GetHairColor", &Lua_Mob::GetHairColor)
	.def("GetHairStyle", &Lua_Mob::GetHairStyle)
	.def("GetHandToHandDamage", (int(Lua_Mob::*)(void))&Lua_Mob::GetHandToHandDamage)
	.def("GetHandToHandDelay", (int(Lua_Mob::*)(void))&Lua_Mob::GetHandToHandDelay)
	.def("GetHaste", (int(Lua_Mob::*)(void))&Lua_Mob::GetHaste)
	.def("GetHateAmount", (int64(Lua_Mob::*)(Lua_Mob))&Lua_Mob::GetHateAmount)
	.def("GetHateAmount", (int64(Lua_Mob::*)(Lua_Mob,bool))&Lua_Mob::GetHateAmount)
	.def("GetHateClosest", (Lua_Mob(Lua_Mob::*)(void))&Lua_Mob::GetHateClosest)
	.def("GetHateClosest", (Lua_Mob(Lua_Mob::*)(bool))&Lua_Mob::GetHateClosest)
	.def("GetHateClosestBot", (Lua_Bot(Lua_Mob::*)(void))&Lua_Mob::GetHateClosestBot)
	.def("GetHateClosestBot", (Lua_Bot(Lua_Mob::*)(bool))&Lua_Mob::GetHateClosestBot)
	.def("GetHateClosestClient", (Lua_Client(Lua_Mob::*)(void))&Lua_Mob::GetHateClosestClient)
	.def("GetHateClosestClient", (Lua_Client(Lua_Mob::*)(bool))&Lua_Mob::GetHateClosestClient)
	.def("GetHateClosestNPC", (Lua_NPC(Lua_Mob::*)(void))&Lua_Mob::GetHateClosestNPC)
	.def("GetHateClosestNPC", (Lua_NPC(Lua_Mob::*)(bool))&Lua_Mob::GetHateClosestNPC)
	.def("GetHateDamageTop", (Lua_Mob(Lua_Mob::*)(Lua_Mob))&Lua_Mob::GetHateDamageTop)
	.def("GetHateList", &Lua_Mob::GetHateList)
	.def("GetHateListBots", (Lua_HateList(Lua_Mob::*)(void))&Lua_Mob::GetHateListBots)
	.def("GetHateListBots", (Lua_HateList(Lua_Mob::*)(uint32))&Lua_Mob::GetHateListBots)
	.def("GetHateListBotCount", &Lua_Mob::GetHateListBotCount)
	.def("GetHateListClients", (Lua_HateList(Lua_Mob::*)(void))&Lua_Mob::GetHateListClients)
	.def("GetHateListClients", (Lua_HateList(Lua_Mob::*)(uint32))&Lua_Mob::GetHateListClients)
	.def("GetHateListClientCount", &Lua_Mob::GetHateListClientCount)
	.def("GetHateListNPCs", (Lua_HateList(Lua_Mob::*)(void))&Lua_Mob::GetHateListNPCs)
	.def("GetHateListNPCs", (Lua_HateList(Lua_Mob::*)(uint32))&Lua_Mob::GetHateListNPCs)
	.def("GetHateListNPCCount", &Lua_Mob::GetHateListNPCCount)
	.def("GetHateListByDistance", (Lua_HateList(Lua_Mob::*)(void))&Lua_Mob::GetHateListByDistance)
	.def("GetHateListByDistance", (Lua_HateList(Lua_Mob::*)(uint32))&Lua_Mob::GetHateListByDistance)
	.def("GetHateListCount", &Lua_Mob::GetHateListCount)
	.def("GetHateRandom", (Lua_Mob(Lua_Mob::*)(void))&Lua_Mob::GetHateRandom)
	.def("GetHateRandomBot", (Lua_Bot(Lua_Mob::*)(void))&Lua_Mob::GetHateRandomBot)
	.def("GetHateRandomClient", (Lua_Client(Lua_Mob::*)(void))&Lua_Mob::GetHateRandomClient)
	.def("GetHateRandomNPC", (Lua_NPC(Lua_Mob::*)(void))&Lua_Mob::GetHateRandomNPC)
	.def("GetHateTop", (Lua_Mob(Lua_Mob::*)(void))&Lua_Mob::GetHateTop)
	.def("GetHateTopBot", (Lua_Bot(Lua_Mob::*)(void))&Lua_Mob::GetHateTopBot)
	.def("GetHateTopClient", (Lua_Client(Lua_Mob::*)(void))&Lua_Mob::GetHateTopClient)
	.def("GetHateTopNPC", (Lua_NPC(Lua_Mob::*)(void))&Lua_Mob::GetHateTopNPC)
	.def("GetHeading", &Lua_Mob::GetHeading)
	.def("GetHelmTexture", &Lua_Mob::GetHelmTexture)
	.def("GetHeroicStrikethrough", &Lua_Mob::GetHeroicStrikethrough)
	.def("GetHerosForgeModel", (int32(Lua_Mob::*)(uint8))&Lua_Mob::GetHerosForgeModel)
	.def("GetINT", &Lua_Mob::GetINT)
	.def("GetInvisibleLevel", (uint8(Lua_Mob::*)(void))&Lua_Mob::GetInvisibleLevel)
	.def("GetInvisibleUndeadLevel", (uint8(Lua_Mob::*)(void))&Lua_Mob::GetInvisibleUndeadLevel)
	.def("GetInvul", (bool(Lua_Mob::*)(void))&Lua_Mob::GetInvul)
	.def("GetItemBonuses", &Lua_Mob::GetItemBonuses)
	.def("GetItemHPBonuses", &Lua_Mob::GetItemHPBonuses)
	.def("GetItemStat", &Lua_Mob::GetItemStat)
	.def("GetLastName", &Lua_Mob::GetLastName)
	.def("GetLevel", &Lua_Mob::GetLevel)
	.def("GetLevelCon", (uint32(Lua_Mob::*)(int))&Lua_Mob::GetLevelCon)
	.def("GetLevelCon", (uint32(Lua_Mob::*)(int,int))&Lua_Mob::GetLevelCon)
	.def("GetLuclinFace", &Lua_Mob::GetLuclinFace)
	.def("GetMR", &Lua_Mob::GetMR)
	.def("GetMana", &Lua_Mob::GetMana)
	.def("GetManaRatio", &Lua_Mob::GetManaRatio)
	.def("GetMaxAGI", &Lua_Mob::GetMaxAGI)
	.def("GetMaxCHA", &Lua_Mob::GetMaxCHA)
	.def("GetMaxDEX", &Lua_Mob::GetMaxDEX)
	.def("GetMaxHP", &Lua_Mob::GetMaxHP)
	.def("GetMaxINT", &Lua_Mob::GetMaxINT)
	.def("GetMaxMana", &Lua_Mob::GetMaxMana)
	.def("GetMaxSTA", &Lua_Mob::GetMaxSTA)
	.def("GetMaxSTR", &Lua_Mob::GetMaxSTR)
	.def("GetMaxWIS", &Lua_Mob::GetMaxWIS)
	.def("GetMeleeDamageMod_SE", &Lua_Mob::GetMeleeDamageMod_SE)
	.def("GetMeleeMinDamageMod_SE", &Lua_Mob::GetMeleeMinDamageMod_SE)
	.def("GetMeleeMitigation", (int32(Lua_Mob::*)(void))&Lua_Mob::GetMeleeMitigation)
	.def("GetMobTypeIdentifier", (uint32(Lua_Mob::*)(void))&Lua_Mob::GetMobTypeIdentifier)
	.def("GetModSkillDmgTaken", (int(Lua_Mob::*)(int))&Lua_Mob::GetModSkillDmgTaken)
	.def("GetModVulnerability", (int(Lua_Mob::*)(int))&Lua_Mob::GetModVulnerability)
	.def("GetNPCTypeID", &Lua_Mob::GetNPCTypeID)
	.def("GetName", &Lua_Mob::GetName)
	.def("GetNimbusEffect1", (uint8(Lua_Mob::*)(void))&Lua_Mob::GetNimbusEffect1)
	.def("GetNimbusEffect2", (uint8(Lua_Mob::*)(void))&Lua_Mob::GetNimbusEffect2)
	.def("GetNimbusEffect3", (uint8(Lua_Mob::*)(void))&Lua_Mob::GetNimbusEffect3)
	.def("GetOrigBodyType", &Lua_Mob::GetOrigBodyType)
	.def("GetOwner", &Lua_Mob::GetOwner)
	.def("GetOwnerID", &Lua_Mob::GetOwnerID)
	.def("GetPR", &Lua_Mob::GetPR)
	.def("GetPausedTimers", &Lua_Mob::GetPausedTimers)
	.def("GetPet", &Lua_Mob::GetPet)
	.def("GetPetOrder", (uint8(Lua_Mob::*)(void))&Lua_Mob::GetPetOrder)
	.def("GetPetType", &Lua_Mob::GetPetType)
	.def("GetPetTypeName", &Lua_Mob::GetPetTypeName)
	.def("GetPhR", &Lua_Mob::GetPhR)
	.def("GetRace", &Lua_Mob::GetRace)
	.def("GetRaceName", &Lua_Mob::GetRaceName)
	.def("GetRacePlural", &Lua_Mob::GetRacePlural)
	.def("GetRemainingTimeMS", &Lua_Mob::GetRemainingTimeMS)
	.def("GetResist", (int(Lua_Mob::*)(int))&Lua_Mob::GetResist)
	.def("GetReverseFactionCon", (int(Lua_Mob::*)(Lua_Mob))&Lua_Mob::GetReverseFactionCon)
	.def("GetRunspeed", &Lua_Mob::GetRunspeed)
	.def("GetSTA", &Lua_Mob::GetSTA)
	.def("GetSTR", &Lua_Mob::GetSTR)
	.def("GetShuffledHateList", &Lua_Mob::GetShuffledHateList)
	.def("GetSize", &Lua_Mob::GetSize)
	.def("GetSkill", (int(Lua_Mob::*)(int))&Lua_Mob::GetSkill)
	.def("GetSkillDmgAmt", (int(Lua_Mob::*)(int))&Lua_Mob::GetSkillDmgAmt)
	.def("GetSkillDmgTaken", (int(Lua_Mob::*)(int))&Lua_Mob::GetSkillDmgTaken)
	.def("GetSpecialAbility", (int(Lua_Mob::*)(int))&Lua_Mob::GetSpecialAbility)
	.def("GetSpecialAbilityParam", (int(Lua_Mob::*)(int,int))&Lua_Mob::GetSpecialAbilityParam)
	.def("GetSpecializeSkillValue", &Lua_Mob::GetSpecializeSkillValue)
	.def("GetSpellBonuses", &Lua_Mob::GetSpellBonuses)
	.def("GetSpellHPBonuses", &Lua_Mob::GetSpellHPBonuses)
	.def("GetTarget", &Lua_Mob::GetTarget)
	.def("GetTexture", &Lua_Mob::GetTexture)
	.def("GetTimerDurationMS", &Lua_Mob::GetTimerDurationMS)
	.def("GetTimers", &Lua_Mob::GetTimers)
	.def("GetUltimateOwner", &Lua_Mob::GetUltimateOwner)
	.def("GetWIS", &Lua_Mob::GetWIS)
	.def("GetWalkspeed", &Lua_Mob::GetWalkspeed)
	.def("GetWaypointH", &Lua_Mob::GetWaypointH)
	.def("GetWaypointID", &Lua_Mob::GetWaypointID)
	.def("GetWaypointPause", &Lua_Mob::GetWaypointPause)
	.def("GetWaypointX", &Lua_Mob::GetWaypointX)
	.def("GetWaypointY", &Lua_Mob::GetWaypointY)
	.def("GetWaypointZ", &Lua_Mob::GetWaypointZ)
	.def("GetWeaponDamage", &Lua_Mob::GetWeaponDamage)
	.def("GetWeaponDamageBonus", &Lua_Mob::GetWeaponDamageBonus)
	.def("GetX", &Lua_Mob::GetX)
	.def("GetY", &Lua_Mob::GetY)
	.def("GetZ", &Lua_Mob::GetZ)
	.def("GotoBind", &Lua_Mob::GotoBind)
	.def("HalveAggro", &Lua_Mob::HalveAggro)
	.def("HasNPCSpecialAtk", (bool(Lua_Mob::*)(const char*))&Lua_Mob::HasNPCSpecialAtk)
	.def("HasOwner", (bool(Lua_Mob::*)(void))&Lua_Mob::HasOwner)
	.def("HasPet", (bool(Lua_Mob::*)(void))&Lua_Mob::HasPet)
	.def("HasProcs", &Lua_Mob::HasProcs)
	.def("HasShieldEquipped", (bool(Lua_Mob::*)(void))&Lua_Mob::HasShieldEquipped)
	.def("HasSpellEffect", &Lua_Mob::HasSpellEffect)
	.def("HasTimer", &Lua_Mob::HasTimer)
	.def("HasTwoHandBluntEquipped", (bool(Lua_Mob::*)(void))&Lua_Mob::HasTwoHandBluntEquipped)
	.def("HasTwoHanderEquipped", (bool(Lua_Mob::*)(void))&Lua_Mob::HasTwoHanderEquipped)
	.def("Heal", &Lua_Mob::RestoreHealth)
	.def("HealDamage", (void(Lua_Mob::*)(uint64))&Lua_Mob::HealDamage)
	.def("HealDamage", (void(Lua_Mob::*)(uint64,Lua_Mob))&Lua_Mob::HealDamage)
	.def("InterruptSpell", (void(Lua_Mob::*)(int))&Lua_Mob::InterruptSpell)
	.def("InterruptSpell", (void(Lua_Mob::*)(void))&Lua_Mob::InterruptSpell)
	.def("IsAIControlled", (bool(Lua_Mob::*)(void))&Lua_Mob::IsAIControlled)
	.def("IsAlwaysAggro", &Lua_Mob::IsAlwaysAggro)
	.def("IsAmnesiad", (bool(Lua_Mob::*)(void))&Lua_Mob::IsAmnesiad)
	.def("IsAnimation", &Lua_Mob::IsAnimation)
	.def("IsAttackAllowed", (bool(Lua_Mob::*)(Lua_Mob))&Lua_Mob::IsAttackAllowed)
	.def("IsAttackAllowed", (bool(Lua_Mob::*)(Lua_Mob,bool))&Lua_Mob::IsAttackAllowed)
	.def("IsBeneficialAllowed", (bool(Lua_Mob::*)(Lua_Mob))&Lua_Mob::IsBeneficialAllowed)
	.def("IsBerserk", &Lua_Mob::IsBerserk)
	.def("IsBlind", (bool(Lua_Mob::*)(void))&Lua_Mob::IsBlind)
	.def("IsBoat", &Lua_Mob::IsBoat)
	.def("IsCasting", &Lua_Mob::IsCasting)
	.def("IsCharmed", &Lua_Mob::IsCharmed)
	.def("IsControllableBoat", &Lua_Mob::IsControllableBoat)
	.def("IsDestructibleObject", &Lua_Mob::IsDestructibleObject)
	.def("IsEliteMaterialItem", (uint32(Lua_Mob::*)(uint8))&Lua_Mob::IsEliteMaterialItem)
	.def("IsEngaged", (bool(Lua_Mob::*)(void))&Lua_Mob::IsEngaged)
	.def("IsEnraged", (bool(Lua_Mob::*)(void))&Lua_Mob::IsEnraged)
	.def("IsFamiliar", &Lua_Mob::IsFamiliar)
	.def("IsFeared", (bool(Lua_Mob::*)(void))&Lua_Mob::IsFeared)
	.def("IsFindable", (bool(Lua_Mob::*)(void))&Lua_Mob::IsFindable)
	.def("IsHorse", &Lua_Mob::IsHorse)
	.def("IsImmuneToSpell", (bool(Lua_Mob::*)(int,Lua_Mob))&Lua_Mob::IsImmuneToSpell)
	.def("IsIntelligenceCasterClass", &Lua_Mob::IsIntelligenceCasterClass)
	.def("IsInvisible", (bool(Lua_Mob::*)(Lua_Mob))&Lua_Mob::IsInvisible)
	.def("IsInvisible", (bool(Lua_Mob::*)(void))&Lua_Mob::IsInvisible)
	.def("IsMeleeDisabled", (bool(Lua_Mob::*)(void))&Lua_Mob::IsMeleeDisabled)
	.def("IsMezzed", (bool(Lua_Mob::*)(void))&Lua_Mob::IsMezzed)
	.def("IsMoving", &Lua_Mob::IsMoving)
	.def("IsPausedTimer", &Lua_Mob::IsPausedTimer)
	.def("IsPet", (bool(Lua_Mob::*)(void))&Lua_Mob::IsPet)
	.def("IsPetOwnerBot", &Lua_Mob::IsPetOwnerBot)
	.def("IsPetOwnerClient", &Lua_Mob::IsPetOwnerClient)
	.def("IsPetOwnerNPC", &Lua_Mob::IsPetOwnerNPC)
	.def("IsPetOwnerOfClientBot", &Lua_Mob::IsPetOwnerOfClientBot)
	.def("IsPureMeleeClass", &Lua_Mob::IsPureMeleeClass)
	.def("IsRoamer", (bool(Lua_Mob::*)(void))&Lua_Mob::IsRoamer)
	.def("IsRooted", (bool(Lua_Mob::*)(void))&Lua_Mob::IsRooted)
	.def("IsRunning", (bool(Lua_Mob::*)(void))&Lua_Mob::IsRunning)
	.def("IsSilenced", (bool(Lua_Mob::*)(void))&Lua_Mob::IsSilenced)
	.def("IsStunned", (bool(Lua_Mob::*)(void))&Lua_Mob::IsStunned)
	.def("IsTargetable", (bool(Lua_Mob::*)(void))&Lua_Mob::IsTargetable)
	.def("IsTargeted", &Lua_Mob::IsTargeted)
	.def("IsTargetLockPet", &Lua_Mob::IsTargetLockPet)
	.def("IsTemporaryPet", &Lua_Mob::IsTemporaryPet)
	.def("IsTrackable", (bool(Lua_Mob::*)(void))&Lua_Mob::IsTrackable)
	.def("IsWarriorClass", &Lua_Mob::IsWarriorClass)
	.def("IsWisdomCasterClass", &Lua_Mob::IsWisdomCasterClass)
	.def("Kill", (void(Lua_Mob::*)(void))&Lua_Mob::Kill)
	.def("MassGroupBuff", (void(Lua_Mob::*)(Lua_Mob, uint16))&Lua_Mob::MassGroupBuff)
	.def("MassGroupBuff", (void(Lua_Mob::*)(Lua_Mob, uint16, bool))&Lua_Mob::MassGroupBuff)
	.def("Mesmerize", (void(Lua_Mob::*)(void))&Lua_Mob::Mesmerize)
	.def("Message", &Lua_Mob::Message)
	.def("MessageString", &Lua_Mob::MessageString)
	.def("Message_StringID", &Lua_Mob::MessageString)
	.def("ModSkillDmgTaken", (void(Lua_Mob::*)(int,int))&Lua_Mob::ModSkillDmgTaken)
	.def("ModVulnerability", (void(Lua_Mob::*)(int,int))&Lua_Mob::ModVulnerability)
	.def("NPCSpecialAttacks", (void(Lua_Mob::*)(const char*,int))&Lua_Mob::NPCSpecialAttacks)
	.def("NPCSpecialAttacks", (void(Lua_Mob::*)(const char*,int,bool))&Lua_Mob::NPCSpecialAttacks)
	.def("NPCSpecialAttacks", (void(Lua_Mob::*)(const char*,int,bool,bool))&Lua_Mob::NPCSpecialAttacks)
	.def("NavigateTo", (void(Lua_Mob::*)(double,double,double))&Lua_Mob::NavigateTo)
	.def("PauseTimer", &Lua_Mob::PauseTimer)
	.def("ProcessSpecialAbilities", (void(Lua_Mob::*)(std::string))&Lua_Mob::ProcessSpecialAbilities)
	.def("ProjectileAnimation", (void(Lua_Mob::*)(Lua_Mob,int))&Lua_Mob::ProjectileAnimation)
	.def("ProjectileAnimation", (void(Lua_Mob::*)(Lua_Mob,int,bool))&Lua_Mob::ProjectileAnimation)
	.def("ProjectileAnimation", (void(Lua_Mob::*)(Lua_Mob,int,bool,double))&Lua_Mob::ProjectileAnimation)
	.def("ProjectileAnimation", (void(Lua_Mob::*)(Lua_Mob,int,bool,double,double))&Lua_Mob::ProjectileAnimation)
	.def("ProjectileAnimation", (void(Lua_Mob::*)(Lua_Mob,int,bool,double,double,double))&Lua_Mob::ProjectileAnimation)
	.def("ProjectileAnimation", (void(Lua_Mob::*)(Lua_Mob,int,bool,double,double,double,double))&Lua_Mob::ProjectileAnimation)
	.def("QuestSay", (void(Lua_Mob::*)(Lua_Client,const char *))&Lua_Mob::QuestSay)
	.def("QuestSay", (void(Lua_Mob::*)(Lua_Client,const char *,luabind::adl::object))&Lua_Mob::QuestSay)
	.def("RandomizeFeatures", (bool(Lua_Mob::*)(void))&Lua_Mob::RandomizeFeatures)
	.def("RandomizeFeatures", (bool(Lua_Mob::*)(bool))&Lua_Mob::RandomizeFeatures)
	.def("RandomizeFeatures", (bool(Lua_Mob::*)(bool,bool))&Lua_Mob::RandomizeFeatures)
	.def("RangedAttack", &Lua_Mob::RangedAttack)
	.def("RemoveAllNimbusEffects", &Lua_Mob::RemoveAllNimbusEffects)
	.def("RemoveNimbusEffect", (void(Lua_Mob::*)(int))&Lua_Mob::RemoveNimbusEffect)
	.def("RemovePet", &Lua_Mob::RemovePet)
	.def("ResistSpell", (double(Lua_Mob::*)(int,int,Lua_Mob))&Lua_Mob::ResistSpell)
	.def("ResistSpell", (double(Lua_Mob::*)(int,int,Lua_Mob,bool))&Lua_Mob::ResistSpell)
	.def("ResistSpell", (double(Lua_Mob::*)(int,int,Lua_Mob,bool,int))&Lua_Mob::ResistSpell)
	.def("ResistSpell", (double(Lua_Mob::*)(int,int,Lua_Mob,bool,int,bool))&Lua_Mob::ResistSpell)
	.def("RestoreEndurance", &Lua_Mob::RestoreEndurance)
	.def("RestoreHealth", &Lua_Mob::RestoreHealth)
	.def("RestoreMana", &Lua_Mob::RestoreMana)
	.def("ResumeTimer", &Lua_Mob::ResumeTimer)
	.def("RunTo", (void(Lua_Mob::*)(double, double, double))&Lua_Mob::RunTo)
	.def("Say", (void(Lua_Mob::*)(const char*))& Lua_Mob::Say)
	.def("Say", (void(Lua_Mob::*)(const char*, uint8))& Lua_Mob::Say)
	.def("SeeHide", (bool(Lua_Mob::*)(void))&Lua_Mob::SeeHide)
	.def("SeeImprovedHide", (bool(Lua_Mob::*)(bool))&Lua_Mob::SeeImprovedHide)
	.def("SeeInvisible", (uint8(Lua_Mob::*)(void))&Lua_Mob::SeeInvisible)
	.def("SeeInvisibleUndead", (uint8(Lua_Mob::*)(void))&Lua_Mob::SeeInvisibleUndead)
	.def("SetSeeInvisibleLevel", (void(Lua_Mob::*)(uint8))&Lua_Mob::SetSeeInvisibleLevel)
	.def("SetSeeInvisibleUndeadLevel", (void(Lua_Mob::*)(uint8))&Lua_Mob::SetSeeInvisibleUndeadLevel)
	.def("SendAppearanceEffect", (void(Lua_Mob::*)(uint32,uint32,uint32,uint32,uint32))&Lua_Mob::SendAppearanceEffect)
	.def("SendAppearanceEffect", (void(Lua_Mob::*)(uint32,uint32,uint32,uint32,uint32,Lua_Client))&Lua_Mob::SendAppearanceEffect)
	.def("SendWearChange", (void(Lua_Mob::*)(uint8))&Lua_Mob::SendWearChange)
	.def("SendBeginCast", &Lua_Mob::SendBeginCast)
	.def("SendIllusionPacket", (void(Lua_Mob::*)(luabind::adl::object))&Lua_Mob::SendIllusionPacket)
	.def("SendPayload", (void(Lua_Mob::*)(int))&Lua_Mob::SendPayload)
	.def("SendPayload", (void(Lua_Mob::*)(int,std::string))&Lua_Mob::SendPayload)
	.def("SendSpellEffect", (void(Lua_Mob::*)(uint32,uint32,uint32,bool,uint32))&Lua_Mob::SendSpellEffect)
	.def("SendSpellEffect", (void(Lua_Mob::*)(uint32,uint32,uint32,bool,uint32,bool))&Lua_Mob::SendSpellEffect)
	.def("SendSpellEffect", (void(Lua_Mob::*)(uint32,uint32,uint32,bool,uint32,bool,Lua_Client))&Lua_Mob::SendSpellEffect)
	.def("SendTo", (void(Lua_Mob::*)(double,double,double))&Lua_Mob::SendTo)
	.def("SendToFixZ", (void(Lua_Mob::*)(double,double,double))&Lua_Mob::SendToFixZ)
	.def("SetAA", (bool(Lua_Mob::*)(int,int))&Lua_Mob::SetAA)
	.def("SetAA", (bool(Lua_Mob::*)(int,int,int))&Lua_Mob::SetAA)
	.def("SetAllowBeneficial", (void(Lua_Mob::*)(bool))&Lua_Mob::SetAllowBeneficial)
	.def("SetAppearance", (void(Lua_Mob::*)(int))&Lua_Mob::SetAppearance)
	.def("SetAppearance", (void(Lua_Mob::*)(int,bool))&Lua_Mob::SetAppearance)
	.def("SetBodyType", (void(Lua_Mob::*)(uint8,bool))&Lua_Mob::SetBodyType)
	.def("SetBucket", (void(Lua_Mob::*)(std::string,std::string))&Lua_Mob::SetBucket)
	.def("SetBucket", (void(Lua_Mob::*)(std::string,std::string,std::string))&Lua_Mob::SetBucket)
	.def("SetBuffDuration", (void(Lua_Mob::*)(int))&Lua_Mob::SetBuffDuration)
	.def("SetBuffDuration", (void(Lua_Mob::*)(int,int))&Lua_Mob::SetBuffDuration)
	.def("SetBuffDuration", (void(Lua_Mob::*)(int,int,int))&Lua_Mob::SetBuffDuration)
	.def("SetCurrentWP", &Lua_Mob::SetCurrentWP)
	.def("SetDestructibleObject", (void(Lua_Mob::*)(bool))&Lua_Mob::SetDestructibleObject)
	.def("SetDisableMelee", (void(Lua_Mob::*)(bool))&Lua_Mob::SetDisableMelee)
	.def("SetEntityVariable", &Lua_Mob::SetEntityVariable)
	.def("SetExtraHaste", (void(Lua_Mob::*)(int))&Lua_Mob::SetExtraHaste)
	.def("SetExtraHaste", (void(Lua_Mob::*)(int,bool))&Lua_Mob::SetExtraHaste)
	.def("SetFlurryChance", (void(Lua_Mob::*)(int))&Lua_Mob::SetFlurryChance)
	.def("SetFlyMode", (void(Lua_Mob::*)(int))&Lua_Mob::SetFlyMode)
	.def("SetGender", (void(Lua_Mob::*)(uint8))&Lua_Mob::SetGender)
	.def("SetGlobal", (void(Lua_Mob::*)(const char*,const char*,int,const char*))&Lua_Mob::SetGlobal)
	.def("SetGlobal", (void(Lua_Mob::*)(const char*,const char*,int,const char*,Lua_Mob))&Lua_Mob::SetGlobal)
	.def("SetHP", &Lua_Mob::SetHP)
	.def("SetHate", (void(Lua_Mob::*)(Lua_Mob))&Lua_Mob::SetHate)
	.def("SetHate", (void(Lua_Mob::*)(Lua_Mob,int64))&Lua_Mob::SetHate)
	.def("SetHate", (void(Lua_Mob::*)(Lua_Mob,int64,int64))&Lua_Mob::SetHate)
	.def("SetHeading", (void(Lua_Mob::*)(double))&Lua_Mob::SetHeading)
	.def("SetInvisible", &Lua_Mob::SetInvisible)
	.def("SetInvul", (void(Lua_Mob::*)(bool))&Lua_Mob::SetInvul)
	.def("SetLevel", (void(Lua_Mob::*)(int))&Lua_Mob::SetLevel)
	.def("SetLevel", (void(Lua_Mob::*)(int,bool))&Lua_Mob::SetLevel)
	.def("SetMana", &Lua_Mob::SetMana)
	.def("SetOOCRegen", (void(Lua_Mob::*)(int64))&Lua_Mob::SetOOCRegen)
	.def("SetPet", &Lua_Mob::SetPet)
	.def("SetPetOrder", (void(Lua_Mob::*)(uint8))&Lua_Mob::SetPetOrder)
	.def("SetPetType", &Lua_Mob::SetPetType)
	.def("SetPseudoRoot", (void(Lua_Mob::*)(bool))&Lua_Mob::SetPseudoRoot)
	.def("SetRace", (void(Lua_Mob::*)(uint16))&Lua_Mob::SetRace)
	.def("SetRunning", (void(Lua_Mob::*)(bool))&Lua_Mob::SetRunning)
	.def("SetSlotTint", (void(Lua_Mob::*)(int,int,int,int))&Lua_Mob::SetSlotTint)
	.def("SetSpecialAbility", (void(Lua_Mob::*)(int,int))&Lua_Mob::SetSpecialAbility)
	.def("SetSpecialAbilityParam", (void(Lua_Mob::*)(int,int,int))&Lua_Mob::SetSpecialAbilityParam)
	.def("SetTarget", &Lua_Mob::SetTarget)
	.def("SetTargetable", (void(Lua_Mob::*)(bool))&Lua_Mob::SetTargetable)
	.def("SetTexture", (void(Lua_Mob::*)(uint8))&Lua_Mob::SetTexture)
	.def("SetTimer", &Lua_Mob::SetTimer)
	.def("SetTimerMS", &Lua_Mob::SetTimerMS)
	.def("StopAllTimers", &Lua_Mob::StopAllTimers)
	.def("StopTimer", &Lua_Mob::StopTimer)
	.def("Shout", (void(Lua_Mob::*)(const char*))& Lua_Mob::Shout)
	.def("Shout", (void(Lua_Mob::*)(const char*, uint8))& Lua_Mob::Shout)
	.def("Signal", (void(Lua_Mob::*)(int))&Lua_Mob::Signal)
	.def("SpellEffect", &Lua_Mob::SpellEffect)
	.def("SpellFinished", (bool(Lua_Mob::*)(int,Lua_Mob))&Lua_Mob::SpellFinished)
	.def("SpellFinished", (bool(Lua_Mob::*)(int,Lua_Mob,int))&Lua_Mob::SpellFinished)
	.def("SpellFinished", (bool(Lua_Mob::*)(int,Lua_Mob,int,int))&Lua_Mob::SpellFinished)
	.def("SpellFinished", (bool(Lua_Mob::*)(int,Lua_Mob,int,int,uint32))&Lua_Mob::SpellFinished)
	.def("SpellFinished", (bool(Lua_Mob::*)(int,Lua_Mob,int,int,uint32,int))&Lua_Mob::SpellFinished)
	.def("SpellFinished", (bool(Lua_Mob::*)(int,Lua_Mob,int,int,uint32,int,bool))&Lua_Mob::SpellFinished)
	.def("Spin", (void(Lua_Mob::*)(void))&Lua_Mob::Spin)
	.def("StopNavigation", (void(Lua_Mob::*)(void))&Lua_Mob::StopNavigation)
	.def("Stun", (void(Lua_Mob::*)(int))&Lua_Mob::Stun)
	.def("TarGlobal", (void(Lua_Mob::*)(const char*,const char*,const char*,int,int,int))&Lua_Mob::TarGlobal)
	.def("TempName", (void(Lua_Mob::*)(const char*))&Lua_Mob::TempName)
	.def("TempName", (void(Lua_Mob::*)(void))&Lua_Mob::TempName)
	.def("ThrowingAttack", &Lua_Mob::ThrowingAttack)
	.def("TryFinishingBlow", &Lua_Mob::TryFinishingBlow)
	.def("TryMoveAlong", (void(Lua_Mob::*)(float,float))&Lua_Mob::TryMoveAlong)
	.def("TryMoveAlong", (void(Lua_Mob::*)(float,float,bool))&Lua_Mob::TryMoveAlong)
	.def("UnStun", (void(Lua_Mob::*)(void))&Lua_Mob::UnStun)
	.def("WalkTo", (void(Lua_Mob::*)(double, double, double))&Lua_Mob::WalkTo)
	.def("WearChange", (void(Lua_Mob::*)(uint8,uint32))&Lua_Mob::WearChange)
	.def("WearChange", (void(Lua_Mob::*)(uint8,uint32,uint32))&Lua_Mob::WearChange)
	.def("WearChange", (void(Lua_Mob::*)(uint8,uint32,uint32,uint32))&Lua_Mob::WearChange)
	.def("WipeHateList", (void(Lua_Mob::*)(void))&Lua_Mob::WipeHateList);
}

luabind::scope lua_register_special_abilities() {
	return luabind::class_<SpecialAbilities>("SpecialAbility")
	.enum_("constants")
	[(
		luabind::value("summon", SpecialAbility::Summon),
		luabind::value("enrage", SpecialAbility::Enrage),
		luabind::value("rampage", SpecialAbility::Rampage),
		luabind::value("area_rampage", SpecialAbility::AreaRampage),
		luabind::value("flurry", SpecialAbility::Flurry),
		luabind::value("triple_attack", SpecialAbility::TripleAttack),
		luabind::value("quad_attack", SpecialAbility::QuadrupleAttack),
		luabind::value("innate_dual_wield", SpecialAbility::DualWield),
		luabind::value("bane_attack", SpecialAbility::BaneAttack),
		luabind::value("magical_attack", SpecialAbility::MagicalAttack),
		luabind::value("ranged_attack", SpecialAbility::RangedAttack),
		luabind::value("unslowable", SpecialAbility::SlowImmunity),
		luabind::value("unmezable", SpecialAbility::MesmerizeImmunity),
		luabind::value("uncharmable", SpecialAbility::CharmImmunity),
		luabind::value("unstunable", SpecialAbility::StunImmunity),
		luabind::value("unsnareable", SpecialAbility::SnareImmunity),
		luabind::value("unfearable", SpecialAbility::FearImmunity),
		luabind::value("undispellable", SpecialAbility::DispellImmunity),
		luabind::value("immune_melee", SpecialAbility::MeleeImmunity),
		luabind::value("immune_magic", SpecialAbility::MagicImmunity),
		luabind::value("immune_fleeing", SpecialAbility::FleeingImmunity),
		luabind::value("immune_melee_except_bane", SpecialAbility::MeleeImmunityExceptBane),
		luabind::value("immune_melee_except_magical", SpecialAbility::MeleeImmunityExceptMagical),
		luabind::value("immune_aggro", SpecialAbility::AggroImmunity),
		luabind::value("immune_aggro_on", SpecialAbility::BeingAggroImmunity),
		luabind::value("immune_casting_from_range", SpecialAbility::CastingFromRangeImmunity),
		luabind::value("immune_feign_death", SpecialAbility::FeignDeathImmunity),
		luabind::value("immune_taunt", SpecialAbility::TauntImmunity),
		luabind::value("tunnelvision", SpecialAbility::TunnelVision),
		luabind::value("dont_buff_friends", SpecialAbility::NoBuffHealFriends),
		luabind::value("immune_pacify", SpecialAbility::PacifyImmunity),
		luabind::value("leash", SpecialAbility::Leash),
		luabind::value("tether", SpecialAbility::Tether),
		luabind::value("destructible_object", SpecialAbility::DestructibleObject),
		luabind::value("no_harm_from_client", SpecialAbility::HarmFromClientImmunity),
		luabind::value("always_flee", SpecialAbility::AlwaysFlee),
		luabind::value("flee_percent", SpecialAbility::FleePercent),
		luabind::value("allow_beneficial", SpecialAbility::AllowBeneficial),
		luabind::value("disable_melee", SpecialAbility::DisableMelee),
		luabind::value("npc_chase_distance", SpecialAbility::NPCChaseDistance),
		luabind::value("allow_to_tank", SpecialAbility::AllowedToTank),
		luabind::value("ignore_root_aggro_rules", SpecialAbility::IgnoreRootAggroRules),
		luabind::value("casting_resist_diff", SpecialAbility::CastingResistDifficulty),
		luabind::value("counter_avoid_damage", SpecialAbility::CounterAvoidDamage),
		luabind::value("immune_ranged_attacks", SpecialAbility::RangedAttackImmunity),
		luabind::value("immune_damage_client", SpecialAbility::ClientDamageImmunity),
		luabind::value("immune_damage_npc", SpecialAbility::NPCDamageImmunity),
		luabind::value("immune_aggro_client", SpecialAbility::ClientAggroImmunity),
		luabind::value("immune_aggro_npc", SpecialAbility::NPCAggroImmunity),
		luabind::value("modify_avoid_damage", SpecialAbility::ModifyAvoidDamage),
		luabind::value("immune_open", SpecialAbility::OpenImmunity),
		luabind::value("immune_assassinate", SpecialAbility::AssassinateImmunity),
		luabind::value("immune_headshot", SpecialAbility::HeadshotImmunity),
		luabind::value("immune_aggro_bot", SpecialAbility::BotAggroImmunity),
		luabind::value("immune_damage_bot", SpecialAbility::BotDamageImmunity)
	)];
}

#endif
