#include "common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "common/data_verification.h"
#include "zone/client.h"
#include "zone/embperl.h"
#include "zone/raids.h"

bool Perl_Raid_IsRaidMember(Raid* self, const char* name) // @categories Raid
{
	return self->IsRaidMember(name);
}

bool Perl_Raid_IsRaidMember(Raid* self, Client* c) // @categories Raid
{
	return self->IsRaidMember(c);
}

void Perl_Raid_CastGroupSpell(Raid* self, Mob* caster, uint16 spell_id, uint32 group_id) // @categories Group, Raid
{
	self->CastGroupSpell(caster, spell_id, group_id);
}

int Perl_Raid_GroupCount(Raid* self, uint32_t group_id) // @categories Group, Raid
{
	return self->GroupCount(group_id);
}

int Perl_Raid_RaidCount(Raid* self) // @categories Raid
{
	return self->RaidCount();
}

uint32_t Perl_Raid_GetGroup(Raid* self, const char* name) // @categories Group, Raid
{
	return self->GetGroup(name);
}

uint32_t Perl_Raid_GetGroup(Raid* self, Client* client) // @categories Group, Raid
{
	return self->GetGroup(client);
}

void Perl_Raid_SplitExp(Raid* self, uint32 experience, Mob* other) // @categories Experience and Level, Raid
{
	self->SplitExp(ExpSource::Quest, experience, other);
}

uint32_t Perl_Raid_GetTotalRaidDamage(Raid* self, Mob* other) // @categories Raid
{
	return self->GetTotalRaidDamage(other);
}

void Perl_Raid_SplitMoney(Raid* self, uint32 gid, uint32 copper, uint32 silver, uint32 gold, uint32 platinum) // @categories Currency and Points, Raid
{
	self->SplitMoney(gid, copper, silver, gold, platinum);
}

void Perl_Raid_SplitMoney(Raid* self, uint32 gid, uint32 copper, uint32 silver, uint32 gold, uint32 platinum, Client* splitter) // @categories Currency and Points, Raid
{
	self->SplitMoney(gid, copper, silver, gold, platinum, splitter);
}

void Perl_Raid_BalanceHP(Raid* self, int32_t penalty, uint32_t group_id) // @categories Raid
{
	self->BalanceHP(penalty, group_id);
}

bool Perl_Raid_IsLeader(Raid* self, const char* name) // @categories Raid
{
	return self->IsLeader(name);
}

bool Perl_Raid_IsLeader(Raid* self, Client* c) // @categories Raid
{
	return self->IsLeader(c);
}

Client* Perl_Raid_GetLeader(Raid* self) // @categories Raid
{
	return self->GetLeader();
}

std::string Perl_Raid_GetLeaderName(Raid* self) // @categories Raid
{
	return self->GetLeaderName();
}

bool Perl_Raid_IsGroupLeader(Raid* self, const char* who) // @categories Group, Raid
{
	return self->IsGroupLeader(who);
}

bool Perl_Raid_IsGroupLeader(Raid* self, Client* c) // @categories Group, Raid
{
	return self->IsGroupLeader(c);
}

uint32_t Perl_Raid_GetHighestLevel(Raid* self) // @categories Raid
{
	return self->GetHighestLevel();
}

uint32_t Perl_Raid_GetLowestLevel(Raid* self) // @categories Raid
{
	return self->GetLowestLevel();
}

Client* Perl_Raid_GetClientByIndex(Raid* self, uint16_t member_index) // @categories Raid
{
	return self->GetClientByIndex(member_index);
}

void Perl_Raid_TeleportGroup(Raid* self, Mob* sender, uint32 zone_id, float x, float y, float z, float heading, uint32 group_id) // @categories Group, Raid
{
	self->TeleportGroup(sender, zone_id, 0, x, y, z, heading, group_id);
}

void Perl_Raid_TeleportRaid(Raid* self, Mob* sender, uint32 zone_id, float x, float y, float z, float heading) // @categories Raid
{
	self->TeleportRaid(sender, zone_id, 0, x, y, z, heading);
}

uint32_t Perl_Raid_GetID(Raid* self) // @categories Raid
{
	return self->GetID();
}

Client* Perl_Raid_GetMember(Raid* self, int member_index) // @categories Raid
{
	if (!EQ::ValueWithin(member_index, 0, (MAX_RAID_MEMBERS - 1))) {
		return nullptr;
	}

	return self->members[member_index].member;
}

bool Perl_Raid_DoesAnyMemberHaveExpeditionLockout(Raid* self, std::string expedition_name, std::string event_name)
{
	return self->AnyMemberHasDzLockout(expedition_name, event_name);
}

bool Perl_Raid_DoesAnyMemberHaveExpeditionLockout(Raid* self, std::string expedition_name, std::string event_name, int max_check_count)
{
	return self->AnyMemberHasDzLockout(expedition_name, event_name); // max_check_count deprecated
}

int Perl_Raid_GetGroupNumber(Raid* self, int member_index) {
	if (
		!EQ::ValueWithin(member_index, 0, 71) ||
		self->members[member_index].group_number == RAID_GROUPLESS
	) {
		return -1;
	}

	return self->members[member_index].group_number;
}

void perl_register_raid()
{
	perl::interpreter perl(PERL_GET_THX);

	auto package = perl.new_class<Raid>("Raid");
	package.add("BalanceHP", &Perl_Raid_BalanceHP);
	package.add("CastGroupSpell", &Perl_Raid_CastGroupSpell);
	package.add("DoesAnyMemberHaveExpeditionLockout", (bool(*)(Raid*, std::string, std::string))&Perl_Raid_DoesAnyMemberHaveExpeditionLockout);
	package.add("DoesAnyMemberHaveExpeditionLockout", (bool(*)(Raid*, std::string, std::string, int))&Perl_Raid_DoesAnyMemberHaveExpeditionLockout);
	package.add("GetClientByIndex", &Perl_Raid_GetClientByIndex);
	package.add("GetGroup", (uint32(*)(Raid*, const char*))&Perl_Raid_GetGroup);
	package.add("GetGroup", (uint32(*)(Raid*, Client*))&Perl_Raid_GetGroup);
	package.add("GetGroupNumber", &Perl_Raid_GetGroupNumber);
	package.add("GetHighestLevel", &Perl_Raid_GetHighestLevel);
	package.add("GetID", &Perl_Raid_GetID);
	package.add("GetLowestLevel", &Perl_Raid_GetLowestLevel);
	package.add("GetMember", &Perl_Raid_GetMember);
	package.add("GetLeader", &Perl_Raid_GetLeader);
	package.add("GetLeaderName", &Perl_Raid_GetLeaderName);
	package.add("GetTotalRaidDamage", &Perl_Raid_GetTotalRaidDamage);
	package.add("GroupCount", &Perl_Raid_GroupCount);
	package.add("IsGroupLeader", (bool(*)(Raid*, const char*))&Perl_Raid_IsGroupLeader);
	package.add("IsGroupLeader", (bool(*)(Raid*, Client*))&Perl_Raid_IsGroupLeader);
	package.add("IsLeader", (bool(*)(Raid*, const char*))&Perl_Raid_IsLeader);
	package.add("IsLeader", (bool(*)(Raid*, Client*))&Perl_Raid_IsLeader);
	package.add("IsRaidMember", (bool(*)(Raid*, const char*))&Perl_Raid_IsRaidMember);
	package.add("IsRaidMember", (bool(*)(Raid*, Client*))&Perl_Raid_IsRaidMember);
	package.add("RaidCount", &Perl_Raid_RaidCount);
	package.add("SplitExp", &Perl_Raid_SplitExp);
	package.add("SplitMoney", (void(*)(Raid*, uint32, uint32, uint32, uint32, uint32))&Perl_Raid_SplitMoney);
	package.add("SplitMoney", (void(*)(Raid*, uint32, uint32, uint32, uint32, uint32, Client*))&Perl_Raid_SplitMoney);
	package.add("TeleportGroup", &Perl_Raid_TeleportGroup);
	package.add("TeleportRaid", &Perl_Raid_TeleportRaid);
}

#endif //EMBPERL_XS_CLASSES

