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

#include "common/struct_strategy.h"
#include "common/patches/IBuff.h"
#include "common/patches/IMessage.h"

class EQStreamIdentifier;

namespace Titanium {

extern void Register(EQStreamIdentifier& into);
extern void Reload();

class Strategy : public StructStrategy
{
public:
	Strategy();

protected:
	virtual std::string Describe() const;
	virtual const EQ::versions::ClientVersion ClientVersion() const;

	//magic macro to declare our opcode processors
#include "ss_declare.h"
#include "titanium_ops.h"
};

class MessageComponent : public ClientPatch::IMessage
{
public:
	MessageComponent() = default;
	~MessageComponent() override = default;

	std::unique_ptr<EQApplicationPacket> Simple(uint32_t color, uint32_t id) const override;
	std::unique_ptr<EQApplicationPacket> Formatted(uint32_t color, uint32_t id,
		const FormattedArgs& args) const override;

	std::unique_ptr<EQApplicationPacket> InterruptSpell(uint32_t message, uint32_t spawn_id,
		const char* spell_link) const override;
	std::unique_ptr<EQApplicationPacket> InterruptSpellOther(Mob* sender, uint32_t message, uint32_t spawn_id,
		const char* name,
		const char* spell_link) const override;

protected:
	[[nodiscard]] virtual uint32_t ResolveID(uint32_t id) const;
	virtual void ResolveArguments(uint32_t id, std::array<const char*, 9>& args) const;
};

class BuffComponent : public ClientPatch::IBuff
{
public:
	BuffComponent(uint32_t maxLongBuffs, uint32_t maxShortBuffs) : IBuff(maxLongBuffs, maxShortBuffs) {}
	BuffComponent() = delete;
	~BuffComponent() override = default;

	std::unique_ptr<EQApplicationPacket> BuffDefinition(Mob* mob, const Buffs_Struct& buff, uint32_t slot,
		bool fade) const override;
	std::unique_ptr<EQApplicationPacket> RefreshBuffs(EmuOpcode opcode, Mob* mob, bool remove,
		bool buff_timers_suspended, const std::vector<uint32_t>& slots) const override;
	bool NeedsWearMessage() const override;
	void SetRefreshType(std::unique_ptr<EQApplicationPacket>& packet, uint8_t refresh_type) const override;
};

} /*Titanium*/

