//
// Created by dannu on 4/24/2026.
//

#pragma once

#include "client_version.h"
#include "common/emu_opcodes.h"

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

	virtual std::unique_ptr<EQApplicationPacket> BuffDefinition(Mob* mob, const Buffs_Struct& buff, int slot,
        bool fade) const = 0;
    virtual std::unique_ptr<EQApplicationPacket> RefreshBuffs(EmuOpcode opcode, Mob* mob, bool remove,
        bool buff_timers_suspended, const std::vector<uint32_t>& slots) const = 0;
    virtual void SetRefreshType(std::unique_ptr<EQApplicationPacket>& packet, Mob* source, Client* target) const = 0;
};

} // namespace Buff
