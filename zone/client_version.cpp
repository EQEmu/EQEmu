/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "client_version.h"

using Version = EQ::versions::ClientVersion;

void Client::SetClientVersion(Version client_version)
{
	m_ClientVersion = client_version;
	m_ClientVersionBit = EQ::versions::ConvertClientVersionToClientVersionBit(client_version);
}

Version Client::GetClientVersion() const { return m_ClientVersion; }

void ClientPatch::InterruptSpell(Client* c, uint32_t message, uint32_t spawn_id, const char* spell_link) {
	QueuePacket(c, &IMessage::InterruptSpell, GetClientComponent<IMessage>(c), message, spawn_id, spell_link);
}

void ClientPatch::InterruptSpellOther(Mob* sender, uint32_t message, uint32_t spawn_id, const char* name,
	const char* spell_link) {
	QueueCloseClients(sender, true, RuleI(Range, SongMessages), nullptr, true,
		sender->IsClient() ? FilterPCSpells : FilterNPCSpells)(
		&IMessage::InterruptSpellOther, GetClientComponent<IMessage>, sender, message, spawn_id, name, spell_link);
}

static bool ShouldSendTargetBuffs(Client* c) {
	// this function checks for server rules against LAA and GM status to determine if a buffs packet should be sent
	// to a client (c) for targeted mobs
	if (c->GetGM() || RuleB(Spells, AlwaysSendTargetsBuffs)) { // this rule bypasses LAA abilities, always return true
		if (c->GetGM()) {
			if (!c->EntityVariableExists(SEE_BUFFS_FLAG)) { // This flag just ensures that the following message is only sent once
				c->Message(Chat::White,
					"Your GM flag allows you to always see your targets' buffs.");
				c->SetEntityVariable(SEE_BUFFS_FLAG, "1");
			}
		}
		return true;
	}

	if (c->IsRaidGrouped()) {
		Raid* raid = c->GetRaid();
		if (raid) {
			uint32 gid = raid->GetGroup(c);
			if (gid < MAX_RAID_GROUPS && raid->GroupCount(gid) >= 3) {
				if (raid->GetLeadershipAA(groupAAInspectBuffs, gid))
					return true;
			}
		}
	} else {
		Group* group = c->GetGroup();
		if (group && group->GroupCount() >= 3) {
			if (group->GetLeadershipAA(groupAAInspectBuffs)) {
				return true;
			}
		}
	}

	return false;
}

static void SetRefreshType(std::unique_ptr<EQApplicationPacket>& packet, Mob* source, Client* target)
{
	uint8_t type = 0;
	if (target->GetID() == source->GetID())
		type = 1;
	else if (source->IsPet())
		type = 2;
	else if (target->HasGroup() && source->GetGroup() == target->GetGroup())
		type = 4;
	else if (source->IsClient())
		type = 5;
	else
		type = 7;

	ClientPatch::GetClientComponent<ClientPatch::IBuff>(target)->SetRefreshType(packet, type);
}

void ClientPatch::SendFullBuffRefresh(Mob* sender, bool remove, bool ackreq) {
	bool suspended = zone->BuffTimersSuspended();
	std::vector<uint32_t> slots;

	// first, send to self if self is client
	if (sender->IsClient()) {
		Client* c = sender->CastToClient();
		FastQueuePacket(c, &IBuff::RefreshBuffs, GetClientComponent<IBuff>(c), OP_RefreshBuffs, sender, false, suspended, slots);
	}

	// next, send to owner if self is a pet to a client
	if (sender->IsPet() && sender->GetOwner()->IsClient()) {
		if (Mob* owner = sender->GetOwner(); owner != nullptr && owner->IsClient()) {
			Client* c = owner->CastToClient();
			FastQueuePacket(c, &IBuff::RefreshBuffs, GetClientComponent<IBuff>(c), OP_RefreshPetBuffs, sender, false, suspended, slots);
		}
	}

	// finally send to all clients targeting the mob, will need to mutate the packet to set the type
	auto mutate = [sender](std::unique_ptr<EQApplicationPacket>& packet, Client* c) {
		SetRefreshType(packet, sender, c);
	};

	QueueClientsByTarget(sender, ackreq, ShouldSendTargetBuffs, mutate)(
		&IBuff::RefreshBuffs, GetClientComponent<IBuff>, OP_RefreshTargetBuffs, sender, false, suspended, slots);

	// if we have remove set, this will clear  any target windows that shouldn't see the buffs
	if (remove)
		QueueClientsByTarget(sender, ackreq, [](Client* c) { return !ShouldSendTargetBuffs(c); }, mutate)(
			&IBuff::RefreshBuffs, GetClientComponent<IBuff>, OP_RefreshTargetBuffs, sender, true, suspended, slots);
}

void ClientPatch::SendSingleBuffChange(Mob* sender, const Buffs_Struct& buff, int slot, bool remove, bool ackreq) {
	bool suspended = zone->BuffTimersSuspended();
	std::vector slots = { static_cast<uint32_t>(slot) };

	// first, send to self if self is client, which takes the definition and the refresh
	if (sender->IsClient()) {
		Client* c = sender->CastToClient();
		IBuff* component = GetClientComponent<IBuff>(c);
		FastQueuePacket(c, &IBuff::BuffDefinition, component, sender, buff,  slot, remove);
		FastQueuePacket(c, &IBuff::RefreshBuffs, component, OP_RefreshBuffs, sender, remove, suspended, slots);

		// the client doesn't automatically do this for some reason pre-TOB
		if (remove && component->NeedsWearMessage())
			c->SendColoredText(Chat::Spells, spells[buff.spellid].spell_fades);
	}

	// the rest of the buff packets do not take the definition, only the refresh
	if (sender->IsPet() && sender->GetOwner()->IsClient()) {
		if (Mob* owner = sender->GetOwner(); owner != nullptr && owner->IsClient()) {
			Client* c = owner->CastToClient();
			FastQueuePacket(c, &IBuff::RefreshBuffs, GetClientComponent<IBuff>(c), OP_RefreshPetBuffs, sender, remove, suspended, slots);
		}
	}

	auto mutate = [sender](std::unique_ptr<EQApplicationPacket>& packet, Client* c) {
		SetRefreshType(packet, sender, c);
	};

	QueueClientsByTarget(sender, ackreq, ShouldSendTargetBuffs, mutate)(
		&IBuff::RefreshBuffs, GetClientComponent<IBuff>, OP_RefreshTargetBuffs, sender, remove, suspended, slots);
}
