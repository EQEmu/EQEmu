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

#include "common/patches/titanium.h"
#include "common/patches/sof.h"
#include "common/patches/sod.h"
#include "common/patches/uf.h"
#include "common/patches/rof.h"
#include "common/patches/rof2.h"
#include "common/patches/tob.h"

using Version = EQ::versions::ClientVersion;

struct ClientComponents
{
	explicit ClientComponents(Version version) : version(version)
	{
		switch (version) {
		case Version::TOB:
			buffComponent = std::make_shared<Buff::TOB>();
			messageComponent = std::make_shared<Message::TOB>();
			break;
		case Version::RoF2:
			buffComponent = std::make_shared<Buff::RoF2>();
			messageComponent = std::make_shared<Message::RoF2>();
			break;
		case Version::RoF:
			buffComponent = std::make_shared<Buff::RoF>();
			messageComponent = std::make_shared<Message::RoF>();
			break;
		case Version::UF:
			buffComponent = std::make_shared<Buff::UF>();
			messageComponent = std::make_shared<Message::UF>();
			break;
		case Version::SoD:
			buffComponent = std::make_shared<Buff::SoD>();
			messageComponent = std::make_shared<Message::SoD>();
			break;
		case Version::SoF:
			buffComponent = std::make_shared<Buff::SoF>();
			messageComponent = std::make_shared<Message::SoF>();
			break;
		default:
			buffComponent = std::make_shared<Buff::Titanium>();
			messageComponent = std::make_shared<Message::Titanium>();
			break;
		}
	}

	const Version version;
    std::shared_ptr<Buff::IBuff> buffComponent;
	std::shared_ptr<Message::IMessage> messageComponent;
};

static const ClientComponents& GetComponents(Version version)
{
	static const std::unordered_map<Version, ClientComponents> patches = [] {
		std::unordered_map<Version, ClientComponents> p;
		p.emplace(Version::Titanium, Version::Titanium);
		p.emplace(Version::SoF, Version::SoF);
		p.emplace(Version::SoD, Version::SoD);
		p.emplace(Version::UF, Version::UF);
		p.emplace(Version::RoF, Version::RoF);
		p.emplace(Version::RoF2, Version::RoF2);
		p.emplace(Version::TOB, Version::TOB);
		return p;
	}();

	return patches.at(version);
}

const std::shared_ptr<Buff::IBuff>& GetBuffComponent(Version version)
{
	return GetComponents(version).buffComponent;
}

const std::shared_ptr<Message::IMessage>& GetMessageComponent(Version version)
{
	return GetComponents(version).messageComponent;
}
