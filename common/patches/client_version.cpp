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

#include <array>

using Version = EQ::versions::ClientVersion;

struct ClientComponents
{
	explicit ClientComponents(Version version) : version(version)
	{
		switch (version) {
		case Version::TOB:
			messageComponent = std::make_unique<Message::TOB>();
			break;
		case Version::RoF2:
			messageComponent = std::make_unique<Message::RoF2>();
			break;
		case Version::RoF:
			messageComponent = std::make_unique<Message::RoF>();
			break;
		case Version::UF:
			messageComponent = std::make_unique<Message::UF>();
			break;
		case Version::SoD:
			messageComponent = std::make_unique<Message::SoD>();
			break;
		case Version::SoF:
			messageComponent = std::make_unique<Message::SoF>();
			break;
		case Version::Titanium:
			messageComponent = std::make_unique<Message::Titanium>();
			break;
		default:
			break;
		}
	}

	const Version version;
	std::unique_ptr<Message::IMessage> messageComponent;
};

// this array must be in the same order as the Version enum because it converts Version to index directly
static const std::array<ClientComponents, EQ::versions::ClientVersionCount> s_patches = {
	{
		ClientComponents(Version::Unknown), // empty
		ClientComponents(Version::Client62), // empty
		ClientComponents(Version::Titanium),
		ClientComponents(Version::SoF),
		ClientComponents(Version::SoD),
		ClientComponents(Version::UF),
		ClientComponents(Version::RoF),
		ClientComponents(Version::RoF2),
		ClientComponents(Version::TOB),
	}
};

const std::unique_ptr<Message::IMessage>& GetMessageComponent(Version version)
{
	return s_patches.at(static_cast<uint32_t>(version)).messageComponent;
}
