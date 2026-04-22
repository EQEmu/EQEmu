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

#include "zone/patch/components/message/titanium.h"
#include "zone/patch/components/message/sof.h"
#include "zone/patch/components/message/sod.h"
#include "zone/patch/components/message/uf.h"
#include "zone/patch/components/message/rof.h"
#include "zone/patch/components/message/rof2.h"
#include "zone/patch/components/message/tob.h"

#include "client.h"
#include "websocketpp/http/parser.hpp"

using Version = EQ::versions::ClientVersion;
using namespace ZoneClient;

struct ClientComponents {
	explicit ClientComponents(Version version) : version(version) {
		switch (version) {
		case Version::TOB:
			messageComponent = std::make_shared<Message::TOB>();
			break;
		case Version::RoF2:
			messageComponent = std::make_shared<Message::RoF2>();
			break;
		case Version::RoF:
			messageComponent = std::make_shared<Message::RoF>();
			break;
		case Version::UF:
			messageComponent = std::make_shared<Message::UF>();
			break;
		case Version::SoD:
			messageComponent = std::make_shared<Message::SoD>();
			break;
		case Version::SoF:
			messageComponent = std::make_shared<Message::SoF>();
			break;
		default:
			messageComponent = std::make_shared<Message::Titanium>();
			break;
		}
	}

	const Version version;
	std::shared_ptr<Message::IMessage> messageComponent;
};

static const ClientComponents& GetComponents(Version version) {
	static const std::unordered_map<Version, ClientComponents> patches = [] {
		std::unordered_map<Version, ClientComponents> p;
		p.emplace(Version::Titanium, Version::Titanium);
		p.emplace(Version::SoF, Version::SoF);
		p.emplace(Version::SoD, Version::SoD);
		p.emplace(Version::UF, Version::UF);
		p.emplace(Version::RoF, Version::RoF);
		p.emplace(Version::RoF2,Version::RoF2);
		p.emplace(Version::TOB, Version::TOB);
		return p;
	}();

	return patches.at(version);
}

const std::shared_ptr<Message::IMessage>& ClientPatch::GetMessageComponent(Version version) {
	return GetComponents(version).messageComponent;
}

void Client::SetClientVersion(Version client_version)
{
	m_ClientVersion    = client_version;
	m_ClientVersionBit = EQ::versions::ConvertClientVersionToClientVersionBit(client_version);
	m_messageComponent = GetComponents(client_version).messageComponent;
}

Version Client::GetClientVersion() const { return m_ClientVersion; }
