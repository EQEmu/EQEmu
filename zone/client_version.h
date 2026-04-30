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

#pragma once

#include "common/emu_versions.h"
#include "common/patches/client_version.h"
#include "common/patches/IBuff.h"
#include "common/patches/IMessage.h"

#include "zone/client.h"
#include "zone/mob.h"

#include <ranges>

// store all _generic_ static functions for the different patches here
namespace ClientPatch {

using ClientList = std::unordered_map<uint16, Client*>;
template<typename Obj> using ComponentGetter = Obj*(*)(const Client*);
using SendPredicate = std::function<bool(Client*)>;
using MutatePacket = std::function<void(std::unique_ptr<EQApplicationPacket>&, Client*)>;

template <typename Component>
Component* GetClientComponent(const Client* client)
{
	return GetComponent<Component>(client->GetClientVersion()).get();
}

template <typename Fun, typename Obj, typename... Args>
	requires std::is_member_function_pointer_v<Fun>
void QueuePacket(Client* c, Fun fun, Obj* obj, Args&&... args)
{
	if (obj != nullptr) {
		std::unique_ptr<EQApplicationPacket> app = std::invoke(fun, obj, std::forward<Args>(args)...);
		if (app)
			c->QueuePacket(app.get());
	}
}

// packet generator queue functions
inline auto QueueClients(Mob* sender, bool ignore_sender = false, bool ackreq = true)
{
	return [=]<typename Fun, typename Obj, typename... Args>(Fun fun, ComponentGetter<Obj> component, Args&&... args)
			requires std::is_member_function_pointer_v<Fun>
	{
		std::array<std::unique_ptr<EQApplicationPacket>, EQ::versions::ClientVersionCount> build_packets;

		for (auto ent : entity_list.GetClientList() | std::views::values) {
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

inline auto QueueCloseClients(
	Mob* sender, bool ignore_sender = false, float distance = 200,
	Mob* skipped_mob = nullptr, bool is_ack_required = true,
	eqFilterType filter = FilterNone)
{
	if (distance <= 0) distance = static_cast<float>(zone->GetClientUpdateRange());

	return [=]<typename Fun, typename Obj, typename... Args>(Fun fun, ComponentGetter<Obj> component, Args&&... args)
			requires std::is_member_function_pointer_v<Fun>
	{
		if (sender == nullptr) {
			QueueClients(sender, ignore_sender, is_ack_required)(fun, component, std::forward<Args>(args)...);
		} else {
			float distance_squared = distance * distance;
			std::array<std::unique_ptr<EQApplicationPacket>, EQ::versions::ClientVersionCount> build_packets;

			for (auto mob : sender->GetCloseMobList(distance) | std::views::values) {
				if (mob && mob->IsClient()) {
					Client* client = mob->CastToClient();
					if ((!ignore_sender || client != sender)
						&& client != skipped_mob
						&& client->Connected()
						&& client->ShouldGetPacket(sender, filter)
						&& DistanceSquared(client->GetPosition(), sender->GetPosition()) < distance_squared)
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

template <typename Fun, typename Obj, typename... Args>
void FastQueuePacket(Client* c, Fun fun, Obj* obj, Args&&... args)
	requires std::is_member_function_pointer_v<Fun>
{
	if (obj != nullptr) {
		std::unique_ptr<EQApplicationPacket> app = std::invoke(fun, obj, std::forward<Args>(args)...);
		if (app) {
			// FastQueuePacket specifically takes lifetime management of packet, so release here
			EQApplicationPacket* packet = app.release();
			c->FastQueuePacket(&packet);
		}
	}
}

inline auto QueueClientsByTarget(Mob* sender, bool ackreq, const SendPredicate& should_send, const MutatePacket& mutate)
{
	return [=]<typename Fun, typename Obj, typename... Args>(Fun fun, ComponentGetter<Obj> component, Args&&... args)
			requires std::is_member_function_pointer_v<Fun>
	{
		if (sender != nullptr) {
			std::array<std::unique_ptr<EQApplicationPacket>, EQ::versions::ClientVersionCount> build_packets;

			for (auto c : entity_list.GetClientList() | std::views::values) {
				Mob* Target = c->GetTarget();
				if (Target == sender && should_send(c)) {
					auto& packet = build_packets.at(static_cast<uint32_t>(c->ClientVersion()));
					if (!packet)
						if (auto comp = component(c); comp != nullptr)
							packet = std::invoke(fun, comp, std::forward<Args>(args)...);

					mutate(packet, c);

					if (packet)
						c->QueuePacket(packet.get(), ackreq, Client::CLIENT_CONNECTED);
				}
			}
		}
	};
}

// Helper functions to wrap the packet construction in sends
template <AllConstChar... Args>
	requires (sizeof...(Args) <= 9)
void SendMessageString(Client* c, uint32_t type, uint32_t id, Args&&... args)
{
	if constexpr (sizeof...(Args) == 0) {
		QueuePacket(c, &IMessage::Simple, GetClientComponent<IMessage>(c), type, id);
	} else {
		IMessage::FormattedArgs a = {args...};
		QueuePacket(c, &IMessage::Formatted, GetClientComponent<IMessage>(c), type, id, a);
	}
}

inline auto BroadcastMessageStringInRadius(
	Mob* sender, bool ignore_sender = false, float distance = 200.f,
	Mob* skipped_mob = nullptr, bool is_ack_required = true,
	eqFilterType filter = FilterNone)
{
	return [=]<AllConstChar... Args>(uint32_t type, uint32_t id, Args&&... args)
			requires (sizeof...(Args) <= 9)
	{
		auto queue_close_clients = QueueCloseClients(sender, ignore_sender, distance, skipped_mob,
			is_ack_required, filter);

		if constexpr (sizeof...(Args) == 0) {
			return queue_close_clients(&IMessage::Simple, GetClientComponent<IMessage>, type, id);
		} else {
			IMessage::FormattedArgs a = {args...};
			return queue_close_clients(&IMessage::Formatted, GetClientComponent<IMessage>, type, id, a);
		}
	};
}

void InterruptSpell(Client* c, uint32_t message, uint32_t spawn_id, const char* spell_link);
void InterruptSpellOther(Mob* sender, uint32_t message, uint32_t spawn_id, const char* name,
	const char* spell_link);

void SendFullBuffRefresh(Mob* sender, bool remove = false, bool ackreq = true);
void SendSingleBuffChange(Mob* sender, const Buffs_Struct& buff, int slot, bool remove = false, bool ackreq = true);

} // namespace ClientPatch
