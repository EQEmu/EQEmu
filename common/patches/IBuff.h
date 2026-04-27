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

    // TODO: I think all of the legacy stuff can just be rolled into regular buff functions and the call sites modified
    virtual std::unique_ptr<EQApplicationPacket> MakeLegacyBuffsPacket(Mob* mob, bool for_target,
        bool clear_buffs) const = 0;
    virtual std::unique_ptr<EQApplicationPacket> LegacyBuffDefinition(Mob* mob, Buffs_Struct& buff, int slot) const = 0; // TODO: add the buffs definition packet

	virtual std::unique_ptr<EQApplicationPacket> BuffDefinition(Mob* mob, const Buffs_Struct& buff, int slot,
        bool fade) const = 0;
    virtual std::unique_ptr<EQApplicationPacket> RefreshBuffs(EmuOpcode opcode, Mob* mob, int32_t timer, bool remove,
        bool buff_timers_suspended, const std::vector<uint32_t>& slots) const = 0;
    virtual void SetRefreshType(std::unique_ptr<EQApplicationPacket>& packet, Mob* source, Client* target) const = 0;
};

} // namespace Buff
