#include "common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "common/data_verification.h"
#include "zone/embperl.h"
#include "zone/groups.h"

void Perl_Group_DisbandGroup(Group* self) // @categories Script Utility, Group
{
	self->DisbandGroup();
}

bool Perl_Group_IsGroupMember(Group* self, Mob* c) // @categories Account and Character, Script Utility, Group
{
	return self->IsGroupMember(c);
}

bool Perl_Group_IsGroupMember(Group* self, const char* name) // @categories Account and Character, Script Utility, Group
{
	return self->IsGroupMember(name);
}

void Perl_Group_CastGroupSpell(Group* self, Mob* caster, uint16 spell_id) // @categories Account and Character, Script Utility, Group
{
	self->CastGroupSpell(caster, spell_id);
}

void Perl_Group_SplitExp(Group* self, uint32_t exp, Mob* other) // @categories Account and Character, Script Utility, Group
{
	self->SplitExp(ExpSource::Quest, exp, other);
}

void Perl_Group_GroupMessage(Group* self, Mob* sender, const char* message) // @categories Script Utility, Group
{
	// if no language is specificed, send it in common
	self->GroupMessage(sender, Language::CommonTongue, Language::MaxValue, message);
}

void Perl_Group_GroupMessage(Group* self, Mob* sender, uint8_t language, const char* message) // @categories Script Utility, Group
{
	if (!EQ::ValueWithin(language, Language::CommonTongue, Language::Unknown27)) {
		language = Language::CommonTongue;
	}

	self->GroupMessage(sender, language, Language::MaxValue, message);
}

uint32_t Perl_Group_GetTotalGroupDamage(Group* self, Mob* other) // @categories Script Utility, Group
{
	return self->GetTotalGroupDamage(other);
}

void Perl_Group_SplitMoney(Group* self, uint32 copper, uint32 silver, uint32 gold, uint32 platinum) // @categories Currency and Points, Script Utility, Group
{
	self->SplitMoney(copper, silver, gold, platinum);
}

void Perl_Group_SplitMoney(Group* self, uint32 copper, uint32 silver, uint32 gold, uint32 platinum, Client* splitter) // @categories Currency and Points, Script Utility, Group
{
	self->SplitMoney(copper, silver, gold, platinum, splitter);
}

void Perl_Group_SetLeader(Group* self, Mob* new_leader) // @categories Account and Character, Script Utility, Group
{
	self->SetLeader(new_leader);
}

Mob* Perl_Group_GetLeader(Group* self) // @categories Account and Character, Script Utility, Group
{
	return self->GetLeader();
}

std::string Perl_Group_GetLeaderName(Group* self) // @categories Account and Character, Script Utility, Group
{
	return self->GetLeaderName();
}

void Perl_Group_SendHPPacketsTo(Group* self, Mob* new_member) // @categories Script Utility, Group
{
	self->SendHPManaEndPacketsTo(new_member);
}

void Perl_Group_SendHPPacketsFrom(Group* self, Mob* new_member) // @categories Script Utility, Group
{
	self->SendHPPacketsFrom(new_member);
}

bool Perl_Group_IsLeader(Group* self, Mob* c) // @categories Account and Character, Script Utility, Group
{
	return self->IsLeader(c);
}

bool Perl_Group_IsLeader(Group* self, const char* name) // @categories Account and Character, Script Utility, Group
{
	return self->IsLeader(name);
}

int Perl_Group_GroupCount(Group* self) // @categories Script Utility, Group
{
	return self->GroupCount();
}

uint32_t Perl_Group_GetHighestLevel(Group* self) // @categories Script Utility, Group
{
	return self->GetHighestLevel();
}

void Perl_Group_TeleportGroup(Group* self, Mob* sender, uint32 zone_id, float x, float y, float z, float heading) // @categories Script Utility, Group
{
	self->TeleportGroup(sender, zone_id, 0, x, y, z, heading);
}

uint32_t Perl_Group_GetID(Group* self) // @categories Script Utility, Group
{
	return self->GetID();
}

Client* Perl_Group_GetMember(Group* self, int member_index) // @categories Account and Character, Script Utility, Group
{
	Mob* member = nullptr;
	if (EQ::ValueWithin(member_index, 0, 5)) {
		member = self->members[member_index];
	}

	return member ? member->CastToClient() : nullptr;
}

uint8_t Perl_Group_GetMemberRole(Group* self, Mob* member) // @categories Account and Character, Script Utility, Group
{
	return self->GetMemberRole(member);
}

uint8_t Perl_Group_GetMemberRole(Group* self, const char* name) // @categories Account and Character, Script Utility, Group
{
	return self->GetMemberRole(name);
}

bool Perl_Group_DoesAnyMemberHaveExpeditionLockout(Group* self, std::string expedition_name, std::string event_name)
{
	return self->AnyMemberHasDzLockout(expedition_name, event_name);
}

bool Perl_Group_DoesAnyMemberHaveExpeditionLockout(Group* self, std::string expedition_name, std::string event_name, int max_check_count)
{
	return self->AnyMemberHasDzLockout(expedition_name, event_name); // max_check_count deprecated
}

uint32_t Perl_Group_GetLowestLevel(Group* self) // @categories Script Utility, Group
{
	return self->GetLowestLevel();
}

uint32_t Perl_Group_GetAverageLevel(Group* self) // @categories Script Utility, Group
{
	return self->GetAvgLevel();
}

void perl_register_group()
{
	perl::interpreter perl(PERL_GET_THX);

	auto package = perl.new_class<Group>("Group");
	package.add("CastGroupSpell", &Perl_Group_CastGroupSpell);
	package.add("DisbandGroup", &Perl_Group_DisbandGroup);
	package.add("DoesAnyMemberHaveExpeditionLockout", (bool(*)(Group*, std::string, std::string))&Perl_Group_DoesAnyMemberHaveExpeditionLockout);
	package.add("DoesAnyMemberHaveExpeditionLockout", (bool(*)(Group*, std::string, std::string, int))&Perl_Group_DoesAnyMemberHaveExpeditionLockout);
	package.add("GetAverageLevel", &Perl_Group_GetAverageLevel);
	package.add("GetHighestLevel", &Perl_Group_GetHighestLevel);
	package.add("GetID", &Perl_Group_GetID);
	package.add("GetLeader", &Perl_Group_GetLeader);
	package.add("GetLeaderName", &Perl_Group_GetLeaderName);
	package.add("GetLowestLevel", &Perl_Group_GetLowestLevel);
	package.add("GetMember", &Perl_Group_GetMember);
	package.add("GetMemberRole", (uint8_t(*)(Group*, Mob*))&Perl_Group_GetMemberRole);
	package.add("GetMemberRole", (uint8_t(*)(Group*, const char*))&Perl_Group_GetMemberRole);
	package.add("GetTotalGroupDamage", &Perl_Group_GetTotalGroupDamage);
	package.add("GroupCount", &Perl_Group_GroupCount);
	package.add("GroupMessage", (void(*)(Group*, Mob*, const char*))&Perl_Group_GroupMessage);
	package.add("GroupMessage", (void(*)(Group*, Mob*, uint8_t, const char*))&Perl_Group_GroupMessage);
	package.add("IsGroupMember", (bool(*)(Group*, const char*))&Perl_Group_IsGroupMember);
	package.add("IsGroupMember", (bool(*)(Group*, Mob*))&Perl_Group_IsGroupMember);
	package.add("IsLeader", (bool(*)(Group*, const char*))&Perl_Group_IsLeader);
	package.add("IsLeader", (bool(*)(Group*, Mob*))&Perl_Group_IsLeader);
	package.add("SendHPPacketsFrom", &Perl_Group_SendHPPacketsFrom);
	package.add("SendHPPacketsTo", &Perl_Group_SendHPPacketsTo);
	package.add("SetLeader", &Perl_Group_SetLeader);
	package.add("SplitExp", &Perl_Group_SplitExp);
	package.add("SplitMoney", (void(*)(Group*, uint32, uint32, uint32, uint32))&Perl_Group_SplitMoney);
	package.add("SplitMoney", (void(*)(Group*, uint32, uint32, uint32, uint32, Client*))&Perl_Group_SplitMoney);
	package.add("TeleportGroup", &Perl_Group_TeleportGroup);
}

#endif //EMBPERL_XS_CLASSES
