//
// Created by dannu on 4/21/2026.
//

#pragma once


#include "common/emu_versions.h"
#include "common/patches/client_version.h"
#include "common/patches/IMessage.h"

#include "zone/client.h"
#include "zone/mob.h"

// store all _generic_ static functions for the different patches here
namespace ClientPatch {

using ClientList = std::unordered_map<uint16, Client*>;
template<typename Obj> using ComponentGetter = std::function<Obj*(const Client*)>;

template <typename Fun, typename Obj, typename... Args>
	requires std::is_member_function_pointer_v<Fun>
static void QueuePacket(Client* c, Fun fun, Obj* obj, Args&&... args)
{
	if (obj != nullptr) {
		std::unique_ptr<EQApplicationPacket> app = std::invoke(fun, obj, std::forward<Args>(args)...);
		if (app)
			c->QueuePacket(app.get());
	}
}

// packet generator queue functions
static auto QueueClients(Mob* sender, bool ignore_sender = false, bool ackreq = true)
{
	return [=]<typename Fun, typename Obj, typename... Args>(Fun fun, const ComponentGetter<Obj>& component, Args&&... args)
			requires std::is_member_function_pointer_v<Fun>
	{
		std::array<std::unique_ptr<EQApplicationPacket>, EQ::versions::ClientVersionCount> build_packets;
		std::unordered_map<uint16, Client*> client_list = entity_list.GetClientList();

		for (auto [_, ent] : client_list) {
			if (!ignore_sender || ent != sender) {
				auto& packet = build_packets.at(static_cast<uint32_t>(ent->ClientVersion()));
				if (!packet)
					if (auto comp = component(ent); comp != nullptr)
						packet = std::invoke(fun, comp, std::forward<Args>(args)...);

				if (packet)
					ent->QueuePacket(packet.get(), ackreq, Client::CLIENT_CONNECTED);
			}
		}
	};
}

static auto QueueCloseClients(
	Mob* sender, bool ignore_sender = false, float distance = 200,
	Mob* skipped_mob = nullptr, bool is_ack_required = true,
	eqFilterType filter = FilterNone)
{
	if (distance <= 0) distance = static_cast<float>(zone->GetClientUpdateRange());

	return [=]<typename Fun, typename Obj, typename... Args>(Fun fun, const ComponentGetter<Obj>& component, Args&&... args)
			requires std::is_member_function_pointer_v<Fun>
	{
		if (sender == nullptr) {
			QueueClients(sender, ignore_sender, is_ack_required)(fun, component, std::forward<Args>(args)...);
		} else {
			float distance_squared = distance * distance;
			std::array<std::unique_ptr<EQApplicationPacket>, EQ::versions::ClientVersionCount> build_packets;

			for (auto& [_, mob] : sender->GetCloseMobList(distance)) {
				if (mob && mob->IsClient()) {
					Client* client = mob->CastToClient();
					if ((!ignore_sender || client != sender)
						&& client != skipped_mob
						&& DistanceSquared(client->GetPosition(), sender->GetPosition()) < distance_squared
						&& client->Connected()
						&& client->ShouldGetPacket(sender, filter))
					{
						auto& packet = build_packets.at(static_cast<uint32_t>(client->ClientVersion()));
						if (!packet)
							if (auto comp = component(client); comp != nullptr)
								packet = std::invoke(fun, comp, std::forward<Args>(args)...);

						if (packet)
							client->QueuePacket(packet.get(), is_ack_required, Client::CLIENT_CONNECTED);
					}
				}
			}
		}
	};
}

} // namespace ClientPatch

// Helpers for the Message interface to send message packets
namespace Message {

// this can return nullptr when the component doesn't exist for the version
static std::function GetComponent = [](const Client* c) -> IMessage* {
	return GetMessageComponent(c->GetClientVersion()).get();
};

// Helper functions to wrap the packet construction in sends
template <AllConstChar... Args>
	requires (sizeof...(Args) <= 9)
void MessageString(Client* c, uint32_t type, uint32_t id, Args&&... args)
{
	if constexpr (sizeof...(Args) == 0) {
		ClientPatch::QueuePacket(c, &IMessage::Simple, GetComponent(c), type, id);
	} else {
		std::array<const char*, 9> a = {args...};
		ClientPatch::QueuePacket(c, &IMessage::Formatted, GetComponent(c), type, id, a);
	}
}

static auto CloseMessageString(
	Mob* sender, bool ignore_sender = false, float distance = 200.f,
	Mob* skipped_mob = nullptr, bool is_ack_required = true,
	eqFilterType filter = FilterNone)
{
	return [=]<AllConstChar... Args>(uint32_t type, uint32_t id, Args&&... args)
			requires (sizeof...(Args) <= 9)
	{
		auto queue_close_clients = ClientPatch::QueueCloseClients(sender, ignore_sender, distance, skipped_mob,
			is_ack_required, filter);

		if constexpr (sizeof...(Args) == 0) {
			return queue_close_clients(&IMessage::Simple, GetComponent, type, id);
		} else {
			std::array<const char*, 9> a = {args...};
			return queue_close_clients(&IMessage::Formatted, GetComponent, type, id, a);
		}
	};
}

inline void InterruptSpell(Client* c, uint32_t message, uint32_t spawn_id, const char* spell_link)
{
	ClientPatch::QueuePacket(c, &IMessage::InterruptSpell, GetComponent(c), message, spawn_id, spell_link);
}

inline void InterruptSpellOther(Mob* sender, uint32_t message, uint32_t spawn_id, const char* name,
	const char* spell_link)
{
	ClientPatch::QueueCloseClients(sender, true, RuleI(Range, SongMessages), nullptr, true,
		sender->IsClient() ? FilterPCSpells : FilterNPCSpells)(
		&IMessage::InterruptSpellOther, GetComponent, sender, message, spawn_id, name, spell_link);
}

} // namespace Message
