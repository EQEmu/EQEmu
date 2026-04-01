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
#include "client.h"

#include "common/event/task.h"
#include "zone/water_map.h"
#include "zone/zone.h"

extern Zone *zone;

void CullPoints(std::vector<FindPerson_Point> &points) {
	if (!zone->HasMap()) {
		return;
	}

	size_t i = 0;
	for (; i < points.size(); ++i) {
		auto &p = points[i];

		for (;;) {
			if (i + 2 >= points.size()) {
				return;
			}

			if (points.size() < 36) {
				return;
			}

			auto &p1 = points[i + 1];
			auto &p2 = points[i + 2];

			if (zone->zonemap->CheckLoS(glm::vec3(p.x, p.y, p.z), glm::vec3(p2.x, p2.y, p2.z))) {
				points.erase(points.begin() + i + 1);
				LogInfo("Culled find path point [{}], connecting [{}]->[{}] instead", i + 1, i, i + 2);
			}
			else {
				break;
			}
		}
	}
}

void Client::SendPathPacket(const std::vector<FindPerson_Point> &points) {
	std::any data(points);
	EQ::Task([=](EQ::Task::ResolveFn resolve, EQ::Task::RejectFn reject) {
		auto points = std::any_cast<std::vector<FindPerson_Point>>(data);
		CullPoints(points);
		resolve(points);
	})
	.Then([this](const std::any &result) {
		auto points = std::any_cast<std::vector<FindPerson_Point>>(result);
		if (points.size() < 2) {
			if (Admin() > AccountStatus::Steward) {
				Message(Chat::System, "Too few points");
			}
		
			EQApplicationPacket outapp(OP_FindPersonReply, 0);
			QueuePacket(&outapp);
			return;
		}
		
		if (points.size() > 36) {
			if (Admin() > AccountStatus::Steward) {
				Message(Chat::System, "Too many points %u", points.size());
			}
		
			EQApplicationPacket outapp(OP_FindPersonReply, 0);
			QueuePacket(&outapp);
			return;
		}
		
		if (Admin() > AccountStatus::Steward) {
			Message(Chat::System, "Total points %u", points.size());
		}
		
		int len = sizeof(FindPersonResult_Struct) + (points.size() + 1) * sizeof(FindPerson_Point);
		auto outapp = new EQApplicationPacket(OP_FindPersonReply, len);
		FindPersonResult_Struct* fpr = (FindPersonResult_Struct*)outapp->pBuffer;
		
		std::vector<FindPerson_Point>::iterator cur, end;
		cur = points.begin();
		end = points.end();
		unsigned int r;
		for (r = 0; cur != end; ++cur, r++) {
			fpr->path[r] = *cur;
		
		}
		//put the last element into the destination field
		--cur;
		fpr->path[r] = *cur;
		fpr->dest = *cur;
		
		FastQueuePacket(&outapp);
	})
	.Run();
}
