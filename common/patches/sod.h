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
#include "common/patches/sof.h"

class EQStreamIdentifier;

namespace SoD {

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
#include "sod_ops.h"
};

class BuffComponent : public Titanium::BuffComponent
{
public:
	BuffComponent(uint32_t maxLongBuffs, uint32_t maxShortBuffs) : Titanium::BuffComponent(maxLongBuffs, maxShortBuffs) {}
	BuffComponent() = delete;
	~BuffComponent() override = default;

	std::unique_ptr<EQApplicationPacket> RefreshBuffs(EmuOpcode opcode, Mob* mob, bool remove,
		bool buff_timers_suspended, const std::vector<uint32_t>& slots) const override;
	void SetRefreshType(std::unique_ptr<EQApplicationPacket>& packet, uint8_t refresh_type) const override;
};

} /*SoD*/
