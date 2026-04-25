//
// Created by dannu on 4/24/2026.
//

#pragma once

#include "common/emu_opcodes.h"

#include <cstdint>
#include <vector>

#include "common/types.h"

class Client;
class Mob;
class EQApplicationPacket;
class Buffs_Struct;

namespace Buff {

class IBuff
{
public:
    IBuff() = default;
    virtual ~IBuff() = default;

    virtual EQApplicationPacket* MakeLegacyBuffsPacket(Mob* mob, int32_t timer, bool for_target, bool clear_buffs) const = 0;

	virtual EQApplicationPacket* BuffDefinition(Mob* mob, const Buffs_Struct& buff, int slot, bool fade) const = 0;
    virtual EQApplicationPacket* RefreshBuffs(EmuOpcode opcode, Mob* mob, int32_t timer, bool remove, bool buff_timers_suspended, const std::vector<uint32_t>& slots) const = 0;
    virtual void SetRefreshType(EQApplicationPacket* packet, Mob* source, Client* target) const = 0;
};

} // namespace Buff
