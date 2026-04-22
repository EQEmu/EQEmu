//
// Created by dannu on 4/21/2026.
//

#pragma once

#include <memory>

#include "common/emu_versions.h"
#include "components/message/IMessage.h"

#include "zone/client.h"
#include "zone/mob.h"

namespace ZoneClient {

// store all static functions for the different patches here
class ClientPatch {
public:
	using ClientList = std::unordered_map<uint16, Client*>;
    static const std::shared_ptr<Message::IMessage>& GetMessageComponent(EQ::versions::ClientVersion version);

	template<typename Fun, typename... Args>
	static void QueuePacket(Client* c, Fun fun, Args&&... args) {
		static_assert(std::is_member_function_pointer_v<Fun>);
		EQApplicationPacket* app = std::invoke(fun, c->GetMessageComponent().get(), std::forward<Args>(args)...);
		if (app != nullptr) {
			c->QueuePacket(app);
			delete app;
		}
	}

	// packet generator queue functions
	static auto QueueClients(Mob* sender, bool ignore_sender = false, bool ackreq = true) {
		return [=]<typename Fun, typename... Args>(Fun fun, Args&&... args) {
			static_assert(std::is_member_function_pointer_v<Fun>);
			std::map<EQ::versions::ClientVersion, EQApplicationPacket*> build_packets;
			auto client_list = entity_list.GetClientList();

			for (auto [_, ent] : client_list) {
				if (!ignore_sender || ent != sender) {
					auto [packet, _] = build_packets.try_emplace(
						ent->ClientVersion(),
						std::invoke(fun, GetMessageComponent(ent->ClientVersion()).get(), std::forward<Args>(args)...));
					if (packet->second != nullptr)
						ent->QueuePacket(packet->second, ackreq, Client::CLIENT_CONNECTED);
				}
			}

			for (auto [_, packet] : build_packets)
				if (packet != nullptr)
					delete packet;
		};
	}

	static auto QueueCloseClients(Mob* sender, bool ignore_sender = false, float distance = 200,
	                              Mob* skipped_mob = nullptr, bool is_ack_required = true,
	                              eqFilterType filter = FilterNone) {
		if (distance <= 0) distance = zone->GetClientUpdateRange();

		return [=]<typename Fun, typename... Args>(Fun fun, Args&&... args) {
			if (sender == nullptr) {
				QueueClients(sender, ignore_sender, is_ack_required)(fun, std::forward<Args>(args)...);
			} else {
				float distance_squared = distance * distance;
				std::map<EQ::versions::ClientVersion, EQApplicationPacket*> build_packets;

				for (auto& [_, mob] : sender->GetCloseMobList(distance)) {
					if (mob && mob->IsClient()) {
						Client* client = mob->CastToClient();
						if ((!ignore_sender || client != sender) &&
							client != skipped_mob &&
							DistanceSquared(client->GetPosition(), sender->GetPosition()) < distance_squared &&
							client->Connected()) {
							eqFilterMode client_filter = client->GetFilter(filter);
							if (filter == FilterNone || client_filter == FilterShow ||
								(client_filter == FilterShowGroupOnly &&
									(sender == client || (client->GetGroup() && client->GetGroup()->IsGroupMember(
										sender)))) ||
								(client_filter == FilterShowSelfOnly && client == sender)
							) {
								auto [packet, _] = build_packets.try_emplace(
									client->ClientVersion(),
									std::invoke(fun, GetMessageComponent(client->ClientVersion()).get(),
									            std::forward<Args>(args)...));
								if (packet->second != nullptr)
									client->QueuePacket(packet->second, is_ack_required, Client::CLIENT_CONNECTED);
							}
						}
					}
				}

				for (auto [_, packet] : build_packets)
					if (packet != nullptr)
						delete packet;;
			}
		};
	}
};

}
