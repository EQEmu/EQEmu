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

#include "common/emu_opcodes.h"

#include <vector>
#include <functional>

class Client;
class Mob;
class EQApplicationPacket;
class Buffs_Struct;

namespace ClientPatch {

class IBuff
{
public:
    using BuffSequenceFunc = std::function<std::unique_ptr<EQApplicationPacket>(const Client*)>;

    IBuff(uint32_t maxLongBuffs, uint32_t maxShortBuffs)
      : m_maxLongBuffs(maxLongBuffs)
      , m_maxShortBuffs(maxShortBuffs)
    {}

    IBuff() = delete;
    virtual ~IBuff() = default;

	virtual std::unique_ptr<EQApplicationPacket> BuffDefinition(Mob* mob, const Buffs_Struct& buff, uint32_t slot,
        bool fade) const = 0;
    virtual std::unique_ptr<EQApplicationPacket> RefreshBuffs(EmuOpcode opcode, Mob* mob, bool remove,
        bool buff_timers_suspended, const std::vector<uint32_t>& slots) const = 0;
    virtual void SetRefreshType(std::unique_ptr<EQApplicationPacket>& packet, uint8_t refresh_type) const = 0;
	virtual bool NeedsWearMessage() const = 0;

    uint32_t ServerToPatchBuffSlot(uint32_t slot) const;

protected:
    uint32_t m_maxLongBuffs;
    uint32_t m_maxShortBuffs;
};

} // namespace Buff
